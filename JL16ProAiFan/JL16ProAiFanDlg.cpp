
// JL16ProAiFanDlg.cpp: 实现文件
#pragma once
#include "pch.h"
//#include "framework.h"
#include "JL16ProAiFan.h"
#include "JL16ProAiFanDlg.h"
#include "afxdialogex.h"
#include "auto_start_helper.h"
#include "RunCommand.h"
#include "NvApiOC.h"
//#include "ryzenadj.h"
#include <thread>
#include <chrono> //std::this_thread::sleep_for(std::chrono::milliseconds(100));
#include <format>
#include <atlstr.h>
#include "BoardProduct.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define TStartAiFanControl 1


#define WM_TRAYICON WM_USER + 100
#define WM_UPDATE_UI + 101
#define ID_MENU_RECOVER 2001
#define ID_MENU_CLOSE 2002



#define JL16ProAiFanINI iniPatn
//#define SwitchMaxFanSpeed1 R"(sudo run  .\JiaoLongWMI.exe Fan-SwitchMaxFanSpeed-1)"
#define SwitchMaxFanSpeed1 R"(.\JiaoLongWMI.exe Fan-SwitchMaxFanSpeed-1)"
//TcmdProcess(SwitchMaxFanSpeed1);


// 定义一个函数将std::vector<int>转换为CString
CString VectorToCString(const std::vector<int>& vec) {
	std::wstringstream wss;
	for (size_t i = 0; i < vec.size(); ++i) {
		if (i > 0) {
			wss << L", "; // 添加逗号和空格
		}
		wss << vec[i]; // 添加整数值
	}
	return CString(wss.str().c_str());
}





// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:

};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)

END_MESSAGE_MAP()


// CJL16ProAiFanDlg 对话框

CJL16ProAiFanDlg* CJL16ProAiFanDlg::pActiveInstance = nullptr;
CFanControl CJL16ProAiFanDlg::CFC = CFanControl();
bool CJL16ProAiFanDlg::UIUpdateFlag = true;

CJL16ProAiFanDlg::CJL16ProAiFanDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_JL16PROAIFAN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	pActiveInstance = this;
	m_hTrayIcon = NULL; // 托盘图标句柄
	m_edit_GpuMaxClock = 0;
	m_GpuMaxClock = 0;
	m_CoreClockOffset = 0;
	m_MemoryClockOffset = 0;
	m_TimerAiFanControl = 1000;

	//m_nid = NULL;
}

void CJL16ProAiFanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_CPUTemp, m_ProgressCtrl_CPUTemp);
	DDX_Control(pDX, IDC_PROGRESS_CPUFanSpeed, m_ProgressCtrl_CPUFanSpeed);
	DDX_Control(pDX, IDC_PROGRESS_GPUTemp, m_ProgressCtrl_GPUTemp);
	DDX_Control(pDX, IDC_PROGRESS_GPUFanSpeed, m_ProgressCtrl_GPUFanSpeed);
	DDX_Control(pDX, IDC_LIST_TempSpeedTable, m_CListCtrl_TempSpeedTable);
	DDX_Control(pDX, IDC_MFCCOLORBUTTON_KeyLight, m_CMFCColorButton_KeyLight);

	DDX_Text(pDX, IDC_EDIT_GpuMaxClock, m_edit_GpuMaxClock);
	//DDV_MinMaxInt(pDX, m_edit_GpuMaxClock, 210, 2800);
	DDX_Control(pDX, IDC_SLIDER_GpuMaxClock, m_SLIDER_GpuMaxClock);
	DDX_Control(pDX, IDC_SLIDER_CoreClockOffset, m_SLIDER_CoreClockOffset);
	DDX_Control(pDX, IDC_SLIDER_MemoryClockOffset, m_SLIDER_MemoryClockOffset);
	DDX_Control(pDX, IDC_SLIDER_TimerAiFanControl, m_SLIDER_TimerAiFanControl);
	DDX_Control(pDX, IDC_SLIDER_CpuMaxTemp, m_SLIDER_CpuMaxTemp);
	DDX_Control(pDX, IDC_SLIDER_CPUFastPPT, m_SLIDER_CPUFastPPT);
	DDX_Control(pDX, IDC_SLIDER_CPUTDC, m_SLIDER_CPUTDC);
}

