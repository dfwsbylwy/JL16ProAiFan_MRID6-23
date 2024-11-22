//#include "stdafx.h"
#include <stdio.h>
//#include <string.h>
#include <string>
#include "RunCommand.h"
#include <windows.h>
#include <thread>



// 描述:execmd函数执行命令，并将结果存储到result字符串数组中
// 参数:cmd表示要执行的命令,  result是执行的结果存储的字符串数组
// 函数执行成功返回1，失败返回0
#pragma warning(disable:4996)

std::string cmdPopen(const std::string& cmd) {
    char buffer[128]; //定义缓冲区
    FILE* pipe = _popen(cmd.c_str(), "r"); //打开管道，并执行命令
    if (!pipe)
        return 0; //返回0表示运行失败

    std::string ret;
    while (fgets(buffer, 128, pipe)) {
        ret += buffer;
    }
    _pclose(pipe); //关闭管道
    return ret;
}


/**
 * @brief	通过CreateProcess调用命令行并获取输出结果
 */
std::string cmdProcess(const std::string& cmdLine) {
	/* 创建匿名管道 */
	SECURITY_ATTRIBUTES _security = { 0 };
	_security.bInheritHandle = TRUE;
	_security.nLength = sizeof(_security);
	_security.lpSecurityDescriptor = NULL;
	HANDLE hRead = NULL, hWrite = NULL;
	if (!CreatePipe(&hRead, &hWrite, &_security, 0)) {
		printf("创建管道失败,error code=%d \n", GetLastError());
	}
	/* cmd命令行转换为Unicode编码 */
	int convLength = MultiByteToWideChar(CP_UTF8, 0, cmdLine.c_str(), (int)strlen(cmdLine.c_str()), NULL, 0);
	if (convLength <= 0) {
		printf("字符串转换长度计算出错\n");
	}
	std::wstring wCmdLine;
	wCmdLine.resize(convLength + 10);
	convLength = MultiByteToWideChar(CP_UTF8, 0, cmdLine.c_str(), (int)strlen(cmdLine.c_str()), &wCmdLine[0], (int)wCmdLine.size());
	if (convLength <= 0) {
		printf("字符串转换出错\n");
	}
	/* 创建新进程执行cmd命令并将结果写入到管道 */
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { 0 };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE; // 隐藏cmd执行的窗口
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	if (!CreateProcess(NULL,
		&wCmdLine[0],
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&si,
		&pi)) {
		printf("创建子进程失败,error code=%d \n", GetLastError());
	}
	/* 等待进程执行命令结束 */
	::WaitForSingleObject(pi.hThread, INFINITE);
	::WaitForSingleObject(pi.hProcess, INFINITE);
	/* 从管道中读取数据 */
	DWORD bufferLen = 10240;
	char* buffer = (char*)malloc(10240);
	memset(buffer, '\0', bufferLen);
	DWORD recLen = 0;
	if (!ReadFile(hRead, buffer, bufferLen, &recLen, NULL)) {
		printf("读取管道内容失败, error code=%d\n", GetLastError());
	}
	std::string ret(buffer);
	/* 关闭句柄 */
	CloseHandle(hRead);
	CloseHandle(hWrite);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	free(buffer);
	return ret;
}


void TcmdProcess(std::string cmdLine) {
	std::string sss = cmdLine;
	std::thread TcmdProcess(cmdProcess, sss);
	TcmdProcess.detach();
}
