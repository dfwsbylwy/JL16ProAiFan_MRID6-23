//#include "pch.h"
#include "CFanControl.h"
#include <cmath> // ����cmathͷ�ļ���ʹ��std::round
//#include <deque>
//#include <iostream>
//#include <windows.h>
#include "RunCommand.h"
#include "ec.hpp"
#include <thread>
#include <chrono>

#define BaseFanSpeed 2100


//using namespace std;
//��̬��Ա��ʼ��ֵ
EmbeddedController CFanControl::FCEC = EmbeddedController();

BYTE CFanControl::m_CPUTemp = 0;
BYTE CFanControl::m_GPUTemp = 0;
BYTE CFanControl::m_MaxTemp = 60;   //Ĭ��60��

unsigned short int CFanControl::m_CPUFanSpeed = 0;
unsigned short int CFanControl::m_GPUFanSpeed = 0;
bool CFanControl::m_FanSpeedZero = TRUE;
bool CFanControl::m_FanSetStatus = FALSE;
bool CFanControl::m_isMRID6_23 = FALSE;
bool CFanControl::m_JiaoLongWMIexeisOK = FALSE;
BYTE CFanControl::m_MaxFanSpeedSet = 25;
BYTE CFanControl::m_ModeSet = GameMode;


unsigned short int CFanControl::m_Steps = 0;

std::vector<std::vector<unsigned short int>> CFanControl::m_TempSpeedTable = {{70,22}, {78,24}, {84,26}, {88,28}, {90,30}, {93,35}, {97,42},{100,49}};

std::map<unsigned short int, unsigned short int> CFanControl::m_FanSpeedCache;


    //���캯����ʹ�ó�Ա��ʼ���б��ʼ����Ա����
CFanControl::CFanControl( )
{

}


void CFanControl::TempSpeedTableSet(std::vector<std::vector<unsigned short int>> vTempSpeedTable)
{
    if (vTempSpeedTable.size() < 2 || vTempSpeedTable[0].size() != 2)
        CFanControl::m_TempSpeedTable = vTempSpeedTable;
    else
        CFanControl::m_TempSpeedTable = { {70,22}, {78,24}, {84,26}, {88,28}, {90,30}, {93,35}, {97,42}, {100,49} };
}


//���ݸ������¶�-ת�ٱ��Ŀ���¶ȣ����ز�ֵ����ķ���ת��
unsigned short int CFanControl::InterpolateFanSpeed( ) {

    // ��黺�����Ƿ��Ѿ��ж�Ӧ�¶ȵĽ��
    auto cacheIt = m_FanSpeedCache.find(CFanControl::m_MaxTemp);
    if (cacheIt != m_FanSpeedCache.end()) {
        return cacheIt->second; // ���ػ����еĽ��
    }

    // ��ȡ�¶�-ת�ٱ������
    short int rows = CFanControl::m_TempSpeedTable.size();
    // Ѱ����ӽ���ѯ�¶ȵ������¶ȵ������
    short int indexLow = -1;
    for (short int i = 0; i < rows; ++i)
    {
        if (CFanControl::m_TempSpeedTable[i][0] > CFanControl::m_MaxTemp)
        {
            if (indexLow == -1) indexLow = i - 1;
            break; // �ҵ����ڲ�ѯ�¶ȵĵ�һ���㣬��¼ǰһ���������
        }
        indexLow = i; // ������ӽ��¶ȵ������
    }

    // ����߽������ȷ����������Ч���������ڲ�ֵ
    if (indexLow == -1) indexLow = 0; // ��������¶ȶ����ڲ�ѯ�¶ȣ���ʹ�õ�һ����
    else if (indexLow == rows - 1) indexLow = rows - 2; // �����ѯ�¶ȴ�������¶ȣ�ʹ�����������

    // ����������ȡ���������¶ȵ������(x,y)��(�¶�, ת��)
    unsigned short int x1 = CFanControl::m_TempSpeedTable[indexLow][0];
    unsigned short int y1 = CFanControl::m_TempSpeedTable[indexLow][1];
    unsigned short int x2 = CFanControl::m_TempSpeedTable[indexLow + 1][0];
    unsigned short int y2 = CFanControl::m_TempSpeedTable[indexLow + 1][1];

    // �������Բ�ֵб��
    double slope = static_cast<double>(y2 - y1) / (x2 - x1);
    // ����б�ʺͲ�ѯ�¶ȼ����ֵ�ķ���ת��
    unsigned short int interpolatedSpeed = y1 + round(slope * (CFanControl::m_MaxTemp - x1));


    // Ӧ�ñ߽�����
    interpolatedSpeed = max(min(interpolatedSpeed, 50), 22);
    // ����õ���ֵ����󣬽���洢�ڻ�����
    m_FanSpeedCache[CFanControl::m_MaxTemp] = interpolatedSpeed;

    return interpolatedSpeed; // ���ز�ֵ���
}


