//#include "pch.h"
#include "CFanControl.h"
#include <cmath> // ����cmathͷ�ļ���ʹ��std::round
//#include <deque>
//#include <iostream>
//#include <windows.h>
#include "ec.hpp"
#include <thread>
#include <chrono>

//using namespace std;
//��̬��Ա��ʼ��ֵ
EmbeddedController CFanControl::FCEC = EmbeddedController();

short int CFanControl::m_CPUTemp = -1;
short int CFanControl::m_GPUTemp = -1;
short int CFanControl::m_MaxTemp = -1;

short int CFanControl::m_CPUFanSpeed = -1;
short int CFanControl::m_GPUFanSpeed = -1;
bool CFanControl::m_FanSpeedZero = FALSE;
bool CFanControl::m_FanSetStatus = TRUE;
bool CFanControl::m_isMRID6_23 = FALSE;
short int CFanControl::m_MaxFanSpeedSet = -1;
short int CFanControl::m_ModeSet = -1;


unsigned short int CFanControl::m_Steps = 1;

std::vector<std::vector<short int>> CFanControl::m_TempSpeedTable = {{70,22}, {78,24}, {84,26}, {88,28}, {90,30}, {93,35}, {97,42},{100,49}};

std::map<short int, short int> CFanControl::m_FanSpeedCache;




    //���캯����ʹ�ó�Ա��ʼ���б��ʼ����Ա����
CFanControl::CFanControl( )
{

}


void CFanControl::TempSpeedTableSet(std::vector<std::vector<short int>> vTempSpeedTable)
{
    if (vTempSpeedTable.size() < 2 || vTempSpeedTable[0].size() != 2)
        CFanControl::m_TempSpeedTable = vTempSpeedTable;
    else
        CFanControl::m_TempSpeedTable = { {70,22}, {78,24}, {84,26}, {88,28}, {90,30}, {93,35}, {97,42}, {100,49} };
}


//���ݸ������¶�-ת�ٱ��Ŀ���¶ȣ����ز�ֵ����ķ���ת��
short int CFanControl::InterpolateFanSpeed( ) {

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
    short int x1 = CFanControl::m_TempSpeedTable[indexLow][0];
    short int y1 = CFanControl::m_TempSpeedTable[indexLow][1];
    short int x2 = CFanControl::m_TempSpeedTable[indexLow + 1][0];
    short int y2 = CFanControl::m_TempSpeedTable[indexLow + 1][1];

    // �������Բ�ֵб��
    double slope = static_cast<double>(y2 - y1) / (x2 - x1);
    // ����б�ʺͲ�ѯ�¶ȼ����ֵ�ķ���ת��
    short int interpolatedSpeed = y1 + round(slope * (CFanControl::m_MaxTemp - x1));


    // Ӧ�ñ߽�����
    interpolatedSpeed = max(min(interpolatedSpeed, 50), 22);
    // ����õ���ֵ����󣬽���洢�ڻ�����
    m_FanSpeedCache[CFanControl::m_MaxTemp] = interpolatedSpeed;

    return interpolatedSpeed; // ���ز�ֵ���
}


void CFanControl::UpdateTemp()
{
    short int ReadCPUTemp = CFanControl::FCEC.readByte(CPUTempAddress);
    if (ReadCPUTemp >= 30 && ReadCPUTemp <= 120)
        CFanControl::m_CPUTemp = ReadCPUTemp;

    short int ReadGPUTemp = CFanControl::FCEC.readByte(GPUtempAddress);
    if (ReadGPUTemp >= 30 && ReadGPUTemp <= 120)
        CFanControl::m_GPUTemp = ReadGPUTemp;


    // ÿ������¶���ཱུ3�ȣ��������10�ȣ���ֹ�¶ȶ�ȡ���󶸱仯;
    short int MaxTempChaDiff = max(CFanControl::m_CPUTemp, CFanControl::m_GPUTemp) - CFanControl::m_MaxTemp;

    MaxTempChaDiff = max(min(MaxTempChaDiff, 10), -3);  // ֵ������-3��10
    CFanControl::m_MaxTemp = CFanControl::m_MaxTemp + MaxTempChaDiff;

}


