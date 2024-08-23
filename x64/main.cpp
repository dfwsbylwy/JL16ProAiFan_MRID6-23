#include <iostream>
#include <string>
#include <cmath> // ����cmathͷ�ļ���ʹ��std::round
#include <vector>
#include <windows.h>
#include <algorithm>
//#include "driver.hpp"
#include "ec.hpp"
#include <stdio.h>

using namespace std;

//ö��״̬ģʽ
enum WMISystemPerMode :unsigned int {
    GameMode,
    PerformanceMode,
    QuietMode,
    DiyMode = 255
};
//ö��״̬ģʽ
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
        ec.writeByte(ModeAddress, QuietMode);//�����Ƴ�ѭ����ǿ��д�ذ칫mode
        printf("Ctrl-C event\n\n");
        Beep(750, 300);
        WhileStatus = FALSE;
        return TRUE;

        // CTRL-CLOSE: confirm that the user wants to exit.
    case CTRL_CLOSE_EVENT:
        ec.writeByte(ModeAddress, QuietMode);//�����Ƴ�ѭ����ǿ��д�ذ칫mode
        Beep(600, 200);
        printf("Ctrl-Close event\n\n");
        WhileStatus = FALSE;
        return TRUE;

        // Pass other signals to the next handler.
    case CTRL_BREAK_EVENT:
        ec.writeByte(ModeAddress, QuietMode);//�����Ƴ�ѭ����ǿ��д�ذ칫mode
        Beep(900, 200);
        printf("Ctrl-Break event\n\n");
        WhileStatus = FALSE;
        return FALSE;

    case CTRL_LOGOFF_EVENT:
        ec.writeByte(ModeAddress, QuietMode);//�����Ƴ�ѭ����ǿ��д�ذ칫mode
        Beep(1000, 200);
        printf("Ctrl-Logoff event\n\n");
        WhileStatus = FALSE;
        return FALSE;

    case CTRL_SHUTDOWN_EVENT:
        ec.writeByte(ModeAddress, QuietMode);//�����Ƴ�ѭ����ǿ��д�ذ칫mode
        Beep(750, 500);
        printf("Ctrl-Shutdown event\n\n");
        WhileStatus = FALSE;
        return FALSE;

    default:
        return FALSE;
    }
}




// �������������ݸ������¶�-ת�ٱ��Ŀ���¶ȣ����ز�ֵ����ķ���ת��
int InterpolateFanSpeed(const vector<vector<int>>& TempSpeedTable, int temp);




int main ( )
{

    // ���ô���ѯ���¶�ֵ
    int MaxTemp = -1;
    int MaxFanSpeed = 22;
    int LastFanSpeedSet = 22;
    int LastCPUFanSpeed = 2200;
    int LastGPUFanSpeed = 2200;



    int LastMode = 255;

    int LastCPUTemp = 55;
    int LastGPUTemp = 55;

    // ��ʼ���¶�-ת�ٶ��ձ�
    vector<vector<int>> TempSpeedTable = { {70,22}, {78,24}, {84,26}, {88,28},{90,30},{95,35},{100,49} };

    bool TempSpeedTableisOK = TRUE;
    if (TempSpeedTable.size() <  2 || TempSpeedTable[0].size() != 2)
    {
        TempSpeedTableisOK = FALSE;
    }


    if (ec.driverFileExist && ec.driverLoaded) 
    {
        printf("EmbeddedController������óɹ�\n");


        //// ��ȡ��ǰ���̵Ŀ���̨���ھ��
        HWND hWndConsole = GetConsoleWindow();

        if (hWndConsole != NULL)
        {
            printf("�ÿ���������ʹ�ã������������ͺ�ʹ�ã���ϵQQ��626251037\n");
            printf("1s�󣬴�����С�����¶�-����ת�ٿ��������У����ֶ��ָ�����\n");

            //ec.writeByte(ModeAddress, GameMode);//��ʼ������GPU����ǽ��
            //Sleep(1400);
            //ec.writeByte(ModeAddress, DiyMode);//��������С��3500���ƣ�
            //// ����WM_SYSCOMMAND��Ϣ����С������
            //SendMessage(hWndConsole, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        }

       
        

        LastMode = DiyMode;
        //if (SetConsoleCtrlHandler(CtrlHandler, TRUE))
        if (TRUE)
        {
            //ѭ�����
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

                    printf("�����쳾�⣬����������\n");
                    printf("СԴԴ��626251037@qq.com\n");

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

                    cout << "CPU��GPU �¶� ��" << std::dec << LastCPUTemp << "��" << LastGPUTemp << "��\n";
                    cout << "CPU��GPU Fan Speed ��" << LastCPUFanSpeed << "��" << LastGPUFanSpeed << "RPM\n";
                    cout << "д��Max Fan Speed��" << LastFanSpeedSet << "00 RPM��0x" << std::hex << LastFanSpeedSet << "��mode=" << LastMode << "\n";

                }

            }//while end
        }
    }  
    return 0;
}


int InterpolateFanSpeed(const vector<vector<int>>& TempSpeedTable, int temp){

    //vector<vector<int>> TempSpeedTable = { {70,22}, {78,24}, {84,26}, {88,28},{90,30},{95,35},{100,49} };

    // ��ȡ�¶�-ת�ٱ������
    int rows = TempSpeedTable.size();
    // Ѱ����ӽ���ѯ�¶ȵ������¶ȵ������
    int indexLow = -1;
    for (int i = 0; i < rows; ++i) {
        if (TempSpeedTable[i][0] > temp) {
            if (indexLow == -1) indexLow = i - 1;
            break; // �ҵ����ڲ�ѯ�¶ȵĵ�һ���㣬��¼ǰһ���������
        }
        indexLow = i; // ������ӽ��¶ȵ������
    }

    // ����߽������ȷ����������Ч���������ڲ�ֵ
    if (indexLow == -1) indexLow = 0; // ��������¶ȶ����ڲ�ѯ�¶ȣ���ʹ�õ�һ����
    else if (indexLow == rows - 1) indexLow = rows - 2; // �����ѯ�¶ȴ�������¶ȣ�ʹ�����������

    // ����������ȡ���������¶ȵ������(x,y)��(�¶�, ת��)
    int x1 = TempSpeedTable[indexLow][0];
    int y1 = TempSpeedTable[indexLow][1];
    int x2 = TempSpeedTable[indexLow + 1][0];
    int y2 = TempSpeedTable[indexLow + 1][1];

    // �������Բ�ֵб��
    double slope = static_cast<double>(y2 - y1) / (x2 - x1);
    //cout << x1 << "," << x2 << "," << y1 << "," << y2 << endl;
    // ����б�ʺͲ�ѯ�¶ȼ����ֵ�ķ���ת��
    int interpolatedSpeed = y1 + round(slope * (temp - x1));

    // ��ӱ߽����ƣ�ȷ������ں���ķ�Χ��
    if (interpolatedSpeed < 22)
        return 22; // ��Сת������
    else if (interpolatedSpeed > 50)
        return 50; // ���ת������
    else
        return interpolatedSpeed; // ���ز�ֵ���
}