BEGIN_MESSAGE_MAP(CJL16ProAiFanDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_TIMER()

	ON_MESSAGE(WM_TRAYICON, OnTrayIcon)
	ON_BN_CLICKED(ID_MENU_RECOVER, &CJL16ProAiFanDlg::OnRecover)
	ON_BN_CLICKED(ID_MENU_CLOSE, &CJL16ProAiFanDlg::OnExit)
	ON_MESSAGE(WM_UPDATE_UI, &CJL16ProAiFanDlg::OnUpdateUI)
	ON_BN_CLICKED(IDC_BTN_Mode02, &CJL16ProAiFanDlg::OnBnClickedBtnMode02)
	ON_BN_CLICKED(IDC_BTN_Mode00, &CJL16ProAiFanDlg::OnBnClickedBtnMode00)
	ON_BN_CLICKED(IDC_BTN_AiFanReboot, &CJL16ProAiFanDlg::OnBnClickedBtnAifanreboot)
	ON_BN_CLICKED(IDC_CHECK_FanSetStatus, &CJL16ProAiFanDlg::OnBnClickedCheckFansetstatus)
	ON_BN_CLICKED(IDC_MFCCOLORBUTTON_KeyLight, &CJL16ProAiFanDlg::OnBnClickedMfccolorbuttonKeylight)

	ON_EN_CHANGE(IDC_EDIT_GpuMaxClock, &CJL16ProAiFanDlg::OnEnChangeEditGpumaxclock)
	ON_BN_CLICKED(IDC_CHECK_GpuMaxClock, &CJL16ProAiFanDlg::OnBnClickedCheckGpumaxclock)
	ON_WM_HSCROLL()

	ON_BN_CLICKED(IDC_CHECK_AutoRun, &CJL16ProAiFanDlg::OnBnClickedCheckAutorun)
END_MESSAGE_MAP()


// CJL16ProAiFanDlg 消息处理程序

BOOL CJL16ProAiFanDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	getszDirectory();
	LoadTempSpeedTableFromTXT(_T("UserTempSpeedTable.txt")); // 假设TXT文件名为NewTempSpeedTable.txt，位于可执行文件同级目录下
	PopulateListControl();
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	// TODO: 在此添加额外的初始化代码
	m_ProgressCtrl_CPUTemp.SetRange(50, 100); // 设置进度条范围从0到100
	m_ProgressCtrl_CPUTemp.SetPos(0);         // 初始位置设为0

	m_ProgressCtrl_GPUTemp.SetRange(50, 100); // 设置进度条范围从0到100
	m_ProgressCtrl_GPUTemp.SetPos(0);         // 初始位置设为0

	m_ProgressCtrl_CPUFanSpeed.SetRange(1500, 5000); // 设置进度条范围从0到100
	m_ProgressCtrl_CPUFanSpeed.SetPos(0);         // 初始位置设为0

	m_ProgressCtrl_GPUFanSpeed.SetRange(1500, 5000); // 设置进度条范围从0到100
	m_ProgressCtrl_GPUFanSpeed.SetPos(0);         // 初始位置设为0

	InitTrayIcon(); // 初始化托盘图标


	//检查主板是否为蛟龙16Pro，并按返回值选择分支。
	if (isJL16Pro()) {
		{
			//初始化为游戏模式
			TcmdProcess(SetPerformaceMode0);
			CFanControl::FCEC.writeByte(ModeAddress, GameMode);

			// 初始化颜色按钮的键盘灯颜色
			int r, g, b;
			TCHAR szValue[256] = TEXT("");
			//GetPrivateProfileString(_T("config"), _T("m_MemoryClockOffset"), _T("0,0,0"), szValue, MAX_PATH, JL16ProAiFanINI);
			GetPrivateProfileString(_T("config"), _T("KeylightRGB"), _T("0,0,0"), szValue, _countof(szValue), JL16ProAiFanINI);
			if (_stscanf_s(szValue, _T("%d, %d, %d"), &r, &g, &b) != 3)
			{
				// 处理错误
				int KeyLightRGB = CFanControl::FCEC.readDword(KeyLight2RGB);
				m_CMFCColorButton_KeyLight.SetColor(RGB((KeyLightRGB >> 8) & 0xFF, (KeyLightRGB >> 16) & 0xFF, (KeyLightRGB >> 24) & 0xFF));
			}
			else {
				m_CMFCColorButton_KeyLight.SetColor(RGB(r, g, b));
			}

			// 初始化m_FanSetStatus
			GetPrivateProfileString(_T("config"), _T("m_FanSetStatus"), _T("true"), szValue, _countof(szValue), JL16ProAiFanINI);

			if (_tcscmp(szValue, _T("true")) == 0)
			{
				CFanControl::m_FanSetStatus = TRUE;
				CheckDlgButton(IDC_CHECK_FanSetStatus, BST_CHECKED);
			}
			else
			{
				CFanControl::m_FanSetStatus = FALSE;
				CheckDlgButton(IDC_CHECK_FanSetStatus, BST_UNCHECKED);
			}

			// 初始化auto_run
			GetPrivateProfileString(_T("config"), _T("auto_run"), _T("true"), szValue, _countof(szValue), JL16ProAiFanINI);

			if (_tcscmp(szValue, _T("true")) == 0)
			{

				CheckDlgButton(IDC_CHECK_AutoRun, BST_CHECKED);
			}
			else
			{
				CheckDlgButton(IDC_CHECK_AutoRun, BST_UNCHECKED);
			}
		}


		{
			m_SLIDER_GpuMaxClock.SetRange(1155, 2100);//设置范围
			m_SLIDER_GpuMaxClock.SetTicFreq(15);//设置显示刻度的间隔
			m_SLIDER_GpuMaxClock.SetLineSize(15);//设置行大小
			m_SLIDER_GpuMaxClock.SetPageSize(30);//设置页大小

			m_SLIDER_CoreClockOffset.SetRange(0, 250);//设置范围
			m_SLIDER_CoreClockOffset.SetTicFreq(50);//设置显示刻度的间隔
			m_SLIDER_CoreClockOffset.SetLineSize(1);//设置行大小
			m_SLIDER_CoreClockOffset.SetPageSize(10);//设置页大小

			m_SLIDER_MemoryClockOffset.SetRange(0, 1000);//设置范围
			m_SLIDER_MemoryClockOffset.SetTicFreq(250);//设置显示刻度的间隔
			m_SLIDER_MemoryClockOffset.SetLineSize(1);//设置行大小
			m_SLIDER_MemoryClockOffset.SetPageSize(10);//设置页大小

			m_SLIDER_TimerAiFanControl.SetRange(5, 35);//设置范围
			m_SLIDER_TimerAiFanControl.SetTicFreq(5);//设置显示刻度的间隔
			m_SLIDER_TimerAiFanControl.SetLineSize(1);//设置行大小
			m_SLIDER_TimerAiFanControl.SetPageSize(5);//设置页大小


			m_SLIDER_CpuMaxTemp.SetRange(80, 100);//设置范围
			m_SLIDER_CpuMaxTemp.SetTicFreq(5);//设置显示刻度的间隔
			m_SLIDER_CpuMaxTemp.SetLineSize(1);//设置行大小
			m_SLIDER_CpuMaxTemp.SetPageSize(5);//设置页大小


			m_SLIDER_CPUFastPPT.SetRange(25, 120);//设置范围
			m_SLIDER_CPUFastPPT.SetTicFreq(5);//设置显示刻度的间隔
			m_SLIDER_CPUFastPPT.SetLineSize(1);//设置行大小
			m_SLIDER_CPUFastPPT.SetPageSize(5);//设置页大小


			m_SLIDER_CPUTDC.SetRange(18, 108);//设置范围
			m_SLIDER_CPUTDC.SetTicFreq(5);//设置显示刻度的间隔
			m_SLIDER_CPUTDC.SetLineSize(1);//设置行大小
			m_SLIDER_CPUTDC.SetPageSize(5);//设置页大小


			m_GpuMaxClock = GetPrivateProfileInt(_T("config"), _T("m_GpuMaxClock"), 0, JL16ProAiFanINI);
			SetDlgItemInt(IDC_EDIT_GpuMaxClock, m_GpuMaxClock, FALSE);

			m_CoreClockOffset = GetPrivateProfileInt(_T("config"), _T("m_CoreClockOffset"), 0, JL16ProAiFanINI);
			m_SLIDER_CoreClockOffset.SetPos(m_CoreClockOffset);//当前停留的位置
			SetDlgItemInt(IDC_STATIC_CoreClockOffset, m_CoreClockOffset, FALSE);

			m_MemoryClockOffset = GetPrivateProfileInt(_T("config"), _T("m_MemoryClockOffset"), 0, JL16ProAiFanINI);
			m_SLIDER_MemoryClockOffset.SetPos(m_MemoryClockOffset);//当前停留的位置
			SetDlgItemInt(IDC_STATIC_MemoryClockOffset, m_MemoryClockOffset, FALSE);
			//初始化CoreMemory超频
			if (m_CoreClockOffset == 0 || m_MemoryClockOffset == 0)
			{
				if (m_CoreClockOffset != 0)
				{
					NvApiSetCoreOC(m_CoreClockOffset);
				}
				if (m_MemoryClockOffset != 0)
				{
					NvApiSetMemoryOC(m_MemoryClockOffset);
				}
			}
			else
			{
				NvApiSetCoreMemoryOC(m_CoreClockOffset, m_MemoryClockOffset);
			}


			m_TimerAiFanControl = GetPrivateProfileInt(_T("config"), _T("m_TimerAiFanControl"), 1000, JL16ProAiFanINI);
			m_SLIDER_TimerAiFanControl.SetPos(m_TimerAiFanControl / 100);//当前停留的位置

			SetTimer(TStartAiFanControl, m_TimerAiFanControl, NULL); //设置AiFan计时器

			CString  formattedString;
			formattedString.Format(_T("%.1f s"), m_TimerAiFanControl / 1000.0); // 保留一位小数
			SetDlgItemText(IDC_STATIC_TimerAiFanControl, formattedString);

			
			libryzenadjData[libryzenadjTool::set_tctl_temp] = GetPrivateProfileInt(_T("libryzenadj"), _T("libryzenadjTool::set_tctl_temp"), 90, JL16ProAiFanINI);
			m_SLIDER_CpuMaxTemp.SetPos(libryzenadjData[libryzenadjTool::set_tctl_temp]);//当前停留的位置
			SetDlgItemInt(IDC_STATIC_CpuMaxTemp, libryzenadjData[libryzenadjTool::set_tctl_temp], FALSE);

			libryzenadjData[libryzenadjTool::set_cpu_fppt] = GetPrivateProfileInt(_T("libryzenadj"), _T("libryzenadjTool::set_cpu_fppt"), 120000, JL16ProAiFanINI);
			if (libryzenadjData[libryzenadjTool::set_cpu_fppt] >= 25000 && libryzenadjData[libryzenadjTool::set_cpu_fppt] < 120000) {
				m_SLIDER_CPUFastPPT.SetPos(libryzenadjData[libryzenadjTool::set_cpu_fppt] / 1000);//当前停留的位置
				SetDlgItemInt(IDC_STATIC_CPUFastPPT, libryzenadjData[libryzenadjTool::set_cpu_fppt] / 1000, FALSE);
			}
			else {
				m_SLIDER_CPUFastPPT.SetPos(120000 / 1000);//当前停留的位置
				SetDlgItemInt(IDC_STATIC_CPUFastPPT, 120000 / 1000, FALSE);
				libryzenadjData.erase(libryzenadjTool::set_cpu_fppt);
			}
				
			libryzenadjData[libryzenadjTool::set_cpu_TDC] = GetPrivateProfileInt(_T("libryzenadj"), _T("libryzenadjTool::set_cpu_TDC"), 108000, JL16ProAiFanINI);
			if (libryzenadjData[libryzenadjTool::set_cpu_TDC] >= 18000 && libryzenadjData[libryzenadjTool::set_cpu_TDC] < 108000) {
				m_SLIDER_CPUTDC.SetPos(libryzenadjData[libryzenadjTool::set_cpu_TDC] / 1000);//当前停留的位置
				SetDlgItemInt(IDC_STATIC_CPUTDC, libryzenadjData[libryzenadjTool::set_cpu_TDC] / 1000, FALSE);
			}
			else {
				m_SLIDER_CPUTDC.SetPos(108000 / 1000);//当前停留的位置
				SetDlgItemInt(IDC_STATIC_CPUTDC, 108000 / 1000, FALSE);
				libryzenadjData.erase(libryzenadjTool::set_cpu_TDC);
			}


			//TCHAR szCpuOffset[256] = TEXT("0, -2,  -3  , -4  , -0, 0, -3, -4 , -1 , -2, -3, -4, -1, -2, -3, -4");
			TCHAR szCpuOffset[256] = TEXT("");
			GetPrivateProfileString(_T("libryzenadj"), _T("libryzenadjTool::set_coper"), _T(""), szCpuOffset, _countof(szCpuOffset), JL16ProAiFanINI);
			// 转换为 std::wstring
			std::wstring wstrCpuOffset(szCpuOffset);

			// Remove spaces from the string
			std::wstring wstrCpuOffsetNoSpaces;
			for (wchar_t ch : wstrCpuOffset) {
				if (!std::isspace(ch)) {
					wstrCpuOffsetNoSpaces += ch;
				}
			}
			// 使用 std::wistringstream 来解析字符串
			std::wistringstream iss(wstrCpuOffsetNoSpaces);

			// 创建一个 std::vector<int> 来存储转换后的整数
			std::vector<int> cpuOffset;

			int val;
			while (iss >> val) {  // std::ws skips leading whitespace
				cpuOffset.push_back(val);

				// Check the next character
				wchar_t nextChar = iss.peek();
				while (nextChar == L',' ) {
					iss.ignore();  // Ignore comma and whitespace
					nextChar = iss.peek();
				}
			}

			libryzenadjData[libryzenadjTool::cpu_core_num] = cpuOffset.size();
			if (libryzenadjData[libryzenadjTool::cpu_core_num] == 8 || libryzenadjData[libryzenadjTool::cpu_core_num] == 16) {
				// 输出结果
				SetDlgItemText(IDC_STATIC_CPUOffSet, VectorToCString(cpuOffset));

				//  缩小显示大小
				//m_bFont.CreatePointFont(170, _T("楷体"));
				m_bFont.CreatePointFont(91, _T("Microsoft Sans Serif"));
				//m_bFont.CreateFont(-24,                        // nHeight
				//	0,                         // nWidth
				//	0,                         // nEscapement
				//	0,                         // nOrientation
				//	FW_BOLD,                 // nWeight
				//	FALSE,                     // bItalic
				//	FALSE,                     // bUnderline
				//	0,                         // cStrikeOut
				//	134,              // nCharSet
				//	OUT_DEFAULT_PRECIS,        // nOutPrecision
				//	CLIP_DEFAULT_PRECIS,       // nClipPrecision
				//	DEFAULT_QUALITY,           // nQuality
				//	DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
				//	_T("楷体_GB2312"));


				GetDlgItem(IDC_STATIC_CPUOffSet)->SetFont(&m_bFont);


				libryzenadjTool Coreid = libryzenadjTool::set_coper00;
				for (int offset : cpuOffset) {
					//libryzenadjData[Coreid] = offset;
					if (offset != 0)
					{
						libryzenadjData[Coreid] = offset;
					}
					// 自增操作
					Coreid = static_cast<libryzenadjTool>(static_cast<int>(Coreid) + 1);
				}
			}

			//ryzenadj2do(libryzenadjData);
			std::thread ryzenadjdoing(ryzenadj2do, std::ref(libryzenadjData));
			ryzenadjdoing.detach();
			
			TcmdProcess(SwitchMaxFanSpeed1);
		}
	}
	else {
		MessageBoxTimeout(NULL, _T("Bios信息不含MRID6_23，该机型不是蛟龙16Pro，程序退出！！！\n如果确定为蛟龙16Pro，重试后仍不行，请联系开发者QQ157067422。"), _T("JL16ProAiFan"), MB_ICONEXCLAMATION, 0, 2100);
	
		EndDialog(IDCANCEL);

	}

	//更新界面
	::PostMessage(*CJL16ProAiFanDlg::pActiveInstance, WM_UPDATE_UI, 0, 0);// 发送消息到UI线程


#ifdef NDEBUG
	AfxBeginThread(CloseMainWM, this); // 软件打开后，异步启用线程3.5s后关闭主窗口
#endif
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CJL16ProAiFanDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	//WM_ENDSESSION
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else 
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}

	if (nID == SC_MINIMIZE)
	{
		ShowWindow(FALSE);//隐藏窗口
	}

}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CJL16ProAiFanDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CJL16ProAiFanDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CJL16ProAiFanDlg::getszDirectory()
{
	TCHAR szExeFilePath[MAX_PATH];
	if (GetModuleFileName(NULL, szExeFilePath, MAX_PATH) != 0)
	{
		// 获取当前执行文件的路径并去除文件名以得到目录路径
		//TCHAR szDirectory[MAX_PATH];
		_tcsrchr(szExeFilePath, '\\')[1] = '\0'; // 将路径中的最后一个'\'之后的字符替换为'\0'，从而得到目录路径
		//_tcscpy_s(szDirectory, szExeFilePath); // 复制路径到szDirectory
			// 将 TCHAR 数组转换为 CString
		CString strszDirectory(szExeFilePath); // 使用构造函数
		szDirectory = strszDirectory;
		iniPatn = strszDirectory + _T("JL16ProAiFan.ini");
		//// 构建 INI 文件的完整路径
		//TCHAR szIniFile[MAX_PATH];
		//_tcscpy_s(szIniFile, MAX_PATH, szDirectory);
		//_tcscat_s(szIniFile, MAX_PATH, _T("JL16ProAiFan.ini"));
		//iniPatn = reinterpret_cast<LPCWSTR>(szIniFile);


	}
	else {
		szDirectory = _T("0");
		iniPatn = _T(".\\JL16ProAiFan.ini");
	}
}

