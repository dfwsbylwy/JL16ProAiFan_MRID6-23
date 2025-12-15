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

    FanSpeedSetLow =10,
    FanSpeedSetHigh =58,

    DiyMode = 255
};


//枚举EC PM channel地址
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

//枚举EC Direct Access内存地址
enum ECMemoryTable : uint16_t
{
    EC_ADDR_PORT = 0x4E,    //EC 地址端口
    EC_DATA_PORT = 0x4F,    //EC 数据端口




    FAN1_Current_RPM_high = 0xC830, //3, #CPU Fan Current_RPM High byte
    FAN1_Current_RPM_low  = 0xC831, //3, #CPU Fan Current_RPM Low byte
    FAN2_Current_RPM_high = 0xC832, //3, #GPU Fan Current_RPM High byte
    FAN2_Current_RPM_low  = 0xC833, //3, #GPU Fan Current_RPM Low byte
    Fan1_RPM = 0XC834,      //CPU Fan 的 RPM 值。受Fan_RPM_SET控制
    Fan2_RPM = 0XC835,      //GPU Fan 的 RPM 值。受Fan_RPM_SET控制
    Fan_RPM_SET = 0xC83C,   //风扇1-2的 RPM 设置
    EC_Version = 0xC411,    //EC 版本
    Temp_Sensor1 = 0xC417,  //#T1#
    Near_CPU = 0xC418,      //#Near_CPU#
    Near_GPU = 0xC419,      //#Near_GPU#
    Temp_Sensor4 = 0xC41A,  //#Charger#
    Temp_Sensor5 = 0xC41B,  //#Environment#
    DTS_CPU = 0xC41C,       //#DTS_CPU#
    DTS_GPU = 0xC41D,       //#DTS_GPU#
    Temp_Sensor8 = 0xC41E,  //#NTC_V5P0A#
    Temp_Sensor9 = 0xC41F,  //#T3#
    Battery_T = 0xC902      //#Battery_T#

    //BAT_mAormW = #0x3B4, #0x3B5, #Battery_Mode# ;
    //BAT_RMC = #0x392, #0x393,  #Battery_RMC#  ;
    //BAT_FCC = #0x394, #0x395,  #Battery_FCC#  ;
    //BAT_RealRSOC = #0x000, #0x000,  #Battery_RSOC# ;
    //BAT_Current = #0xB27, #0xB28,  #Battery_C#    ;
    //BAT_Voltage = #0x390, #0x391,  #Battery_V#    ;
    //
    //FAN1_Current_RPM, = #0xC831, #0xC830, #3, #Fan1 Current_RPM#   ;
    //FAN1_Goal_RPM, = #0XC834, #0x0000, #3, #Fan1 Goal_RPM#      ;
    //FAN1_RPM_Level, = #0xC836, #0x0000, #3, #Fan1 Current_Level# ;
    //FAN1_Set_RPM, = #0xC83C, #0x0000, #3, #Fan1 Set_RPM#       ;
    //FAN1_PWM, = #0x1805, #0x0000, #3, #Fan1 PWM Duty_Cycle#;
    //
    //FAN2_Current_RPM, = #0xC833, #0xC832, #3, #Fan2 Current_RPM#   ;
    //FAN2_Goal_RPM, = #0XC835, #0x0000, #3, #Fan2 Goal_RPM#      ;
    //FAN2_RPM_Level, = #0xC837, #0x0000, #3, #Fan2 Current_Level# ;
    //FAN2_Set_RPM, = #0xC83D, #0x0000, #3, #Fan2 Set_RPM#       ;
    //FAN2_PWM, = #0x1804, #0x0000, #3, #Fan2 PWM Duty_Cycle#;
};


class CFanControl
{
public:
    // 变量申明
    static EmbeddedController FCEC;

    static BYTE m_CPUTemp;
    static BYTE m_GPUTemp;
    static BYTE m_MaxTemp;

    static unsigned short int m_CPUFanSpeed;
    static unsigned short int m_GPUFanSpeed;
    static bool m_FanSpeedZero;
    static bool m_FanSetStatus;
    static bool m_isMRID6_23;
    static bool m_JiaoLongWMIexeisOK;

    static BYTE m_MaxFanSpeedSet;
    static BYTE m_ModeSet;  //now status
    static bool BIOSVersionNoV31;
    static unsigned short int m_Steps;

    // 温度-转速控制表
    static std::vector<std::vector<unsigned short int>> m_TempSpeedTable;

    // 添加一个静态成员变量来作为温度-转速表缓存
    static std::map<unsigned short int, unsigned short int> m_FanSpeedCache;


    // 方法定义
    CFanControl();
    void TempSpeedTableSet(std::vector<std::vector<unsigned short int>> vTempSpeedTable);
    unsigned short int InterpolateFanSpeed();
    //void UpdateTemp(short int maxup = 10, short int maxdown = -3);
    void UpdateTemp();
    void UpdateFanSpeed();
    void UpdateMaxFanSpeedSet();
    void FanSpeedNoZero();
    void FixedMaxFanSpeed2Mode();
    void UpdateMode();
    void SetMaxFanSpeed();
};
