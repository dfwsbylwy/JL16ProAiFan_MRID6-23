#pragma once
#include <vector>
#include <iostream>
#include <windows.h>
#include "ec.hpp"

//ö��״̬ģʽ
enum WMISystemPerMode :unsigned char {
    GameMode = 0,
    PerformanceMode,
    QuietMode,
    DiyMode = 255
};
//ö��״̬ģʽ
enum ECAddress :unsigned char {
    CPUTempAddress = 0x1c,
    GPUtempAddress = 0x1d,
    MaxFanSpeedAddress = 0x5f,
    ModeAddress = 0xe4,
    KeyLight2RGB = 0xe8,
    KeyLightR = 0xe9,
    KeyLightG = 0xea,
    KeyLightB = 0xeb
};

class CFanControl
{
public:
    // ��������
    static EmbeddedController FCEC;

    static short int m_CPUTemp;
    static short int m_GPUTemp;
    static short int m_MaxTemp;

    static short int m_CPUFanSpeed;
    static short int m_GPUFanSpeed;
    static bool m_FanSpeedZero;
    static bool m_FanSetStatus;
    static bool m_isMRID6_23;


    static short int m_MaxFanSpeedSet;

    static short int m_ModeSet;  //now status

    static unsigned short int m_Steps;

    // �¶�-ת�ٿ��Ʊ�
    static std::vector<std::vector<short int>> m_TempSpeedTable;

    // ���һ����̬��Ա��������Ϊ�¶�-ת�ٱ���
    static std::map<short int, short int> m_FanSpeedCache;


    // ��������
    CFanControl();
    void TempSpeedTableSet(std::vector<std::vector<short int>> vTempSpeedTable);
    short int InterpolateFanSpeed();
    void UpdateTemp();
    void UpdateFanSpeed();
    void CheckFanSpeedZero();
    void UpdateMode();
    void SetMaxFanSpeed(bool & UIUpdateFlag);
};
