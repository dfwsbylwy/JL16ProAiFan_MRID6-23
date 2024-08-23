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


bool isJL16Pro() {
	std::wstring BoardProduct = L"MRID6-23_V35"; // 假设这是你的BoardProduct值
	std::wstring searchFor = L"MRID6-23";

	BoardProduct = getBoardProduct();//主板

	size_t foundPos = BoardProduct.find(searchFor);
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