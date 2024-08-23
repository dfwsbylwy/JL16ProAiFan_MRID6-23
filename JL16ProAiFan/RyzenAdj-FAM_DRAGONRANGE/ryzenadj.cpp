// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2018-2019 Jiaxun Yang <jiaxun.yang@flygoat.com> */
/* Ryzen NB SMU Service Request Tool */

#include <string.h>


#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define VER_STR "v0.0.1"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h> // For LoadLibrary, GetProcAddress, etc.
#include <shlwapi.h> // For PathCombine
#endif
#include <sys/stat.h> // For file existence check
#include <iostream>
#include <cstring> // For strlen
#include <tchar.h>
#include <map>
#include "ryzenadj.h"


//void* load_library(LPCWSTR library_name) {
//#if defined(_WIN32) || defined(_WIN64)
//    return LoadLibrary(library_name);
//#else
//    return dlopen(library_name, RTLD_LAZY);
//#endif
//}




bool doAdjust (int opt, unsigned int arg, smu_service_args_t* args, nb_t& nb) {

    memset(args, 0, sizeof(*args));  //结构体清0；
    //smu_service_args_t args = { 0, 0, 0, 0, 0, 0 }
    printf("load to set");

    args->arg0 = arg;
    if (smu_service_req(nb, opt, args) == 0x1) {
        printf("Successfully set %u to %x\n", arg, args->arg0);
        return true;
    }
    else {
        printf("Failed to set %u\n", arg);
        return false;
    }

}

unsigned int GetCoreMask(int coreIndex, int coresInCcx) {
    unsigned int ccxInCcd = 1u;

    unsigned int ccd = static_cast<unsigned int>(coreIndex / coresInCcx);
    unsigned int ccx = 0u;
    unsigned int core = static_cast<unsigned int>(coreIndex % coresInCcx);

    unsigned int ccxnum = ccx + 1u;
    //cout << ccd << "/" << ccx << "/" << core << "\n";
    return ((((ccd << 4) | ((ccx % ccxnum) & 0xF)) << 4) | ((core % coresInCcx) & 0xF)) << 20;

}


// Function to get the number of physical CPU cores
int get_physical_cores() {
#if defined(_WIN32) || defined(_WIN64)
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = nullptr;
    DWORD returnLength = 0;
    GetLogicalProcessorInformation(nullptr, &returnLength);

    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(returnLength);
    if (buffer == nullptr) {
        return 0;
    }

    if (!GetLogicalProcessorInformation(buffer, &returnLength)) {
        free(buffer);
        return 0;
    }

    int physicalCores = 0;
    for (DWORD i = 0; i < returnLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (buffer[i].Relationship == RelationProcessorCore) {
            physicalCores++;
        }
    }

    free(buffer);
    return physicalCores;
#else
    sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.procs;
    }
    return 1; // Default to 1 core if unable to determine
#endif
}


int ryzenadj2do(std::map <libryzenadjTool, int> & libryzenadjData)
//int ryzenadj2do()
{    
    //std::map <libryzenadjTool, unsigned int> libryzenadjData;
    //printf("I'm come in\n");
    pci_obj_t pci_obj;
    nb_t nb;

    smu_service_args_t *smuArgs;
    int err = 0;

    pci_obj = init_pci_obj();
    if(!pci_obj){
        printf("Unable to get PCI Obj\n");
        return -1;
    }

    nb = get_nb(pci_obj);
    if(!nb){
        printf("Unable to get NB Obj\n");
        return -1;
    }

    smuArgs = (smu_service_args_t *)malloc(sizeof(*smuArgs));

    //unsigned int fast_limit = 53000;
    //doAdjust(0x56, fast_limit, smuArgs, nb);

    //初始化必要参数

    int cpuCores = 0;
    int coresInCcx = 0;




    // 使用范围基础的 for 循环遍历 map
    //std::cout << "Contents of the map:" << std::endl;
    for (const auto& Data : libryzenadjData) 
    {
        //std::cout << Data.first << ": " << Data.second << std::endl;

        switch (Data.first) {
        case libryzenadjTool::set_tctl_temp:
            if(!doAdjust(static_cast<int>(DRAGONRANGEaddr::set_tctl_temp), Data.second, smuArgs, nb))
                { err++; }
            break;
        case libryzenadjTool::set_cpu_fppt:
            if (!doAdjust(static_cast<int>(DRAGONRANGEaddr::set_cpu_fppt), Data.second, smuArgs, nb))
                { err++; }
            break;
        case libryzenadjTool::set_cpu_TDC:
            if (!doAdjust(static_cast<int>(DRAGONRANGEaddr::set_cpu_TDC), Data.second, smuArgs, nb))
                { err++; }
            break;
        case libryzenadjTool::set_cpu_EDC:
            if (!doAdjust(static_cast<int>(DRAGONRANGEaddr::set_cpu_EDC), Data.second, smuArgs, nb))
                { err++; }
            break;
        case libryzenadjTool::set_coall:
            if (!doAdjust(static_cast<int>(DRAGONRANGEaddr::set_coall), Data.second, smuArgs, nb))
                { err++; }
            break;
        case libryzenadjTool::cpu_core_num:
            if (cpuCores == 0) {
                cpuCores = libryzenadjData[libryzenadjTool::cpu_core_num];

                if (cpuCores == 0) {
                    cpuCores = get_physical_cores();
                }

                if (cpuCores == 6 || cpuCores == 8 || cpuCores == 12 || cpuCores == 16) {
                    coresInCcx = (cpuCores % 6 == 0) ? 6 : 8;
                    //coresInCcx = 8;
                }
            }
            break;
        default:
            int Coreid = static_cast<int>(Data.first) - CoreIdAddNum;
            if ( Coreid >= 0 && Coreid < 16) {
                if (cpuCores == 0) {
                    cpuCores = libryzenadjData[libryzenadjTool::cpu_core_num];

                    if (cpuCores == 0) {
                        cpuCores = get_physical_cores();
                    }

                    if (cpuCores == 6 || cpuCores == 8 || cpuCores == 12 || cpuCores == 16) {
                        coresInCcx = (cpuCores % 6 == 0) ? 6 : 8;
                        //coresInCcx = 8;
                    }
                }

                if (coresInCcx == 8 || coresInCcx == 6) {
                    unsigned int cpuOffsetMask = GetCoreMask(Coreid, coresInCcx) | (Data.second & 0xFFFF);

                    if (!doAdjust(static_cast<int>(DRAGONRANGEaddr::set_coper), cpuOffsetMask, smuArgs, nb))
                    {
                        err++;
                    }
                }
                
            }
            break;
        }

    }


    //system("pause");

    free(smuArgs);
    free_pci_obj(pci_obj);
    //std::cout << "err cout:" << err << std::endl;
    return err;
}

