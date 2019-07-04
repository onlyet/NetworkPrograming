#include "pch.h"
#include "IocpServer.h"
#include "Net.h"
#include "IoContext.h"
#include "ClientContext.h"
//#include "DataPacket.h"

#include <iostream>

#include <process.h>
#include <MSWSock.h>
#include <mstcpip.h>    //for struct tcp_keepalive
#include <WS2tcpip.h>     //for inet_ntop

using namespace std;

#define EXIT_THREAD 0
constexpr int POST_ACCEPT_CNT = 10;

IocpServer::IocpServer(short listenPort) :
    m_hComPort(NULL)
    , m_hExitEvent(NULL)
    , m_listenPort(listenPort)
    , m_pListenClient(nullptr)
    , m_nWorkerCnt(0)
{
    m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (WSA_INVALID_EVENT == m_hExitEvent)
    {
        cout << "CreateEvent failed with error: " << WSAGetLastError() << endl;
    }
    InitializeCriticalSection(&m_csConnList);
}

IocpServer::~IocpServer()
{
    stop();

    DeleteCriticalSection(&m_csConnList);
    Net::unInit();
}

bool IocpServer::start()
{
    if (!Net::init())
    {
        cout << "network initial failed" << endl;
        return false;
    }

    if (!createListenClient(m_listenPort))
    {
        stop();
        return false;
    }

    if (!createIocpWorker())
    {
        stop();
        return false;
    }

    //投递accept请求
    for (int i = 0; i < POST_ACCEPT_CNT; ++i)
    {
        IoContext* pListenIoCtx = m_pListenClient->getIoContext(PostType::ACCEPT_EVENT);
        if (!postAccept(pListenIoCtx))
        {
            stop();
            return false;
        }
    }

    return true;
}

bool IocpServer::stop()
{
    //同步等待所有工作线程退出
    exitIocpWorker();
    //关闭工作线程句柄
    for_each(m_hWorkerThreads.begin(), m_hWorkerThreads.end(),
        [](const HANDLE& h) { CloseHandle(h); });

    if (m_hExitEvent)
    {
        CloseHandle(m_hExitEvent);
        m_hExitEvent = NULL;
    }
    if (m_hComPort)
    {
        CloseHandle(m_hComPort);
        m_hComPort = NULL;
    }
    if (m_pListenClient)
    {
        closesocket(m_pListenClient->m_socket);
        m_pListenClient->m_socket = INVALID_SOCKET;
        delete m_pListenClient;
        m_pListenClient = nullptr;
    }
    removeClients();

    return true;
}

unsigned WINAPI IocpServer::IocpWorkerThread(LPVOID arg)
{
    IocpServer* pThis = static_cast<IocpServer*>(arg);

    DWORD           dwBytes;
    ClientContext*  pClientCtx = nullptr;
    IoContext*      pIoCtx = nullptr;

    while (WAIT_OBJECT_0 != WaitForSingleObject(pThis->m_hExitEvent, 0))
    {
        int ret = GetQueuedCompletionStatus(pThis->m_hComPort, &dwBytes, 
            (PULONG_PTR)&pClientCtx, (LPOVERLAPPED*)&pIoCtx, INFINITE);
        if (EXIT_THREAD == pClientCtx)
        {
            //退出工作线程
            cout << "exit" << endl;
            break;
        }
        if (0 == ret)
        {
            cout << "GetQueuedCompletionStatus failed with error: " << WSAGetLastError() << endl;
            return 0;
        }

        pIoCtx->m_dwBytesTransferred = dwBytes;
        if (0 == pIoCtx->m_dwBytesTransferred)
        {
            pIoCtx->m_postType = PostType::CLOSE_EVENT;
        }

        switch (pIoCtx->m_postType)
        {
        case PostType::ACCEPT_EVENT:
            pThis->handleAccept(pClientCtx, pIoCtx);
            break;
        case PostType::RECV_EVENT:
            pThis->handleRecv(pClientCtx, pIoCtx);
            break;
        case PostType::SEND_EVENT:
            pThis->handleSend(pClientCtx, pIoCtx);
            break;
        case PostType::CLOSE_EVENT:
            pThis->handleClose(pClientCtx, pIoCtx);
            break;
        default:
            pThis->handleClose(pClientCtx, pIoCtx);
            break;
        }
    }

    cout << "exit" << endl;
    return 0;
}

HANDLE IocpServer::associateWithCompletionPort(ClientContext* pClient)
{
    HANDLE hRet;
    if (nullptr == pClient)
    {
        hRet = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    }
    else
    {
        hRet = CreateIoCompletionPort((HANDLE)pClient->m_socket, m_hComPort, (ULONG_PTR)pClient, 0);
    }
    if (NULL == hRet)
    {
        cout << "failed to associate the socket with completion port" << endl;
    }
    return hRet;
}