void CFanControl::CheckFanSpeedZero( )
{
    //���η����ٶȶ�ȡΪ0����ǿ�Ƴ�ʼ��ģʽΪ��Ϸģʽ��������·���ת�١�
    //if (CFanControl::m_CPUFanSpeed == 0 && CFanControl::m_GPUFanSpeed == 0)
    if (CFanControl::m_CPUFanSpeed <= 2100 && CFanControl::m_GPUFanSpeed <= 2100)
    {
        if (CFanControl::m_FanSpeedZero && CFanControl::m_MaxTemp >= 70)
        {
            if (CFanControl::FCEC.writeByte(ModeAddress, GameMode))
            {
                CFanControl::m_ModeSet = GameMode;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                CFanControl::m_FanSpeedZero = FALSE;
            }

        }
        //CFanControl::UpdateFanSpeed(); //���·���ת��
        CFanControl::m_FanSpeedZero = TRUE;
    }
    else
    {

        CFanControl::m_FanSpeedZero = FALSE;
    }

}


void CFanControl::UpdateFanSpeed()
{
    short int CPUFanSpeedQ = CFanControl::FCEC.readByte(0x9b);
    short int CPUFanSpeedH = CFanControl::FCEC.readByte(0x9C);
    short int GPUFanSpeedQ = CFanControl::FCEC.readByte(0x9d);
    short int GPUFanSpeedH = CFanControl::FCEC.readByte(0x9e);


    if (CPUFanSpeedQ >= 0 && CPUFanSpeedH >= 0)
            CFanControl::m_CPUFanSpeed = CPUFanSpeedQ * 255 + CPUFanSpeedH;
    if (GPUFanSpeedQ >= 0 && GPUFanSpeedH >= 0)
            CFanControl::m_GPUFanSpeed = GPUFanSpeedQ * 255 + GPUFanSpeedH;
}


void CFanControl::UpdateMode()
{

    short int ReadModeSet = CFanControl::FCEC.readByte(ModeAddress);

    // Mode update
    if (ReadModeSet >=0 && CFanControl::m_ModeSet != ReadModeSet)
        CFanControl::m_ModeSet = ReadModeSet;
}


void CFanControl::SetMaxFanSpeed(bool & UIUpdateFlag)
{
    UpdateMode();

    if (CFanControl::m_FanSetStatus)
    {
        // MaxFanSpeed set
        short int MaxFanSpeedValue = InterpolateFanSpeed();
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
                }
                break;
            }
            case PerformanceMode: {
                if (MaxFanSpeedValue <= 35)
                {
                    CFanControl::FCEC.writeByte(ModeAddress, DiyMode);
                }
                else
                {
                    CFanControl::FCEC.writeByte(ModeAddress, GameMode);
                }
                break;
            }
            case QuietMode: {
                //cout << "��⵽QuietMode,��ʼ��Mode����������GPU���ĺ�FanSpeed����\n";
                if (CFanControl::FCEC.writeByte(ModeAddress, GameMode)) // QuietMode�����Ƚ���GameMode����ʼ������GPU����ǽ��
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(700));
                    //std::this_thread::sleep_for(std::chrono::milliseconds(210));
                    //CFanControl::FCEC.writeByte(ModeAddress, DiyMode);
                }
                break;
            }

            case DiyMode: {
                if (MaxFanSpeedValue > 35)
                {
                    CFanControl::FCEC.writeByte(ModeAddress, GameMode);
                }
                break;
            }
            default: {//����DiyModeֵ
                if (MaxFanSpeedValue > 35)
                {
                    CFanControl::FCEC.writeByte(ModeAddress, GameMode);
                }
                break;
            }
        }//switch endline

    }
    else if( UIUpdateFlag ){

        // MaxFanSpeed update
        short int ReadMaxFanSpeedValue = CFanControl::FCEC.readByte(MaxFanSpeedAddress);
        if (CFanControl::m_MaxFanSpeedSet != ReadMaxFanSpeedValue && ReadMaxFanSpeedValue >= 22 && ReadMaxFanSpeedValue <= 58)
        {
            CFanControl::m_MaxFanSpeedSet = ReadMaxFanSpeedValue;
        }

    }
}

