#include <iostream>
#include <windows.h>
#include <cmath> // 引入cmath头文件以使用std::round
#include <vector>
#include <algorithm>
#include "ec.hpp"
#include <stdio.h>
#include <thread>

 using namespace std;




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
    ModeAddress = 0xe4
};
//EmbeddedController ec = EmbeddedController();  //新建ec实例




class FanControl
{
public:
    // 变量申明

    short int CPUTemp;
    short int GPUTemp;
    short int MaxTemp;

    short int CPUFanSpeed;
    short int GPUFanSpeed;
    bool FanSpeedZero;
    bool WhileStatus;

    short int MaxFanSpeedSet;
    short int ModeSet;

    unsigned short int Steps;
    // int ReadModeSet;
    EmbeddedController ec;

    // 温度-转速对照表
    vector<vector<short int>> TempSpeedTable;


    // 方法定义
    //构造函数，使用成员初始化列表初始化成员变量
    FanControl ( vector<vector<short int>> vTempSpeedTable = { {70,22}, {78,24}, {84,26}, {88,28},{90,30},{95,35},{100,49} })
     {
        // 注意：这里的大括号内可以有其他构造逻辑，
        // 但成员初始化是在这里完成的。
        //初始化值
        CPUTemp = 70;
        GPUTemp = 50;
        MaxTemp = 70;

        CPUFanSpeed = 0;
        GPUFanSpeed = 0;
        FanSpeedZero = FALSE;
        WhileStatus = TRUE;

        ec = EmbeddedController();

        if (vTempSpeedTable.size() <  2 || vTempSpeedTable[0].size() != 2)
            TempSpeedTable = vTempSpeedTable;
        else
            TempSpeedTable = { {70,22}, {78,24}, {84,26}, {88,28},{90,30},{95,35},{100,49} };

        //ec.writeByte(ModeAddress, GameMode);//初始化解锁GPU功耗墙；
        Steps = 1;
        ModeSet = ec.readByte(ModeAddress);
        MaxFanSpeedSet = ec.readByte(MaxFanSpeedAddress);

    }

    void TempSpeedTableSet(vector<vector<short int>> vTempSpeedTable )
    {
        if (vTempSpeedTable.size() < 2 || vTempSpeedTable[0].size() != 2)
            TempSpeedTable = vTempSpeedTable;
    }

    void TempSpeedisZero()
    {

        if (max(CPUFanSpeed, GPUFanSpeed) == 0 && MaxTemp > 70)
        {
            ec.writeByte(ModeAddress, GameMode);
        }

    }


    //根据给定的温度-转速表和目标温度，返回插值计算的风扇转速
    short int InterpolateFanSpeed (short int temp) {

        // 获取温度-转速表的行数
        short int rows = TempSpeedTable.size();
        // 寻找最接近查询温度的两个温度点的索引
        short int indexLow = -1;
        for (short int i = 0; i < rows; ++i)
        {
            if (TempSpeedTable[i][0] > temp)
            {
                if (indexLow == -1) indexLow = i - 1;
                break; // 找到大于查询温度的第一个点，记录前一个点的索引
            }
            indexLow = i; // 更新最接近温度点的索引
        }

        // 处理边界情况，确保有两个有效的索引用于插值
        if (indexLow == -1) indexLow = 0; // 如果所有温度都大于查询温度，则使用第一个点
        else if (indexLow == rows - 1) indexLow = rows - 2; // 如果查询温度大于最高温度，使用最后两个点

        // 根据索引获取两个相邻温度点的坐标(x,y)即(温度, 转速)
        short int x1 = TempSpeedTable[indexLow][0];
        short int y1 = TempSpeedTable[indexLow][1];
        short int x2 = TempSpeedTable[indexLow + 1][0];
        short int y2 = TempSpeedTable[indexLow + 1][1];

        // 计算线性插值斜率
        double slope = static_cast<double>(y2 - y1) / (x2 - x1);
        //cout << x1 << "," << x2 << "," << y1 << "," << y2 << endl;
        // 根据斜率和查询温度计算插值的风扇转速
        short int interpolatedSpeed = y1 + round(slope * (temp - x1));

        // 添加边界限制，确保结果在合理的范围内
        if (interpolatedSpeed < 22)
            return 22; // 最小转速限制
        else if (interpolatedSpeed > 50)
            return 50; // 最大转速限制
        else
            return interpolatedSpeed; // 返回插值结果
    }

