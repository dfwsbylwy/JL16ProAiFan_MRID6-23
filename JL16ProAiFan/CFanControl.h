#pragma once
#include <vector>
#include <iostream>
#include <windows.h>
#include "ec.hpp"


#define SetPerformaceMode2 R"(.\JiaoLongWMI.exe PerformaceMode-SetPerformaceMode-2)"  //QuietMode
#define SetPerformaceMode0 R"(.\JiaoLongWMI.exe PerformaceMode-SetPerformaceMode-0)"  //GameMode
//TcmdProcess(SetPerformaceMode2,TRUE, CFanControl::m_JiaoLongWMIexeisOK);
//TcmdProcess(SetPerformaceMode0,TRUE, CFanControl::m_JiaoLongWMIexeisOK);
#define SwitchMaxFanSpeed1 R"(.\JiaoLongWMI.exe Fan-SwitchMaxFanSpeed-1)"
//TcmdProcess(SwitchMaxFanSpeed1,TRUE, CFanControl::m_JiaoLongWMIexeisOK);

//枚举状态模式
enum WMISystemPerMode :unsigned char {
    GameMode = 0,
    PerformanceMode,
    QuietMode,
    DiyMode = 255
};
//枚举状态模式
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
    // 变量申明
    static EmbeddedController FCEC;

    static unsigned short int m_CPUTemp;
    static unsigned short int m_GPUTemp;
    static unsigned short int m_MaxTemp;

    static unsigned short int m_CPUFanSpeed;
    static unsigned short int m_GPUFanSpeed;
    static bool m_FanSpeedZero;
    static bool m_FanSetStatus;
    static bool m_isMRID6_23;
    static bool m_JiaoLongWMIexeisOK;

    static unsigned short int m_MaxFanSpeedSet;

    static unsigned short int m_ModeSet;  //now status

    static unsigned short int m_Steps;

    // 温度-转速控制表
    static std::vector<std::vector<unsigned short int>> m_TempSpeedTable;

    // 添加一个静态成员变量来作为温度-转速表缓存
    static std::map<unsigned short int, unsigned short int> m_FanSpeedCache;


    // 方法定义
    CFanControl();
    void TempSpeedTableSet(std::vector<std::vector<unsigned short int>> vTempSpeedTable);
    unsigned short int InterpolateFanSpeed();
    void UpdateTemp();
    void UpdateFanSpeed();
    void FanSpeedNoZero();
    void FixedMaxFanSpeed2Mode();
    void UpdateMode();
    void SetMaxFanSpeed(bool & UIUpdateFlag);
};
