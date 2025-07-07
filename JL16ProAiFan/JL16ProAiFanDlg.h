
// JL16ProAiFanDlg.h: 头文件
//
#pragma once
#include "CFanControl.h"
//#include "AMDSMUDragon.h"
#include "ryzenadj.h"
#include <regex>




// CJL16ProAiFanDlg 对话框
class CJL16ProAiFanDlg : public CDialogEx
{
// 构造
public:
	CJL16ProAiFanDlg(CWnd* pParent = nullptr);	// 标准构造函数
	static CJL16ProAiFanDlg* pActiveInstance; // 静态指针，指向活动实例
	// 存储当前执行文件的路径
	CString szDirectory;
	CString iniPatn;
	//LPCWSTR iniPatn;
	void getszDirectory();

	//最小化到托盘处理
	void InitTrayIcon();

	afx_msg LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);


	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);


	// 托盘图标处理
	NOTIFYICONDATA m_nid;
	HICON m_hTrayIcon; // 托盘图标句柄

	static bool UIUpdateFlag;


	afx_msg void OnClose();
	void OnRecover();
	void OnExit();


	static CFanControl CFC;
	LRESULT OnUpdateUI(WPARAM wParam, LPARAM lParam);

	//刷新界面信息显示
	//static UINT StartAiFanControl(LPVOID pParam);   // 改用OnTimer模式
	void OnTimer(UINT_PTR nIDEvent);


	void LoadTempSpeedTableFromTXT(LPCTSTR filePath);

	void PopulateListControl();

	static UINT DelayTask(LPVOID pParam);  // 异步启用线程3.5s后关闭主窗口，仅接受1个参数



	CProgressCtrl m_ProgressCtrl_CPUTemp;
	CProgressCtrl m_ProgressCtrl_CPUFanSpeed;
	CProgressCtrl m_ProgressCtrl_GPUTemp;
	CProgressCtrl m_ProgressCtrl_GPUFanSpeed;
	afx_msg void OnBnClickedBtnMode02();
	afx_msg void OnBnClickedBtnMode00();
	afx_msg void OnBnClickedBtnAifanreboot();
	afx_msg void OnBnClickedCheckFansetstatus();


	CListCtrl m_CListCtrl_TempSpeedTable;

	afx_msg void OnBnClickedMfccolorbuttonKeylight();
	CMFCColorButton m_CMFCColorButton_KeyLight;

	afx_msg void OnEnChangeEditGpumaxclock();
	int m_edit_GpuMaxClock;
	afx_msg void OnBnClickedCheckGpumaxclock();



	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	// m_SLIDER_GpuMaxClock
	CSliderCtrl m_SLIDER_GpuMaxClock;
	// m_SLIDER_CoreClockOffset
	CSliderCtrl m_SLIDER_CoreClockOffset;
	// m_SLIDER_MemoryClockOffset
	CSliderCtrl m_SLIDER_MemoryClockOffset;
	// m_SLIDER_TimerAiFanControl
	CSliderCtrl m_SLIDER_TimerAiFanControl;
	// m_SLIDER_MaxFanSpeedSet
	CSliderCtrl m_SLIDER_MaxFanSpeedSet;



	int m_GpuMaxClock;
	int m_CoreClockOffset;
	int m_MemoryClockOffset;
	int m_TimerAiFanControl;


	afx_msg void OnBnClickedCheckAutorun();

	int ReadRTXMaxPwr();

	bool FileExists(const std::string& filePath);

	// m_SLIDER_CpuMaxTemp
	CSliderCtrl m_SLIDER_CpuMaxTemp;
	// m_SLIDER_CPUFastPPT
	CSliderCtrl m_SLIDER_CPUFastPPT;
	// m_SLIDER_CPUTDC
	CSliderCtrl m_SLIDER_CPUTDC;

	std::map <libryzenadjTool, int> libryzenadjData;


// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_JL16PROAIFAN_DIALOG };
#endif


// 实现
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	CFont m_bFont;
};


CString VectorToCString(const std::vector<int>& vec);