bool IocpServer::getAcceptExPtr()
{
    DWORD dwBytes;
    GUID GuidAcceptEx = WSAID_ACCEPTEX;
    LPFN_ACCEPTEX lpfnAcceptEx = NULL;
    int ret = WSAIoctl(m_pListenClient->m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &GuidAcceptEx, sizeof(GuidAcceptEx),
        &lpfnAcceptEx, sizeof(lpfnAcceptEx),
        &dwBytes, NULL, NULL);
    if (SOCKET_ERROR == ret)
    {
        cout << "WSAIoctl failed with error: " << WSAGetLastError();
        closesocket(m_pListenClient->m_socket);
        return false;
    }
    m_lpfnAcceptEx = lpfnAcceptEx;
    return true;
}

bool IocpServer::getAcceptExSockaddrs()
{
    DWORD dwBytes;
    GUID GuidAddrs = WSAID_GETACCEPTEXSOCKADDRS;
    LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExAddr = NULL;
    int ret = WSAIoctl(m_pListenClient->m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &GuidAddrs, sizeof(GuidAddrs),
        &lpfnGetAcceptExAddr, sizeof(lpfnGetAcceptExAddr),
        &dwBytes, NULL, NULL);
    if (SOCKET_ERROR == ret)
    {
        cout << "WSAIoctl failed with error: " << WSAGetLastError();
        closesocket(m_pListenClient->m_socket);
        return false;
    }
    m_lpfnGetAcceptExAddr = lpfnGetAcceptExAddr;
    return true;
}

