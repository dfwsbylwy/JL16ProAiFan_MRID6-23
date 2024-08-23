#pragma once
#include "nb_smu_ops.h"

constexpr int CoreIdAddNum = 100;

enum class libryzenadjTool : unsigned int {
    set_tctl_temp = 1,
    set_cpu_fppt,
    set_cpu_TDC,
    set_cpu_EDC,
    set_coall,
    set_coper,
    cpu_core_num = 16,
    set_coper00 = CoreIdAddNum,  //coreid + 100
    set_coper01,
    set_coper02,
    set_coper03,
    set_coper04,
    set_coper05,
    set_coper06,
    set_coper07,
    set_coper08,
    set_coper09,
    set_coper10,
    set_coper11,
    set_coper12,
    set_coper13,
    set_coper14,
    set_coper15,
};


enum DRAGONRANGEaddr : unsigned int
{
	set_coper = 0x6,        // OC one
	set_coall = 0x7,        // OC all
	set_cpu_fppt = 0x56,    // Short-term power (mW)
	set_cpu_TDC = 0x57,     // TDC (mA)
	set_cpu_EDC = 0x58,     // EDC (mA)
	set_tctl_temp = 0x59,   // Temperature wall (°„C)
	set_disable_oc = 0x5e,  // Disable overclocking
	set_enable_oc = 0x5d    // Enable overclocking
};



bool doAdjust(int opt, unsigned int arg, smu_service_args_t* args, nb_t& nb);

//int ryzenadj2do();
int ryzenadj2do(std::map<libryzenadjTool, int> & libryzenadjData);


unsigned int GetCoreMask(int coreIndex, int coresInCcx);

int get_physical_cores();


