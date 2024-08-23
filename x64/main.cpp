#include <iostream>
#include <string>
#include <cmath> // 引入cmath头文件以使用std::round
#include <vector>
#include <windows.h>
#include <algorithm>
//#include "driver.hpp"
#include "ec.hpp"
#include <stdio.h>

using namespace std;

//枚举状态模式
enum WMISystemPerMode :unsigned int {
    GameMode,
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
EmbeddedController ec = EmbeddedController();

bool WhileStatus = TRUE;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
        // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
        ec.writeByte(ModeAddress, QuietMode);//程序推出循环，强制写回办公mode
        printf("Ctrl-C event\n\n");
        Beep(750, 300);
        WhileStatus = FALSE;
        return TRUE;

        // CTRL-CLOSE: confirm that the user wants to exit.
    case CTRL_CLOSE_EVENT:
        ec.writeByte(ModeAddress, QuietMode);//程序推出循环，强制写回办公mode
        Beep(600, 200);
        printf("Ctrl-Close event\n\n");
        WhileStatus = FALSE;
        return TRUE;

        // Pass other signals to the next handler.
    case CTRL_BREAK_EVENT:
        ec.writeByte(ModeAddress, QuietMode);//程序推出循环，强制写回办公mode
        Beep(900, 200);
        printf("Ctrl-Break event\n\n");
        WhileStatus = FALSE;
        return FALSE;

    case CTRL_LOGOFF_EVENT:
        ec.writeByte(ModeAddress, QuietMode);//程序推出循环，强制写回办公mode
        Beep(1000, 200);
        printf("Ctrl-Logoff event\n\n");
        WhileStatus = FALSE;
        return FALSE;

    case CTRL_SHUTDOWN_EVENT:
        ec.writeByte(ModeAddress, QuietMode);//程序推出循环，强制写回办公mode
        Beep(750, 500);
        printf("Ctrl-Shutdown event\n\n");
        WhileStatus = FALSE;
        return FALSE;

    default:
        return FALSE;
    }
}




// 函数声明：根据给定的温度-转速表和目标温度，返回插值计算的风扇转速
int InterpolateFanSpeed(const vector<vector<int>>& TempSpeedTable, int temp);