// 初始化托盘图标
void CJL16ProAiFanDlg::InitTrayIcon()
{
	m_hTrayIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME); // IDI_TRAYICON 是资源中的托盘图标ID
	ZeroMemory(&m_nid, sizeof(NOTIFYICONDATA));
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDR_MAINFRAME; // 图标ID
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_nid.uCallbackMessage = WM_TRAYICON; // 自定义消息处理托盘图标事件
	m_nid.hIcon = m_hTrayIcon;
	lstrcpy(m_nid.szTip, _T("JL16ProAiFan")); // 提示文本
	Shell_NotifyIcon(NIM_ADD, &m_nid); // 添加托盘图标
}


// 处理托盘图标消息
LRESULT CJL16ProAiFanDlg::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{

	if (wParam != IDR_MAINFRAME)
		return 1;

	switch (lParam)
	{
		case WM_LBUTTONDBLCLK: // 双击左键的处理
		{
			if(CJL16ProAiFanDlg::UIUpdateFlag)
			{	
				OnClose();
			}
			else 
			{
				OnRecover();
			}
			break;
		}
		case WM_RBUTTONUP: // 右键起来时弹出菜单
		{

			POINT point;
			GetCursorPos(&point);//检索鼠标光标的位置（以屏幕坐标为单位）

			// 创建并显示菜单
			CMenu menu;
			menu.CreatePopupMenu();//创建弹出式菜单
			menu.AppendMenuW(MF_STRING, ID_MENU_RECOVER, L"恢复窗口");//添加菜单项
			menu.AppendMenuW(MF_STRING, ID_MENU_CLOSE, L"关闭软件");
			SetForegroundWindow();//托盘菜单焦点控制
			// 显示菜单，这里使用 TPM_LEFTALIGN 仅作示例，根据需要调整
			menu.TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this);
			PostMessage(WM_COMMAND, 0, 0); // 发送WM_COMMAND结束菜单

			break;
		}

	
	}

	return 0;
}