    void UpdateTemp ( )
    {
        short int ReadCPUTemp = ec.readByte(CPUTempAddress);
        if (ReadCPUTemp >= 40 && ReadCPUTemp <= 120)
            CPUTemp = ReadCPUTemp;

        short int ReadGPUTemp = ec.readByte(GPUtempAddress);
        if (ReadGPUTemp >= 30 && ReadGPUTemp <= 120)
            GPUTemp = ReadGPUTemp;


        // 每次最大温度最多降3度，最多上升10度，防止温度读取错误陡变化;
        short int NowMaxTemp = max(CPUTemp, GPUTemp);
        short int MaxTempChaDiff = NowMaxTemp - MaxTemp;

        if ( MaxTempChaDiff >= 0 )
        {
            MaxTempChaDiff = min( MaxTempChaDiff, 10 );
        }
        else //if (MaxTempChaDiff < 0 )
        {
            MaxTempChaDiff = max( MaxTempChaDiff, -3 );
        }

        MaxTemp = MaxTemp + MaxTempChaDiff;

    }

    void UpdateFanSpeed ( )
    {
        short int CPUFanSpeedQ = ec.readByte(0x9b);
        short int CPUFanSpeedH = ec.readByte(0x9C);
        short int GPUFanSpeedQ = ec.readByte(0x9d);
        short int GPUFanSpeedH = ec.readByte(0x9e);


        //两次风扇速度读取为0，则强制初始化模式为游戏模式，否则更新风扇转速。
        if (CPUFanSpeedQ == 0 && CPUFanSpeedH == 0 && GPUFanSpeedQ == 0 && GPUFanSpeedH == 0)  
        {
            if (FanSpeedZero)
            {
                ec.writeByte(ModeAddress, GameMode);
            }
            FanSpeedZero = TRUE;
        }
        else
        { 
            if (CPUFanSpeedQ && CPUFanSpeedH)
                CPUFanSpeed = CPUFanSpeedQ * 255 + CPUFanSpeedH;
            if (GPUFanSpeedQ && GPUFanSpeedH)
                GPUFanSpeed = GPUFanSpeedQ * 255 + GPUFanSpeedH;

            FanSpeedZero = FALSE;

        }

    }

    void SetMaxFanSpeed ( )
    {

        short int MaxFanSpeedValue = InterpolateFanSpeed( MaxTemp );

        ModeSet = ec.readByte(ModeAddress);
        switch (ModeSet)
        {
            case GameMode:{
                if (MaxFanSpeedValue <= 35)
                {
                    //cout << "监测到GameMode，且MaxFanSpeed<= 35，解锁FanSpeed限制\n";
                    ec.writeByte(ModeAddress, DiyMode);
                }

                if (MaxFanSpeedSet != MaxFanSpeedValue)
                {
                    ec.writeByte(MaxFanSpeedAddress, MaxFanSpeedValue); // write value of register 0xe4
                    MaxFanSpeedSet = MaxFanSpeedValue;
                }

                break;
            }
            case PerformanceMode:{
                if (MaxFanSpeedValue <= 35)
                {
                    //cout << "监测到PerformanceMode，且MaxFanSpeed<= 35，解锁FanSpeed限制\n";
                    ec.writeByte(ModeAddress, DiyMode);
                }
                else
                {
                    ec.writeByte(ModeAddress, GameMode);
                }

                if (MaxFanSpeedSet != MaxFanSpeedValue)
                {
                    ec.writeByte(MaxFanSpeedAddress, MaxFanSpeedValue); // write value of register 0xe4
                    MaxFanSpeedSet = MaxFanSpeedValue;
                }

                break;
            }
            case QuietMode:{
                //cout << "监测到QuietMode,初始化Mode参数，解锁GPU功耗和FanSpeed限制\n";
                ec.writeByte(ModeAddress, GameMode);//QuietMode必须先进入GameMode，初始化解锁GPU功耗墙；
                if (MaxFanSpeedValue <= 35)
                {
                    Sleep(100);
                    ec.writeByte(ModeAddress, DiyMode);//解锁风扇小于3500rpm限制；
                }


                if (MaxFanSpeedSet != MaxFanSpeedValue)
                {
                    ec.writeByte(MaxFanSpeedAddress, MaxFanSpeedValue); // write value of register 0xe4
                    MaxFanSpeedSet = MaxFanSpeedValue;
                }

                break;
            }

            case DiyMode: {
                if (MaxFanSpeedValue > 35)
                {
                    ec.writeByte(ModeAddress, GameMode);//解锁风扇小于3500rpm限制；
                }

                if (MaxFanSpeedSet != MaxFanSpeedValue)
                {
                    ec.writeByte(MaxFanSpeedAddress, MaxFanSpeedValue); // write value of register 0xe4
                    MaxFanSpeedSet = MaxFanSpeedValue;
                }

                break;
            }

            default: {
                if (MaxFanSpeedValue > 35)
                {
                    ec.writeByte(ModeAddress, GameMode);
                }
               
                if (MaxFanSpeedSet != MaxFanSpeedValue)
                {
                    ec.writeByte(MaxFanSpeedAddress, MaxFanSpeedValue); // write value of register 0xe4
                    MaxFanSpeedSet = MaxFanSpeedValue;
                }
            }
        }//switch endline

    }

};