bool IocpServer::setKeepAlive(SOCKET s, IoContext* pIoCtx, int time, int interval)
{
    if (!Net::setKeepAlive(s, TRUE))
        return false;

    tcp_keepalive keepAlive;
    keepAlive.onoff = 1;
    keepAlive.keepalivetime = time * 1000;        //30秒 
    keepAlive.keepaliveinterval = interval * 1000;    //间隔10秒
    DWORD dwBytes;
    //根据msdn这里要传一个OVERRLAP结构
    int ret = WSAIoctl(s, SIO_KEEPALIVE_VALS,
        &keepAlive, sizeof(tcp_keepalive), NULL, 0,
        &dwBytes, &pIoCtx->m_overlapped, NULL);
    if (SOCKET_ERROR == ret && WSA_IO_PENDING != WSAGetLastError())
    {
        cout << "WSAIoctl failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool IocpServer::createListenClient(short listenPort)
{
    //创建完成端口
    m_hComPort = associateWithCompletionPort(nullptr);
    if (NULL == m_hComPort)
        return false;

    //创建具有重叠功能的socket
    SOCKET listenSocket = Net::WSASocket_();
    if (SOCKET_ERROR == listenSocket)
    {
        cout << "create socket failed" << endl;
        return false;
    }
    m_pListenClient = new ClientContext(listenSocket);

    //关联监听socket和完成端口
    if (NULL == associateWithCompletionPort(m_pListenClient))
    {
        return false;
    }

    SecureZeroMemory(&m_pListenClient->m_addr, sizeof(SOCKADDR));
    m_pListenClient->m_addr.sin_family = AF_INET;
    m_pListenClient->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_pListenClient->m_addr.sin_port = htons(listenPort);

    if (SOCKET_ERROR == Net::bind(m_pListenClient->m_socket, &m_pListenClient->m_addr))
    {
        cout << "bind failed" << endl;
        return false;
    }

    if (SOCKET_ERROR == Net::listen(m_pListenClient->m_socket))
    {
        cout << "listen failed" << endl;
        return false;
    }

    //获取acceptEx函数指针
    if (!getAcceptExPtr())
        return false;

    //获取GetAcceptExSockaddrs函数指针
    if (!getAcceptExSockaddrs())
        return false;

    return true;
}

bool IocpServer::createIocpWorker()
{
    //根据CPU核数创建IO线程
    HANDLE hWorker;
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    for (DWORD i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
    {
        hWorker = (HANDLE)_beginthreadex(NULL, 0, IocpWorkerThread, this, 0, NULL);
        if (NULL == hWorker)
        {
            return false;
        }
        m_hWorkerThreads.emplace_back(hWorker);
        ++m_nWorkerCnt;
    }
    cout << "started iocp worker thread count: " << m_nWorkerCnt << endl;
    return true;
}

bool IocpServer::exitIocpWorker()
{
    int ret = 0;
    SetEvent(m_hExitEvent);
    for (int i = 0; i < m_nWorkerCnt; ++i)
    {
        //通知工作线程退出
        ret = PostQueuedCompletionStatus(m_hComPort, 0, EXIT_THREAD, NULL);
        if (FALSE == ret)
        {
            cout << "PostQueuedCompletionStatus failed with error: " << WSAGetLastError() << endl;
        }
    }

    //这里不明白为什么会返回0，不是应该返回m_nWorkerCnt-1吗？
    ret = WaitForMultipleObjects(m_nWorkerCnt, m_hWorkerThreads.data(), TRUE, INFINITE);

    return true;
}

bool IocpServer::postAccept(IoContext* pIoCtx)
{
    pIoCtx->resetBuffer();

    char* pBuf = pIoCtx->m_wsaBuf.buf;
    ULONG nLen = pIoCtx->m_wsaBuf.len;
    OVERLAPPED* pOverlapped = &pIoCtx->m_overlapped;
    LPFN_ACCEPTEX lpfnAcceptEx = (LPFN_ACCEPTEX)m_lpfnAcceptEx;

    pIoCtx->m_socket = Net::WSASocket_();
    if (SOCKET_ERROR == pIoCtx->m_socket)
    {
        cout << "create socket failed" << endl;
        return false;
    }

    DWORD dwRecvByte;
    if (FALSE == lpfnAcceptEx(m_pListenClient->m_socket, pIoCtx->m_socket, pBuf,
        nLen - ACCEPT_ADDRS_SIZE, sizeof(SOCKADDR) + 16,
        sizeof(SOCKADDR) + 16, &dwRecvByte, pOverlapped))
    {
        if (WSA_IO_PENDING != WSAGetLastError())
        {
            cout << "acceptEx failed" << endl;
            return false;
        }
    }

    return true;
}

bool IocpServer::postRecv(IoContext* pIoCtx)
{
    pIoCtx->resetBuffer();

    DWORD dwBytes;
    //设置这个标志，则没收完的数据下一次接收
    DWORD flag = MSG_PARTIAL;
    int ret = WSARecv(pIoCtx->m_socket, &pIoCtx->m_wsaBuf, 1,
        &dwBytes, &flag, &pIoCtx->m_overlapped, NULL);
    if (SOCKET_ERROR == ret && WSA_IO_PENDING != WSAGetLastError())
    {
        cout << "WSARecv failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool IocpServer::postSend(IoContext* pIoCtx)
{
    DWORD dwBytesSent;
    DWORD flag = MSG_PARTIAL;
    int ret = WSASend(pIoCtx->m_socket, &pIoCtx->m_wsaBuf, 1, &dwBytesSent,
        flag, &pIoCtx->m_overlapped, NULL);
    if (SOCKET_ERROR == ret && WSA_IO_PENDING != WSAGetLastError())
    {
        cout << "WSASend failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool IocpServer::postParse(ClientContext* pConnClient, IoContext* pIoCtx)
{
    DWORD dwTransferred = pIoCtx->m_wsaBuf.len;
    int ret = PostQueuedCompletionStatus(m_hComPort, dwTransferred, (ULONG_PTR)pConnClient, &pIoCtx->m_overlapped);
    if (FALSE == ret)
    {
        cout << "PostQueuedCompletionStatus failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool IocpServer::handleAccept(ClientContext* pListenClient, IoContext* pIoCtx)
{
    LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExAddr = (LPFN_GETACCEPTEXSOCKADDRS)m_lpfnGetAcceptExAddr;
    char* pBuf = pIoCtx->m_wsaBuf.buf;
    ULONG nLen = pIoCtx->m_wsaBuf.len;
    LPSOCKADDR_IN localAddr = nullptr;
    LPSOCKADDR_IN peerAddr = nullptr;
    int localAddrLen = sizeof(SOCKADDR_IN);
    int peerAddrLen = sizeof(SOCKADDR_IN);

    lpfnGetAcceptExAddr(pBuf, nLen - ACCEPT_ADDRS_SIZE, localAddrLen + 16,
        peerAddrLen + 16, (LPSOCKADDR*)&localAddr, &localAddrLen,
        (LPSOCKADDR*)&peerAddr, &peerAddrLen);

    char localAddrBuf[1024] = { 0 };
    inet_ntop(AF_INET, &localAddr->sin_addr, localAddrBuf, 1024);
    char peerAddrBuf[1024] = { 0 };
    inet_ntop(AF_INET, &peerAddr->sin_addr, peerAddrBuf, 1024);
    //cout << "local address: " << inet_ntoa(localAddr->sin_addr) << ":" << ntohs(localAddr->sin_port) << "\t"
    //    << "peer address: " << inet_ntoa(peerAddr->sin_addr) << ":" << ntohs(peerAddr->sin_port) << endl;
    cout << "local address: " << localAddrBuf << ":" << ntohs(localAddr->sin_port) << "\t"
        << "peer address: " << peerAddrBuf << ":" << ntohs(peerAddr->sin_port) << endl;

    //创建新的ClientContext来保存新的连接socket，原来的IoContext要用来接收新的连接
    ClientContext* pConnClient = new ClientContext(pIoCtx->m_socket);
    memcpy_s(&pConnClient->m_addr, peerAddrLen, peerAddr, peerAddrLen);

    //关联新连接的socket与完成端口，只有关联了，才能收到该socket的IO完成通知，GetQueuedCompletionStatus才能取回包
    if (NULL == associateWithCompletionPort(pConnClient))
    {
        return false;
    }

    ////开启心跳机制
    //setKeepAlive(pConnClient->m_socket, pIoCtx);

    /**
    * 将第一次接收到的数据拷贝到ClientContext::m_inBuf
    * 此处不加锁，因为现在还没有其它线程访问m_inBuf
    */
    pConnClient->appendToBuffer(pBuf, pIoCtx->m_dwBytesTransferred);

    //投递一个新的accpet请求
    postAccept(pIoCtx);

    //将客户端加入连接列表
    addClient(pConnClient);

    ////解数据包
    //pConnClient->decodePacket();

    echo(pConnClient);

    //投递recv请求
    IoContext* pRecvIoCtx = pConnClient->getIoContext(PostType::RECV_EVENT);
    postRecv(pRecvIoCtx);

    return true;
}

bool IocpServer::handleRecv(ClientContext* pConnClient, IoContext* pIoCtx)
{
    char* pBuf = pIoCtx->m_wsaBuf.buf;
    int nLen = pIoCtx->m_wsaBuf.len;

    //加锁
    pConnClient->appendToBuffer(pBuf, pIoCtx->m_dwBytesTransferred);

    ////解数据包
    //pConnClient->decodePacket();

    ////投递send请求
    //IoContext* pSendIoCtx = pConnClient->getIoContext(PostType::SEND_EVENT);
    //postSend(pSendIoCtx);

    echo(pConnClient);

    //投递新的recv请求
    IoContext* pRecvIoCtx = pConnClient->getIoContext(PostType::RECV_EVENT);
    postRecv(pRecvIoCtx);

    return true;
}

bool IocpServer::handleSend(ClientContext* pConnClient, IoContext* pIoCtx)
{
    return true;
}

bool IocpServer::handleParse(ClientContext* pConnClient, IoContext* pIoCtx)
{
    //解析数据包，加锁
    //DataPacket::parse(pConnClient);

    //投递send请求
    IoContext* pRecvIoCtx = pConnClient->getIoContext(PostType::SEND_EVENT);
    postSend(pRecvIoCtx);

    return true;
}

bool IocpServer::handleClose(ClientContext* pConnClient, IoContext* pIoCtx)
{
    removeClient(pConnClient);
    delete pConnClient;
    return true;
}

void IocpServer::addClient(ClientContext* pConnClient)
{
    EnterCriticalSection(&m_csConnList);
    m_connList.emplace_back(pConnClient);
    LeaveCriticalSection(&m_csConnList);
}

void IocpServer::removeClient(ClientContext* pConnClient)
{
    EnterCriticalSection(&m_csConnList);
    m_connList.remove(pConnClient);
    LeaveCriticalSection(&m_csConnList);
}

void IocpServer::removeClients()
{
    EnterCriticalSection(&m_csConnList);
    for_each(m_connList.begin(), m_connList.end(),
        [](ClientContext* it) { delete it; });
    m_connList.erase(m_connList.begin(), m_connList.end());
    LeaveCriticalSection(&m_csConnList);
}

bool IocpServer::decodePacket()
{

    return false;
}

void IocpServer::echo(ClientContext* pConnClient)
{
    pConnClient->m_outBuf = pConnClient->m_inBuf;
    pConnClient->m_inBuf.consume(pConnClient->m_inBuf.size());

    IoContext* pSendIoCtx = pConnClient->getIoContext(PostType::SEND_EVENT);
    pSendIoCtx->m_wsaBuf.buf = pConnClient->m_outBuf.begin();
    pSendIoCtx->m_wsaBuf.len = pConnClient->m_outBuf.size();
    postSend(pSendIoCtx);
}