LRESULT CJL16ProAiFanDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	switch (message)
	{
	case WM_QUERYENDSESSION:            //关机、重启、注销
	{
		if (lParam == ENDSESSION_LOGOFF)//注销
		{
			OnExit();

		}
		else   //重启/关机
		{
			OnExit();
		}
	}
	break;
	case WM_POWERBROADCAST:
	{
		switch (wParam)
		{
		case PBT_APMSUSPEND:         //睡眠、休眠
		{
			//OnExit();
			//KillTimer(TStartAiFanControl);
		}
		break;
		case PBT_APMRESUMEAUTOMATIC: //睡眠、休眠恢复
		{
			//OnExit();
			std::this_thread::sleep_for(std::chrono::milliseconds(3500));

			CFC.UpdateFanSpeed(); //重新更新风扇转速
			if (CFanControl::m_CPUFanSpeed < 1900 || CFanControl::m_GPUFanSpeed < 1900)
			{
				TcmdProcess(SetPerformaceMode0);
				CFanControl::FCEC.writeByte(ModeAddress, GameMode);
				CFanControl::m_ModeSet = GameMode;
				std::this_thread::sleep_for(std::chrono::milliseconds(3500));
				CFC.UpdateFanSpeed(); //重新更新风扇转速
			}

		}
		break;
		}
	}
	break;
	}

	return CDialogEx::WindowProc(message, wParam, lParam);
}


