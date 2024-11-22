//#include "stdafx.h"
#include <stdio.h>
//#include <string.h>
#include <string>
#include "RunCommand.h"
#include <windows.h>
#include <thread>



// ����:execmd����ִ�������������洢��result�ַ���������
// ����:cmd��ʾҪִ�е�����,  result��ִ�еĽ���洢���ַ�������
// ����ִ�гɹ�����1��ʧ�ܷ���0
#pragma warning(disable:4996)

std::string cmdPopen(const std::string& cmd) {
    char buffer[128]; //���建����
    FILE* pipe = _popen(cmd.c_str(), "r"); //�򿪹ܵ�����ִ������
    if (!pipe)
        return 0; //����0��ʾ����ʧ��

    std::string ret;
    while (fgets(buffer, 128, pipe)) {
        ret += buffer;
    }
    _pclose(pipe); //�رչܵ�
    return ret;
}


/**
 * @brief	ͨ��CreateProcess���������в���ȡ������
 */
std::string cmdProcess(const std::string& cmdLine) {
	/* ���������ܵ� */
	SECURITY_ATTRIBUTES _security = { 0 };
	_security.bInheritHandle = TRUE;
	_security.nLength = sizeof(_security);
	_security.lpSecurityDescriptor = NULL;
	HANDLE hRead = NULL, hWrite = NULL;
	if (!CreatePipe(&hRead, &hWrite, &_security, 0)) {
		printf("�����ܵ�ʧ��,error code=%d \n", GetLastError());
	}
	/* cmd������ת��ΪUnicode���� */
	int convLength = MultiByteToWideChar(CP_UTF8, 0, cmdLine.c_str(), (int)strlen(cmdLine.c_str()), NULL, 0);
	if (convLength <= 0) {
		printf("�ַ���ת�����ȼ������\n");
	}
	std::wstring wCmdLine;
	wCmdLine.resize(convLength + 10);
	convLength = MultiByteToWideChar(CP_UTF8, 0, cmdLine.c_str(), (int)strlen(cmdLine.c_str()), &wCmdLine[0], (int)wCmdLine.size());
	if (convLength <= 0) {
		printf("�ַ���ת������\n");
	}
	/* �����½���ִ��cmd��������д�뵽�ܵ� */
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { 0 };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE; // ����cmdִ�еĴ���
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
		printf("�����ӽ���ʧ��,error code=%d \n", GetLastError());
	}
	/* �ȴ�����ִ��������� */
	::WaitForSingleObject(pi.hThread, INFINITE);
	::WaitForSingleObject(pi.hProcess, INFINITE);
	/* �ӹܵ��ж�ȡ���� */
	DWORD bufferLen = 10240;
	char* buffer = (char*)malloc(10240);
	memset(buffer, '\0', bufferLen);
	DWORD recLen = 0;
	if (!ReadFile(hRead, buffer, bufferLen, &recLen, NULL)) {
		printf("��ȡ�ܵ�����ʧ��, error code=%d\n", GetLastError());
	}
	std::string ret(buffer);
	/* �رվ�� */
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