void CFanControl::UpdateTemp()
{
    BYTE ReadCPUTemp = CFanControl::FCEC.readByte(CPUTempAddress);
    if (ReadCPUTemp >= 30 && ReadCPUTemp <= 120)
        CFanControl::m_CPUTemp = ReadCPUTemp;

    BYTE ReadGPUTemp = CFanControl::FCEC.readByte(GPUtempAddress);
    if (ReadGPUTemp >= 30 && ReadGPUTemp <= 120)
        CFanControl::m_GPUTemp = ReadGPUTemp;


    // ÿ������¶���ཱུ3�ȣ��������10�ȣ���ֹ�¶ȶ�ȡ���󶸱仯;
    short int MaxTempChaDiff = max(CFanControl::m_CPUTemp, CFanControl::m_GPUTemp) - CFanControl::m_MaxTemp;

    MaxTempChaDiff = max(min(MaxTempChaDiff, 10), -3);  // ֵ������-3��10
    CFanControl::m_MaxTemp = CFanControl::m_MaxTemp + MaxTempChaDiff;

}


void CFanControl::FanSpeedNoZero( )
{
    if (CFanControl::m_CPUFanSpeed < BaseFanSpeed && CFanControl::m_GPUFanSpeed < BaseFanSpeed)
    {
        if (CFanControl::m_MaxTemp >= 65 && CFanControl::m_ModeSet != GameMode)
        {
            TcmdProcess(SetPerformaceMode0, FALSE, CFanControl::m_JiaoLongWMIexeisOK);
            CFanControl::FCEC.writeByte(ModeAddress, GameMode);
            CFanControl::m_ModeSet = GameMode;
        }
    }
    else if (CFanControl::m_CPUFanSpeed >= BaseFanSpeed && CFanControl::m_GPUFanSpeed >= BaseFanSpeed)
    {
        CFanControl::m_FanSpeedZero = FALSE;
        if (!CFanControl::m_FanSetStatus)
        {
            FixedMaxFanSpeed2Mode();
        }
    }
}

void CFanControl::FixedMaxFanSpeed2Mode()
{

    //if (!CFanControl::m_FanSetStatus) {
    //}
    if (CFanControl::m_MaxFanSpeedSet < 35)
    {
        if (CFanControl::FCEC.writeByte(ModeAddress, DiyMode))
            CFanControl::m_ModeSet = DiyMode;
    }
    else if (CFanControl::m_MaxFanSpeedSet < 50)
    {
        if (CFanControl::FCEC.writeByte(ModeAddress, GameMode))
            CFanControl::m_ModeSet = GameMode;
    }
    else if (CFanControl::m_MaxFanSpeedSet <= 58)
    {
        if (CFanControl::FCEC.writeByte(ModeAddress, PerformanceMode))
            CFanControl::m_ModeSet = PerformanceMode;
    }
    
}