void CJL16ProAiFanDlg::OnClose()
{
	//CDialogEx::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	// 如果用户点击关闭按钮，最小化到托盘而不是直接关闭
	ShowWindow(SW_HIDE); //最小化到托盘
	CJL16ProAiFanDlg::UIUpdateFlag = false;
}


void CJL16ProAiFanDlg::OnRecover()
{
	// TODO: 在此处添加实现代码.
	ShowWindow(SW_NORMAL); // 显示主窗口
	SetForegroundWindow();// 置为前台窗口
	CJL16ProAiFanDlg::UIUpdateFlag = true;
}

void CJL16ProAiFanDlg::OnExit()
{

	KillTimer(TStartAiFanControl);

	//if (CFanControl::m_FanSetStatus == true && CFanControl::m_ModeSet >= 2)
	if ( CFanControl::m_ModeSet >= 2)
	{
		TcmdProcess(SetPerformaceMode0);
		CFanControl::FCEC.writeByte(ModeAddress, GameMode);//程序退出，强制写回办公mode
		CFanControl::FCEC.writeByte(MaxFanSpeedAddress, 35);
	}
	EndDialog(IDCANCEL);
}



LRESULT CJL16ProAiFanDlg::OnUpdateUI(WPARAM wParam, LPARAM lParam)
{
	//数据更新
	// 将变量转换为CString
	CString str;

	// 更新静态文本的值
	SetDlgItemInt(IDC_STATIC_CPUTemp, CFanControl::m_CPUTemp, FALSE);
	SetDlgItemInt(IDC_STATIC_GPUTemp, CFanControl::m_GPUTemp, FALSE);
	SetDlgItemInt(IDC_STATIC_CPUFanSpeed, CFanControl::m_CPUFanSpeed, FALSE);
	SetDlgItemInt(IDC_STATIC_GPUFanSpeed, CFanControl::m_GPUFanSpeed, FALSE);

	m_ProgressCtrl_CPUTemp.SetPos(CFanControl::m_CPUTemp);
	m_ProgressCtrl_GPUTemp.SetPos(CFanControl::m_GPUTemp);
	m_ProgressCtrl_CPUFanSpeed.SetPos(CFanControl::m_CPUFanSpeed);
	m_ProgressCtrl_GPUFanSpeed.SetPos(CFanControl::m_GPUFanSpeed);

	//std::string formatted = std::format("最高温度℃ = {}\n最大转速 *100 = {}\nModeSet = {:X}\nSteps = {}",\
	//	         CFanControl::m_MaxTemp, CFanControl::m_MaxFanSpeedSet, CFanControl::m_ModeSet, CFanControl::m_Steps);
	std::string formatted = std::format("{}\n{}\n{:X}\n{}",\
		         CFanControl::m_MaxTemp, CFanControl::m_MaxFanSpeedSet, CFanControl::m_ModeSet, CFanControl::m_Steps);
	str = formatted.c_str();
	SetDlgItemText(IDC_STATIC_FanControlInfo, str);


	return 0;
}



void CJL16ProAiFanDlg::OnTimer(UINT_PTR nIDEvent)
{


	switch (nIDEvent)
	{
		case TStartAiFanControl: // 假设IDT_TIMER1是你为某个特定任务设置的定时器ID
		{
			CFC.UpdateTemp();     //更新平台温度
			CFC.SetMaxFanSpeed(CJL16ProAiFanDlg::UIUpdateFlag); //更新风扇控制

			if (CJL16ProAiFanDlg::UIUpdateFlag)
			{
				CFC.UpdateFanSpeed(); //更新风扇转速
				CFC.CheckFanSpeedZero();
				::PostMessage(*CJL16ProAiFanDlg::pActiveInstance, WM_UPDATE_UI, 0, 0);// 发送消息到UI线程
			}
			else {
				CFC.CheckFanSpeedZero();
				if (CFanControl::m_FanSpeedZero)
				{
					CFC.UpdateFanSpeed(); //更新风扇转速
				}
			}


			CFanControl::m_Steps++;

			break;
		}
		//case IDT_TIMER2: // 另一个定时器ID
		//{
		//	// 执行与IDT_TIMER2相关的操作
		//	// ...
		//	break;
		// // 添加定时器消息处理函数
		//}
		// 可以添加更多case分支以处理其他定时器
		default:
			// 如果nIDEvent不匹配任何已知的定时器ID，可以在这里处理或忽略
			break;
	}





#ifdef _DEBUG
	if (CFanControl::m_Steps % 35 == 0) //35秒试用版
	{   //OnExit();

		CFanControl::m_FanSetStatus = FALSE;
		//WritePrivateProfileString(_T("config"), _T("m_FanSetStatus"), _T("false"), JL16ProAiFanINI);
		if(CFanControl::FCEC.writeByte(ModeAddress, QuietMode)) // 强制写回办公mode
			CFanControl::m_ModeSet = QuietMode;
		if (CFanControl::FCEC.writeByte(MaxFanSpeedAddress, 22))
			CFanControl::m_MaxFanSpeedSet = 22;
		CheckDlgButton(IDC_CHECK_FanSetStatus, BST_UNCHECKED); // 设置IDC_CHECK_FanSetStatus取消选中
	}
#endif

}