int main ( )
{

    // 设置待查询的温度值
    int MaxTemp = -1;
    int MaxFanSpeed = 22;
    int LastFanSpeedSet = 22;
    int LastCPUFanSpeed = 2200;
    int LastGPUFanSpeed = 2200;



    int LastMode = 255;

    int LastCPUTemp = 55;
    int LastGPUTemp = 55;

    // 初始化温度-转速对照表
    vector<vector<int>> TempSpeedTable = { {70,22}, {78,24}, {84,26}, {88,28},{90,30},{95,35},{100,49} };

    bool TempSpeedTableisOK = TRUE;
    if (TempSpeedTable.size() <  2 || TempSpeedTable[0].size() != 2)
    {
        TempSpeedTableisOK = FALSE;
    }


    if (ec.driverFileExist && ec.driverLoaded) 
    {
        printf("EmbeddedController程序调用成功\n");


        //// 获取当前进程的控制台窗口句柄
        HWND hWndConsole = GetConsoleWindow();

        if (hWndConsole != NULL)
        {
            printf("该控制器付费使用，请向作者赞赏后使用，联系QQ：626251037\n");
            printf("1s后，窗口最小化，温度-风扇转速控制器运行，可手动恢复窗口\n");

            //ec.writeByte(ModeAddress, GameMode);//初始化解锁GPU功耗墙；
            //Sleep(1400);
            //ec.writeByte(ModeAddress, DiyMode);//解锁风扇小于3500限制；
            //// 发送WM_SYSCOMMAND消息来最小化窗口
            //SendMessage(hWndConsole, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        }

       
        

        LastMode = DiyMode;
        //if (SetConsoleCtrlHandler(CtrlHandler, TRUE))
        if (TRUE)
        {
            //循环监测
            while (WhileStatus)
            {
                Sleep(1000);
                system("cls");
                int ReadCPUTemp = ec.readByte(CPUTempAddress);
                if (ReadCPUTemp >= 55 && ReadCPUTemp <= 110)
                    LastCPUTemp = ReadCPUTemp;

                int ReadGPUTemp = ec.readByte(GPUtempAddress);
                if (ReadGPUTemp >= 55 && ReadGPUTemp <= 110)
                    LastGPUTemp = ReadGPUTemp;

                MaxTemp = max(LastCPUTemp, LastGPUTemp);

                if (TempSpeedTableisOK)
                {

                    printf("跳出红尘外，享自由人生\n");
                    printf("小源源：626251037@qq.com\n");

                    MaxFanSpeed = InterpolateFanSpeed(TempSpeedTable, MaxTemp);

                    if (LastFanSpeedSet != MaxFanSpeed)
                    {

                        if (MaxFanSpeed <= 35)
                        {
                            if (LastMode != DiyMode)
                            {
                                ec.writeByte(ModeAddress, DiyMode);
                                LastMode = DiyMode;
                            }
                        }
                        else
                        {
                            if (LastMode != GameMode)
                            {
                                ec.writeByte(ModeAddress, GameMode);
                                LastMode = GameMode;
                            }
                        }


                        ec.writeByte(MaxFanSpeedAddress, MaxFanSpeed); // write value of register 0xe4
                        LastFanSpeedSet = MaxFanSpeed;

                    }


                    //int CPUFanspeed = ec.readByte(0x9b) * 255 + ec.readByte(0x9c);//CPUFanspeed
                    //Sleep(100);
                    //int GPUFanspeed = ec.readByte(0x9d) * 255 + ec.readByte(0x9e);//GPUFanspeed
                    int CPUFanspeedQ = ec.readByte(0x9b);
                    int CPUFanspeedH = ec.readByte(0x9C);
                    int GPUFanspeedQ = ec.readByte(0x9d);
                    int GPUFanspeedH = ec.readByte(0x9e);

                    if (CPUFanspeedQ && CPUFanspeedH)
                        LastCPUFanSpeed = CPUFanspeedQ * 255 + CPUFanspeedH;
                    if (GPUFanspeedQ && GPUFanspeedH)
                        LastGPUFanSpeed = GPUFanspeedQ * 255 + GPUFanspeedH;

                    cout << "CPU、GPU 温度 ：" << std::dec << LastCPUTemp << "、" << LastGPUTemp << "℃\n";
                    cout << "CPU、GPU Fan Speed ：" << LastCPUFanSpeed << "、" << LastGPUFanSpeed << "RPM\n";
                    cout << "写入Max Fan Speed：" << LastFanSpeedSet << "00 RPM，0x" << std::hex << LastFanSpeedSet << "，mode=" << LastMode << "\n";

                }

            }//while end
        }
    }  
    return 0;
}


int InterpolateFanSpeed(const vector<vector<int>>& TempSpeedTable, int temp){

    //vector<vector<int>> TempSpeedTable = { {70,22}, {78,24}, {84,26}, {88,28},{90,30},{95,35},{100,49} };

    // 获取温度-转速表的行数
    int rows = TempSpeedTable.size();
    // 寻找最接近查询温度的两个温度点的索引
    int indexLow = -1;
    for (int i = 0; i < rows; ++i) {
        if (TempSpeedTable[i][0] > temp) {
            if (indexLow == -1) indexLow = i - 1;
            break; // 找到大于查询温度的第一个点，记录前一个点的索引
        }
        indexLow = i; // 更新最接近温度点的索引
    }

    // 处理边界情况，确保有两个有效的索引用于插值
    if (indexLow == -1) indexLow = 0; // 如果所有温度都大于查询温度，则使用第一个点
    else if (indexLow == rows - 1) indexLow = rows - 2; // 如果查询温度大于最高温度，使用最后两个点

    // 根据索引获取两个相邻温度点的坐标(x,y)即(温度, 转速)
    int x1 = TempSpeedTable[indexLow][0];
    int y1 = TempSpeedTable[indexLow][1];
    int x2 = TempSpeedTable[indexLow + 1][0];
    int y2 = TempSpeedTable[indexLow + 1][1];

    // 计算线性插值斜率
    double slope = static_cast<double>(y2 - y1) / (x2 - x1);
    //cout << x1 << "," << x2 << "," << y1 << "," << y2 << endl;
    // 根据斜率和查询温度计算插值的风扇转速
    int interpolatedSpeed = y1 + round(slope * (temp - x1));

    // 添加边界限制，确保结果在合理的范围内
    if (interpolatedSpeed < 22)
        return 22; // 最小转速限制
    else if (interpolatedSpeed > 50)
        return 50; // 最大转速限制
    else
        return interpolatedSpeed; // 返回插值结果
}