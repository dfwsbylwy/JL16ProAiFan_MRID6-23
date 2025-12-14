#include "wmic.h"
#include "BoardProduct.h"

//std::cout << "Hello World!\n";
//system("COLOR 0A");
//try
//{
//	WMIC w;
//	w.OperatingSystem();//系统
//	w.VideoController();//显卡
//	w.DiskDrive();//硬盘
//	w.BaseBoard();//主板
//	w.BIOS();//主板 BIOS 芯片
//	w.PhysicalMemory();//内存条
//	w.Processor();//CPU处理器
//	w.NetworkAdapter();//网卡
//}
//catch (const WMIC_ERROR& e)
//{
//	std::cout << e.what() << std::endl;
//}


std::wstring getBoardProduct()
{

	try
	{
		WMIC w;
		std::wstring BoardProduct = w.BoardProduct();//主板

		return BoardProduct;
	}
	catch (const WMIC_ERROR& e)
		{
			//std::cout << e.what() << std::endl;
		}
}


std::wstring getSMBIOSBIOSVersion()
{

	try
	{
		WMIC w;
		std::wstring SMBIOSBIOSVersion = w.SMBIOSBIOSVersion();//主板

		return SMBIOSBIOSVersion;
	}
	catch (const WMIC_ERROR& e)
	{
		//std::cout << e.what() << std::endl;
	}
}



bool isJL16Pro() {
	std::wstring SMBIOSBIOSVersion = L"MRID6_23_V35"; // 假设这是你的SMBIOSBIOSVersion值
	std::wstring searchForBoardProduct = L"MRID6_23";
	std::wstring searchForBIOSVersion = L"V31";

	//SMBIOSBIOSVersion = getBoardProduct();//主板bios
	SMBIOSBIOSVersion = getSMBIOSBIOSVersion();//主板bios

	size_t foundPos = SMBIOSBIOSVersion.find(searchForBoardProduct);
	//如果 find 函数返回的不是 std::string::npos，则说明子串存在
	if (foundPos != std::wstring::npos) {
		//std::wcout << L"Found \"" << searchFor << L"\" at position: " << foundPos << std::endl;

		return true;
	}
	else {
		//std::wcout << L"\"MRID6_23\" not found." << std::endl;
		return false;
	}

}