void CJL16ProAiFanDlg::OnBnClickedBtnMode02()
{
	CFanControl::m_FanSetStatus = FALSE;
	CheckDlgButton(IDC_CHECK_FanSetStatus, BST_UNCHECKED);
	WritePrivateProfileString(_T("config"), _T("m_FanSetStatus"), _T("false"), JL16ProAiFanINI);
	TcmdProcess(SetPerformaceMode2);
	CFanControl::FCEC.writeByte(ModeAddress, QuietMode);
	CFanControl::m_ModeSet = QuietMode;
	if (CFanControl::FCEC.writeByte(MaxFanSpeedAddress, 22))
		CFanControl::m_MaxFanSpeedSet = 22;
}


void CJL16ProAiFanDlg::OnBnClickedBtnMode00()
{
	CFanControl::m_FanSetStatus = FALSE;
	CheckDlgButton(IDC_CHECK_FanSetStatus, BST_UNCHECKED);
	WritePrivateProfileString(_T("config"), _T("m_FanSetStatus"), _T("false"), JL16ProAiFanINI);
	TcmdProcess(SetPerformaceMode0);
	CFanControl::FCEC.writeByte(ModeAddress, GameMode);
	CFanControl::m_ModeSet = GameMode;
	if (CFanControl::FCEC.writeByte(MaxFanSpeedAddress, 35))
		CFanControl::m_MaxFanSpeedSet = 35;
}


void CJL16ProAiFanDlg::OnBnClickedBtnAifanreboot()
{
	CFanControl::m_FanSetStatus = TRUE;
	WritePrivateProfileString(_T("config"), _T("m_FanSetStatus"), _T("true"), JL16ProAiFanINI);
	CheckDlgButton(IDC_CHECK_FanSetStatus, BST_CHECKED);
	CFanControl::m_MaxFanSpeedSet = -1;
	TcmdProcess(SetPerformaceMode0);
	CFanControl::FCEC.writeByte(ModeAddress, GameMode);
	CFanControl::m_ModeSet = GameMode;
	TcmdProcess(SwitchMaxFanSpeed1);
}



void CJL16ProAiFanDlg::OnBnClickedCheckFansetstatus()
{

	// 获取状态
	int nCheckState = ((CButton*)GetDlgItem(IDC_CHECK_FanSetStatus))->GetCheck();
	// 使用switch结构处理不同状态
	switch (nCheckState)
	{
		case BST_CHECKED:
		{
			// 处理选中状态的逻辑
			// 注意：此处实际上不需要再次设置为选中，因为已经是选中状态
			// ((CButton*)GetDlgItem(IDC_CHECK_FanSetStatus))->SetCheck(BST_CHECKED);
			CFanControl::m_FanSetStatus = TRUE;
			WritePrivateProfileString(_T("config"), _T("m_FanSetStatus"), _T("true"), JL16ProAiFanINI);
			CFanControl::m_MaxFanSpeedSet = -1;

			TcmdProcess(SwitchMaxFanSpeed1);
			break;
		}
		case BST_UNCHECKED:
		{
			// 如果需要在此处处理未选中状态的逻辑，可以添加代码
			CFanControl::m_FanSetStatus = FALSE;
			WritePrivateProfileString(_T("config"), _T("m_FanSetStatus"), _T("false"), JL16ProAiFanINI);
			TcmdProcess(SetPerformaceMode2);
			CFanControl::FCEC.writeByte(ModeAddress, QuietMode);//程序退出，强制写回办公mode
			CFanControl::m_ModeSet = QuietMode;
			if (CFanControl::FCEC.writeByte(MaxFanSpeedAddress, 22))
				CFanControl::m_MaxFanSpeedSet = 22;
			break;
		}
		// BST_INDETERMINATE状态通常不处理，或者根据需要添加处理逻辑
		default:
			break;
	}


}


// 在CJL16ProAiFanDlg类中添加以下方法
void CJL16ProAiFanDlg::LoadTempSpeedTableFromTXT(LPCTSTR fileName)
{
	if (_tcscmp(szDirectory, _T("0")) != 0)
	{
		// 拼接得到TXT文件的完整路径
		CString txtFilePath = szDirectory;
		txtFilePath += _T("\\");
		txtFilePath += fileName;

		std::ifstream file(txtFilePath.GetString());
		//std::vector<std::vector<short int>> CFanControl::m_TempSpeedTable = { {70,22}, {78,24}, {84,26}, {88,28}, {90,30}, {93,35}, {97,42},{100,49} };
		if (!file.is_open())
		{
			AfxMessageBox(_T("无法打开文件同目录下“UserTempSpeedTable.txt”，来加载自定义风扇曲线数据！\
请创建该文件，然后从低温到高温逐行输入2个整数并用空格隔开“温度值 最大转速值/100”，确保你已检查数据，示例如下：\n\
70 22\n78 24\n84 26\n88 28\n90 30\n93 35\n97 42\n100 49\n"));
//			MessageBoxTimeout(NULL, _T("无法打开文件同目录下“UserTempSpeedTable.txt”，来加载自定义风扇曲线数据！\
//请创建该文件，然后从低温到高温逐行输入2个整数并用空格隔开“温度值 最大转速值/100”，确保你已检查数据，示例如下：\n\
//70 22\n78 24\n84 26\n88 28\n90 30\n93 35\n97 42\n100 49\n"), _T("JL16ProAiFan"), MB_ICONEXCLAMATION, 0, 2100);
			return;
		}


		CFanControl::m_TempSpeedTable.clear(); // 清空原有数据

        std::vector<short int> rowData;
        short int value;
        while (file >> value)
        {
            rowData.push_back(value);
            if (rowData.size() == 2) // 假设每行有2个整数，代表温度和转速
            {
                CFanControl::m_TempSpeedTable.push_back(rowData);
                rowData.clear(); // 读完一行后清空，准备读取下一行
            }

        }

		if (!rowData.empty()) // 防止最后一行数据不足两个数未被处理的情况
		{
			// 根据实际情况处理最后一行数据不完整的情况，这里简单忽略或添加逻辑处理
			// CFanControl::m_TempSpeedTable.push_back(rowData); // 如果最后一行数据也应被加入，可以取消注释这行
		}
	}
	else
	{
		AfxMessageBox(_T("无法获取可执行文件路径！"));
		//MessageBoxTimeout(NULL, _T("无法获取可执行文件路径！"), _T("JL16ProAiFan"), MB_ICONEXCLAMATION, 0, 2100);
	}
}