void CFanControl::UpdateFanSpeed()
{
    BYTE CPUFanSpeedQ = CFanControl::FCEC.readByte(0x9b);
    BYTE CPUFanSpeedH = CFanControl::FCEC.readByte(0x9C);
    BYTE GPUFanSpeedQ = CFanControl::FCEC.readByte(0x9d);
    BYTE GPUFanSpeedH = CFanControl::FCEC.readByte(0x9e);

    //if (CPUFanSpeedQ > 0 && CPUFanSpeedH >= 0)
    //        CFanControl::m_CPUFanSpeed = CPUFanSpeedQ * 255 + CPUFanSpeedH;
    //if (GPUFanSpeedQ > 0 && GPUFanSpeedH >= 0)
    //        CFanControl::m_GPUFanSpeed = GPUFanSpeedQ * 255 + GPUFanSpeedH;

    if (CPUFanSpeedQ > 2 && CPUFanSpeedQ <= 24)
        CFanControl::m_CPUFanSpeed = (CPUFanSpeedQ << 8) | CPUFanSpeedH;
    if (GPUFanSpeedQ > 2 && GPUFanSpeedQ <= 24)
        CFanControl::m_GPUFanSpeed = (GPUFanSpeedQ << 8) | GPUFanSpeedH;

}

void CFanControl::UpdateMaxFanSpeedSet()
{
    BYTE ReadMaxFanSpeedValue = CFanControl::FCEC.readByte(MaxFanSpeedAddress);
    if (CFanControl::m_MaxFanSpeedSet != ReadMaxFanSpeedValue && ReadMaxFanSpeedValue >= 22 && ReadMaxFanSpeedValue <= 58)
    {
        CFanControl::m_MaxFanSpeedSet = ReadMaxFanSpeedValue;
    }
}


void CFanControl::UpdateMode()
{

    BYTE ReadModeSet = CFanControl::FCEC.readByte(ModeAddress);

    // Mode update
    if ((ReadModeSet >= 0 && ReadModeSet <= 2) || ReadModeSet == 255)
    {
        CFanControl::m_ModeSet = ReadModeSet;
    }
}


void CFanControl::SetMaxFanSpeed(bool & UIUpdateFlag)
{
    //if (CFanControl::m_Steps % 10 == 0)
    //{
    //    UpdateMaxFanSpeedSet();
    //}

    // MaxFanSpeed set
    unsigned short int MaxFanSpeedValue = InterpolateFanSpeed();
    if (CFanControl::m_MaxFanSpeedSet != MaxFanSpeedValue)
    {
        if (CFanControl::FCEC.writeByte(MaxFanSpeedAddress, MaxFanSpeedValue))
            CFanControl::m_MaxFanSpeedSet = MaxFanSpeedValue;
    }

    // Mode set
    switch (CFanControl::m_ModeSet)
    {
        case GameMode: {
            if (MaxFanSpeedValue <= 35)
            {
                CFanControl::FCEC.writeByte(ModeAddress, DiyMode);
                CFanControl::m_ModeSet = DiyMode;
            }
            break;
        }
        case PerformanceMode: {
            if (MaxFanSpeedValue <= 35)
            {
                CFanControl::FCEC.writeByte(ModeAddress, DiyMode);
                CFanControl::m_ModeSet = DiyMode;
            }
            else
            {
                CFanControl::FCEC.writeByte(ModeAddress, GameMode);
                CFanControl::m_ModeSet = GameMode;
            }
            break;
        }
        case QuietMode: {
            //cout << "��⵽QuietMode,��ʼ��Mode����������GPU���ĺ�FanSpeed����\n";
            if (CFanControl::FCEC.writeByte(ModeAddress, GameMode)) // QuietMode�����Ƚ���GameMode����ʼ������GPU����ǽ��
            {
                CFanControl::m_ModeSet = GameMode;
                //std::this_thread::sleep_for(std::chrono::milliseconds(700));
                //std::this_thread::sleep_for(std::chrono::milliseconds(210));
                //CFanControl::FCEC.writeByte(ModeAddress, DiyMode);
            }
            break;
        }
        case DiyMode: {
            if (MaxFanSpeedValue > 35)
            {
                CFanControl::FCEC.writeByte(ModeAddress, GameMode);
                CFanControl::m_ModeSet = GameMode;
            }
            break;
        }
        default: {//����DiyModeֵ
            if (MaxFanSpeedValue > 35)
            {
                CFanControl::FCEC.writeByte(ModeAddress, GameMode);
                CFanControl::m_ModeSet = GameMode;
            }
            break;
        }
    }//switch endline

}

