// Loader.cpp : �������̨Ӧ�ó������ڵ㡣
//  ��������Server.dll

#include "stdafx.h"
#include "Loader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ψһ��Ӧ�ó������

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	HMODULE hdll = LoadLibrary(_T("Server.dll"));
	if (!hdll)
	{
		return 0;
	}
	typedef void (*STARTSERVER)();
	STARTSERVER p_StartServer;
	p_StartServer = (STARTSERVER)GetProcAddress(hdll,"StartServer");
	p_StartServer();
	return 0;
}