// 显示m_TempSpeedTable数据到CListCtrl控件
void CJL16ProAiFanDlg::PopulateListControl()
{

	// 初始化List Control，如果尚未初始化，设置视图模式、列数等
	m_CListCtrl_TempSpeedTable.SetView(LV_VIEW_DETAILS); // 设置为详细视图
	// 设置列表控件的风格
	DWORD dwStyle = m_CListCtrl_TempSpeedTable.GetExtendedStyle();
	dwStyle |= LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	m_CListCtrl_TempSpeedTable.SetExtendedStyle(dwStyle);


	m_CListCtrl_TempSpeedTable.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 50);
	m_CListCtrl_TempSpeedTable.InsertColumn(1, _T("温度"), LVCFMT_CENTER, 50);
	m_CListCtrl_TempSpeedTable.InsertColumn(2, _T("转速*100"), LVCFMT_CENTER, 80);

	//// 假设TempSpeedTable已定义并初始化
	//std::vector<std::vector< short int>> TempSpeedTable = { {70,22}, {80,24}, {85,26}, {87,28},{89,30},{95,35},{98,42} };

	// 遍历TempSpeedTable并添加到List Control
	for (int i = 0; i < CFanControl::m_TempSpeedTable.size(); ++i)
	{
		int nItem = m_CListCtrl_TempSpeedTable.InsertItem(i, _T(""));
		m_CListCtrl_TempSpeedTable.SetItemText(nItem, 0, std::to_wstring(i).c_str());
		m_CListCtrl_TempSpeedTable.SetItemText(nItem, 1, std::to_wstring(CFanControl::m_TempSpeedTable[i][0]).c_str());
		m_CListCtrl_TempSpeedTable.SetItemText(nItem, 2, std::to_wstring(CFanControl::m_TempSpeedTable[i][1]).c_str());
	}
}

UINT CJL16ProAiFanDlg::CloseMainWM(LPVOID pParam)  //软件打开后，异步关闭主窗口
{
		// 类型转换回CJL16ProAiFanDlg*
		CJL16ProAiFanDlg* pDlg = static_cast<CJL16ProAiFanDlg*>(pParam);

		std::this_thread::sleep_for(std::chrono::milliseconds(3500));//软件开启3.5秒后最小化到托盘
		pDlg->ShowWindow(SW_HIDE);
		CJL16ProAiFanDlg::UIUpdateFlag = false;


		return TRUE;
}

void CJL16ProAiFanDlg::OnBnClickedMfccolorbuttonKeylight()//IDC_MFCCOLORBUTTON_KeyLight
{
	// 初始化颜色按钮的键盘灯颜色
	COLORREF color = m_CMFCColorButton_KeyLight.GetColor();

	// 使用Windows API宏来获取RGB分量
	BYTE r = GetRValue(color);    // 获取红色分量
	BYTE g = GetGValue(color);    // 获取绿色分量
	BYTE b = GetBValue(color);    // 获取蓝色分量

	if (CFanControl::FCEC.writeByte(KeyLightR, r) && CFanControl::FCEC.writeByte(KeyLightG, g) && CFanControl::FCEC.writeByte(KeyLightB, b))
	{
		std::wstring result = std::format(_T("{},{},{}"), r, g, b);
		WritePrivateProfileString(_T("config"), _T("KeylightRGB"), result.c_str(), JL16ProAiFanINI);
	}

}


void CJL16ProAiFanDlg::OnEnChangeEditGpumaxclock()
{

	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	UpdateData(TRUE);
	if (m_edit_GpuMaxClock >= 210 && m_edit_GpuMaxClock <= 2100)
	{
		//MessageBoxTimeout(NULL, str, _T("JL16ProAiFan"), MB_ICONEXCLAMATION, 0, 2100);
		std::string cmdLine = std::format("nvidia-smi -lgc 0,{}", m_edit_GpuMaxClock);
		std::string result = cmdProcess(cmdLine);
		WritePrivateProfileString(_T("config"), _T("m_GpuMaxClock"), std::to_wstring(m_edit_GpuMaxClock).c_str(), JL16ProAiFanINI);
		CheckDlgButton(IDC_CHECK_GpuMaxClock, BST_CHECKED); // 设置IDC_CHECK_GpuMaxClock被选中
	}
	else {

	}

	m_SLIDER_GpuMaxClock.SetPos(m_edit_GpuMaxClock);//当前停留的位置
	UpdateData(FALSE);

	// TODO:  在此添加控件通知处理程序代码
}


void CJL16ProAiFanDlg::OnBnClickedCheckGpumaxclock()
{
	// 获取状态
	int nCheckState = ((CButton*)GetDlgItem(IDC_CHECK_GpuMaxClock))->GetCheck();
	// 使用switch结构处理不同状态
	switch (nCheckState)
	{
	case BST_CHECKED:
	{
		// 处理选中状态的逻辑
		// 注意：此处实际上不需要再次设置为选中，因为已经是选中状态
		// ((CButton*)GetDlgItem(IDC_CHECK_GpuMaxClock))->SetCheck(BST_CHECKED);

		SetDlgItemInt(IDC_EDIT_GpuMaxClock, 1500, FALSE);
		//WritePrivateProfileString(_T("config"), _T("m_GpuMaxClock"), _T("1500"), JL16ProAiFanINI);
		break;
	}
	case BST_UNCHECKED:
	{
		// 如果需要在此处处理未选中状态的逻辑，可以添加代码
		// 但是示例中没有给出未选中时的具体操作，故此分支留空
		SetDlgItemInt(IDC_EDIT_GpuMaxClock, 0, FALSE);
		std::string cmdLine = "nvidia-smi -rgc";
		std::string result = cmdProcess(cmdLine);
		WritePrivateProfileString(_T("config"), _T("m_GpuMaxClock"), _T("0"), JL16ProAiFanINI);
		break;
	}
	// BST_INDETERMINATE状态通常不处理，或者根据需要添加处理逻辑
	default:
		break;
	}
}


void CJL16ProAiFanDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	std::map <libryzenadjTool, int> libryzenadjDataTmp;
	CString  formattedString;
	CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
	switch (pSlider->GetDlgCtrlID())
	{
		//Slider1
	case IDC_SLIDER_GpuMaxClock:
		UpdateData(TRUE);
		m_GpuMaxClock = m_SLIDER_GpuMaxClock.GetPos();
		UpdateData(FALSE);
		SetDlgItemInt(IDC_EDIT_GpuMaxClock, m_GpuMaxClock, FALSE);
		//WritePrivateProfileString(_T("config"), _T("m_GpuMaxClock"), std::to_wstring(m_GpuMaxClock).c_str(), JL16ProAiFanINI);
		break;

	case IDC_SLIDER_CoreClockOffset:
		UpdateData(TRUE);
		m_CoreClockOffset = m_SLIDER_CoreClockOffset.GetPos();//取得当前位置值  
		UpdateData(FALSE);
		NvApiSetCoreOC(m_CoreClockOffset);
		SetDlgItemInt(IDC_STATIC_CoreClockOffset, m_CoreClockOffset, FALSE);
		WritePrivateProfileString(_T("config"), _T("m_CoreClockOffset"), std::to_wstring(m_CoreClockOffset).c_str(), JL16ProAiFanINI);
		break;

	case IDC_SLIDER_MemoryClockOffset:
		UpdateData(TRUE);
		m_MemoryClockOffset = m_SLIDER_MemoryClockOffset.GetPos();//取得当前位置值  
		UpdateData(FALSE);
		NvApiSetMemoryOC(m_MemoryClockOffset);
		SetDlgItemInt(IDC_STATIC_MemoryClockOffset,m_MemoryClockOffset, FALSE);
		WritePrivateProfileString(_T("config"), _T("m_MemoryClockOffset"), std::to_wstring(m_MemoryClockOffset).c_str(), JL16ProAiFanINI);
		break;

	case IDC_SLIDER_TimerAiFanControl:
		UpdateData(TRUE);
		m_TimerAiFanControl = m_SLIDER_TimerAiFanControl.GetPos() * 100;//取得当前位置值  
		UpdateData(FALSE);

		KillTimer(TStartAiFanControl);
		SetTimer(TStartAiFanControl, m_TimerAiFanControl, NULL);

		formattedString.Format(_T("%.1f s"), m_TimerAiFanControl / 1000.0); // 保留一位小数
		SetDlgItemText(IDC_STATIC_TimerAiFanControl, formattedString);
		WritePrivateProfileString(_T("config"), _T("m_TimerAiFanControl"), std::to_wstring(m_TimerAiFanControl).c_str(), JL16ProAiFanINI);
		break;

	case IDC_SLIDER_CpuMaxTemp:
		UpdateData(TRUE);
		libryzenadjDataTmp = { {libryzenadjTool::set_tctl_temp, m_SLIDER_CpuMaxTemp.GetPos()} };
		UpdateData(FALSE);

		if (!ryzenadj2do(libryzenadjDataTmp)){
			SetDlgItemInt(IDC_STATIC_CpuMaxTemp, libryzenadjDataTmp[libryzenadjTool::set_tctl_temp], FALSE);
			WritePrivateProfileString(_T("libryzenadj"), _T("libryzenadjTool::set_tctl_temp"), std::to_wstring(libryzenadjDataTmp[libryzenadjTool::set_tctl_temp]).c_str(), JL16ProAiFanINI);
		}
		
		break;

	case IDC_SLIDER_CPUFastPPT:
		UpdateData(TRUE);
		libryzenadjDataTmp = { {libryzenadjTool::set_cpu_fppt, m_SLIDER_CPUFastPPT.GetPos() * 1000} };
		UpdateData(FALSE);

		if (!ryzenadj2do(libryzenadjDataTmp)){
			SetDlgItemInt(IDC_STATIC_CPUFastPPT, libryzenadjDataTmp[libryzenadjTool::set_cpu_fppt] / 1000, FALSE);
			WritePrivateProfileString(_T("libryzenadj"), _T("libryzenadjTool::set_cpu_fppt"), std::to_wstring(libryzenadjDataTmp[libryzenadjTool::set_cpu_fppt]).c_str(), JL16ProAiFanINI);
		}
		break;

	case IDC_SLIDER_CPUTDC:
		UpdateData(TRUE);
		libryzenadjDataTmp = { {libryzenadjTool::set_cpu_TDC, m_SLIDER_CPUTDC.GetPos() * 1000} };
		UpdateData(FALSE);

		if (!ryzenadj2do(libryzenadjDataTmp)){
			SetDlgItemInt(IDC_STATIC_CPUTDC, libryzenadjDataTmp[libryzenadjTool::set_cpu_TDC] / 1000, FALSE);
			WritePrivateProfileString(_T("libryzenadj"), _T("libryzenadjTool::set_cpu_TDC"), std::to_wstring(libryzenadjDataTmp[libryzenadjTool::set_cpu_TDC]).c_str(), JL16ProAiFanINI);
		}
		break;

	}


	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}





void CJL16ProAiFanDlg::OnBnClickedCheckAutorun()
{
	// 获取状态
	int nCheckState = ((CButton*)GetDlgItem(IDC_CHECK_AutoRun))->GetCheck();
	// 使用switch结构处理不同状态
	switch (nCheckState)
	{
	case BST_CHECKED:
	{
		create_auto_start_task_for_this_user(TRUE);
		WritePrivateProfileString(_T("config"), _T("auto_run"), _T("true"), JL16ProAiFanINI);
		break;
	}
	case BST_UNCHECKED:
	{
		// 如果需要在此处处理未选中状态的逻辑，可以添加代码
		// 但是示例中没有给出未选中时的具体操作，故此分支留空
		delete_auto_start_task_for_this_user();
		WritePrivateProfileString(_T("config"), _T("auto_run"), _T("false"), JL16ProAiFanINI);
		break;
	}
	// BST_INDETERMINATE状态通常不处理，或者根据需要添加处理逻辑
	default:
		break;
	}
}


int CJL16ProAiFanDlg::ReadRTXMaxPwr()
{
	std::string cmdLine = std::format("nvidia-smi");
	std::string result = cmdProcess(cmdLine);
	// 使用正则表达式匹配最大功耗的数字部分
	std::regex pattern(R"(/ +(\d+)W \|)");
	std::smatch match;

	// 查找匹配项
	if (std::regex_search(result, match, pattern)) {
		// 输出匹配的数字部分
		return std::stoi(match[1].str());
	}
	else {
		//std::cout << "未找到匹配的最大功耗值" << std::endl;
		return 80;
	}
}

