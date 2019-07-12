#include "pch.h"
#include "IocpServer.h"
#include "Net.h"
#include "IoContext.h"
#include "ClientContext.h"
#include "LockGuard.h"

#include <iostream>
#include <assert.h>

#include <process.h>
#include <mstcpip.h>    //for struct tcp_keepalive


using namespace std;

//工作线程退出标志
#define EXIT_THREAD 0
constexpr int POST_ACCEPT_CNT = 10;

IocpServer::IocpServer(short listenPort, int maxConnectionCount) :
    m_bIsShutdown(false)
    , m_hComPort(NULL)
    , m_hExitEvent(NULL)
    , m_hWriteCompletedEvent(NULL)
    , m_listenPort(listenPort)
    , m_pListenCtx(nullptr)
    , m_nWorkerCnt(0)
    , m_nConnClientCnt(0)
    , m_nMaxConnClientCnt(maxConnectionCount)
{
    //手动reset，初始状态为nonsignaled
    m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (WSA_INVALID_EVENT == m_hExitEvent)
    {
        cout << "CreateEvent failed with error: " << WSAGetLastError() << endl;
    }

    //自动reset，初始状态为signaled
    m_hWriteCompletedEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
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
        return false;
    }
    if (!createIocpWorker())
    {
        return false;
    }
    if (!initAcceptIoContext())
    {
        return false;
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

    for_each(m_acceptIoCtxList.begin(), m_acceptIoCtxList.end(),
        [](AcceptIoContext* mAcceptIoCtx) {

        CancelIo((HANDLE)mAcceptIoCtx->m_acceptSocket);
        closesocket(mAcceptIoCtx->m_acceptSocket);
        mAcceptIoCtx->m_acceptSocket = INVALID_SOCKET;

        while (!HasOverlappedIoCompleted(&mAcceptIoCtx->m_overlapped))
            Sleep(1);

        delete mAcceptIoCtx;
    });
    m_acceptIoCtxList.clear();

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
    if (m_pListenCtx)
    {
        closesocket(m_pListenCtx->m_socket);
        m_pListenCtx->m_socket = INVALID_SOCKET;
        delete m_pListenCtx;
        m_pListenCtx = nullptr;
    }
    removeAllClientContext();

    return true;
}

bool IocpServer::shutdown()
{
    m_bIsShutdown = true;

    int ret = CancelIoEx((HANDLE)m_pListenCtx->m_socket, NULL);
    if (0 == ret)
    {
        cout << "CancelIoEx failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    closesocket(m_pListenCtx->m_socket);
    m_pListenCtx->m_socket = INVALID_SOCKET;

    for_each(m_acceptIoCtxList.begin(), m_acceptIoCtxList.end(),
        [](AcceptIoContext* pAcceptIoCtx) {
        int ret = CancelIoEx((HANDLE)pAcceptIoCtx->m_acceptSocket, &pAcceptIoCtx->m_overlapped);
        if (0 == ret)
        {
            cout << "CancelIoEx failed with error: " << WSAGetLastError() << endl;
            return;
        }
        closesocket(pAcceptIoCtx->m_acceptSocket);
        pAcceptIoCtx->m_acceptSocket = INVALID_SOCKET;

        while (!HasOverlappedIoCompleted(&pAcceptIoCtx->m_overlapped))
            Sleep(1);

        delete pAcceptIoCtx;
    });
    m_acceptIoCtxList.clear();


    return false;
}

bool IocpServer::send(ClientContext* pConnClient, PBYTE pData, UINT len)
{
    Buffer sendBuf;
    sendBuf.write(pData, len);

    LockGuard lk(&pConnClient->m_csLock);

    if (0 == pConnClient->m_outBuf.getBufferLen())
    {
        pConnClient->m_outBuf.copy(sendBuf);
        pConnClient->m_sendIoCtx->m_wsaBuf.buf = (PCHAR)pConnClient->m_outBuf.getBuffer();
        pConnClient->m_sendIoCtx->m_wsaBuf.len = pConnClient->m_outBuf.getBufferLen();

        PostResult result = postSend(pConnClient);
        if (PostResult::PostResultFailed == result)
        {
            CloseClient(pConnClient);
            removeClientContext(pConnClient);
            return false;
        }
    }
    else
    {
        pConnClient->m_outBufQueue.push(sendBuf);
    }
    //int ret = WaitForSingleObject(m_hWriteCompletedEvent, INFINITE);
    //PostQueuedCompletionStatus(m_hComPort, 0, (ULONG_PTR)pConnClient, &pConnClient->m_sendIoCtx->m_overlapped);
    return true;
}

unsigned WINAPI IocpServer::IocpWorkerThread(LPVOID arg)
{
    IocpServer* pThis = static_cast<IocpServer*>(arg);

    int             ret;
    DWORD           dwBytesTransferred;
    DWORD           dwMilliSeconds = INFINITE;
    ULONG_PTR       lpCompletionKey;
    LPOVERLAPPED    lpOverlapped = nullptr;

    while (WAIT_OBJECT_0 != WaitForSingleObject(pThis->m_hExitEvent, 0))
    {
        ret = GetQueuedCompletionStatus(pThis->m_hComPort, &dwBytesTransferred,
            &lpCompletionKey, &lpOverlapped, dwMilliSeconds);
        if (EXIT_THREAD == lpCompletionKey)
        {
            //退出工作线程
            cout << "EXIT_THREAD" << endl;
            break;
        }
        //超时的时候触发，INFINITE不会触发
        if (0 == ret)
        {
            cout << "GetQueuedCompletionStatus failed with error: " << WSAGetLastError() << endl;
            pThis->handleClose(lpCompletionKey);
            continue;
        }

        IoContext* pIoCtx = (IoContext*)lpOverlapped;

        //对端关闭
        if (0 == dwBytesTransferred && 5 != pIoCtx->m_postType)
        {
            pThis->handleClose(lpCompletionKey);
            continue;
        }

        //shutdown状态则停止接受连接
        if (pThis->m_bIsShutdown && lpCompletionKey == (ULONG_PTR)pThis)
        {
            continue;
        }

        switch (pIoCtx->m_postType)
        {
        case PostType::ACCEPT_EVENT:
            pThis->handleAccept(lpOverlapped, dwBytesTransferred);
            break;
        case PostType::RECV_EVENT:
            pThis->handleRecv(lpCompletionKey, lpOverlapped, dwBytesTransferred);
            break;
        case PostType::SEND_EVENT:
            pThis->handleSend(lpCompletionKey, lpOverlapped, dwBytesTransferred);
            break;
        case PostType::CLOSE_EVENT:
            pThis->handleClose(lpCompletionKey);
            break;
        default:
            break;
        }
    }

    cout << "exit" << endl;
    return 0;
}

HANDLE IocpServer::associateWithCompletionPort(SOCKET s, ULONG_PTR completionKey)
{
    HANDLE hRet;
    ClientContext* pClientCtx = (ClientContext*)completionKey;
    if (NULL == completionKey)
    {
        hRet = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    }
    else 
    {
        hRet = CreateIoCompletionPort((HANDLE)s, m_hComPort, completionKey, 0);
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
    int ret = WSAIoctl(m_pListenCtx->m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &GuidAcceptEx, sizeof(GuidAcceptEx),
        &lpfnAcceptEx, sizeof(lpfnAcceptEx),
        &dwBytes, NULL, NULL);
    if (SOCKET_ERROR == ret)
    {
        cout << "WSAIoctl failed with error: " << WSAGetLastError();
        closesocket(m_pListenCtx->m_socket);
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
    int ret = WSAIoctl(m_pListenCtx->m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &GuidAddrs, sizeof(GuidAddrs),
        &lpfnGetAcceptExAddr, sizeof(lpfnGetAcceptExAddr),
        &dwBytes, NULL, NULL);
    if (SOCKET_ERROR == ret)
    {
        cout << "WSAIoctl failed with error: " << WSAGetLastError();
        closesocket(m_pListenCtx->m_socket);
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
    m_pListenCtx = new ListenContext(listenPort);

    //创建完成端口
    m_hComPort = associateWithCompletionPort(INVALID_SOCKET, NULL);
    if (NULL == m_hComPort)
        return false;

    //关联监听socket和完成端口，这里将this指针作为completionKey给完成端口
    if (NULL == associateWithCompletionPort(m_pListenCtx->m_socket, (ULONG_PTR)this))
    {
        return false;
    }

    if (SOCKET_ERROR == Net::bind(m_pListenCtx->m_socket, &m_pListenCtx->m_addr))
    {
        cout << "bind failed" << endl;
        return false;
    }

    if (SOCKET_ERROR == Net::listen(m_pListenCtx->m_socket))
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

bool IocpServer::initAcceptIoContext()
{
    //投递accept请求
    for (int i = 0; i < POST_ACCEPT_CNT; ++i)
    {
        AcceptIoContext* pAcceptIoCtx = new AcceptIoContext(PostType::ACCEPT_EVENT);
        m_acceptIoCtxList.emplace_back(pAcceptIoCtx);
        if (!postAccept(pAcceptIoCtx))
        {
            return false;
        }
    }
    return true;
}

bool IocpServer::postAccept(AcceptIoContext* pAcceptIoCtx)
{
    pAcceptIoCtx->resetBuffer();

    DWORD dwRecvByte;
    PCHAR pBuf = pAcceptIoCtx->m_wsaBuf.buf;
    ULONG nLen = pAcceptIoCtx->m_wsaBuf.len - ACCEPT_ADDRS_SIZE;

    LPOVERLAPPED pOverlapped = &pAcceptIoCtx->m_overlapped;
    //LPOVERLAPPED pOverlapped = (LPOVERLAPPED)pAcceptIoCtx;
    LPFN_ACCEPTEX lpfnAcceptEx = (LPFN_ACCEPTEX)m_lpfnAcceptEx;

    //创建用于接受连接的socket
    pAcceptIoCtx->m_acceptSocket = Net::WSASocket_();
    if (SOCKET_ERROR == pAcceptIoCtx->m_acceptSocket)
    {
        cout << "create socket failed" << endl;
        return false;
    }

    if (FALSE == lpfnAcceptEx(m_pListenCtx->m_socket, pAcceptIoCtx->m_acceptSocket,
        pBuf, nLen, sizeof(SOCKADDR) + 16, sizeof(SOCKADDR) + 16,
        &dwRecvByte, pOverlapped))
    {
        if (WSA_IO_PENDING != WSAGetLastError())
        {
            cout << "acceptEx failed" << endl;
            return false;
        }
    }
    else
    {
        // Accept completed synchronously. We need to marshal the data received over to the 
        // worker thread ourselves...
    }
    return true;
}

PostResult IocpServer::postRecv(ClientContext* pConnClient)
{
    PostResult result = PostResult::PostResultSuccesful;
    RecvIoContext* pRecvIoCtx = pConnClient->m_recvIoCtx;

    pRecvIoCtx->resetBuffer();

    LockGuard lk(&pConnClient->m_csLock);
    if (INVALID_SOCKET != pConnClient->m_socket)
    {
        DWORD dwBytes;
        //设置这个标志，则没收完的数据下一次接收
        DWORD dwFlag = MSG_PARTIAL;
        int ret = WSARecv(pConnClient->m_socket, &pRecvIoCtx->m_wsaBuf, 1,
            &dwBytes, &dwFlag, &pRecvIoCtx->m_overlapped, NULL);
        if (SOCKET_ERROR == ret && WSA_IO_PENDING != WSAGetLastError())
        {
            cout << "WSARecv failed with error: " << WSAGetLastError() << endl;
            result = PostResult::PostResultFailed;
        }
    }
    return result;
}

PostResult IocpServer::postSend(ClientContext* pConnClient)
{
    PostResult result = PostResult::PostResultSuccesful;
    IoContext* pSendIoCtx = pConnClient->m_sendIoCtx;

    LockGuard lk(&pConnClient->m_csLock);
    if (INVALID_SOCKET != pConnClient->m_socket)
    {
        DWORD dwBytesSent;
        DWORD dwFlag = MSG_PARTIAL;
        int ret = WSASend(pConnClient->m_socket, &pSendIoCtx->m_wsaBuf, 1, &dwBytesSent,
            dwFlag, &pSendIoCtx->m_overlapped, NULL);
        if (SOCKET_ERROR == ret && WSA_IO_PENDING != WSAGetLastError())
        {
            cout << "WSASend failed with error: " << WSAGetLastError() << endl;
            result = PostResult::PostResultFailed;
        }
    }
    return result;
}

#if 0
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
#endif

bool IocpServer::handleAccept(LPOVERLAPPED lpOverlapped, DWORD dwBytesTransferred)
{
    AcceptIoContext* pAcceptIoCtx = (AcceptIoContext*)lpOverlapped;
    Net::updateAcceptContext(m_pListenCtx->m_socket, pAcceptIoCtx->m_acceptSocket);

    //达到最大连接数则关闭新的socket
    if (m_nConnClientCnt == m_nMaxConnClientCnt)
    {
        closesocket(pAcceptIoCtx->m_acceptSocket);
        pAcceptIoCtx->m_acceptSocket = INVALID_SOCKET;
        postAccept(pAcceptIoCtx);
        return true;
    }
    InterlockedIncrement(&m_nConnClientCnt);

    LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExAddr = (LPFN_GETACCEPTEXSOCKADDRS)m_lpfnGetAcceptExAddr;
    char* pBuf = pAcceptIoCtx->m_wsaBuf.buf;
    ULONG nLen = pAcceptIoCtx->m_wsaBuf.len - ACCEPT_ADDRS_SIZE;
    LPSOCKADDR_IN localAddr = nullptr;
    LPSOCKADDR_IN peerAddr = nullptr;
    int localAddrLen = sizeof(SOCKADDR_IN);
    int peerAddrLen = sizeof(SOCKADDR_IN);

    lpfnGetAcceptExAddr(pBuf, nLen, localAddrLen + 16,
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
    ClientContext* pConnClient = new ClientContext(pAcceptIoCtx->m_acceptSocket);
    memcpy_s(&pConnClient->m_addr, peerAddrLen, peerAddr, peerAddrLen);

    if (NULL == associateWithCompletionPort(pConnClient->m_socket, (ULONG_PTR)pConnClient))
    {
        return false;
    }

    notifyNewConnection(pConnClient);
    notifyPackageReceived();

    ////开启心跳机制
    //setKeepAlive(pConnClient->m_socket, pIoCtx);

    pConnClient->appendToBuffer((PBYTE)pBuf, dwBytesTransferred);

    //投递一个新的accpet请求
    postAccept(pAcceptIoCtx);

    //将客户端加入连接列表
    addClientContext(pConnClient);

    //解数据包
    pConnClient->decodePacket();

    echo(pConnClient);

    //投递recv请求
    PostResult result = postRecv(pConnClient);
    if (PostResult::PostResultFailed == result)
    {
        CloseClient(pConnClient);
        removeClientContext(pConnClient);
    }

    return true;
}

bool IocpServer::handleRecv(ULONG_PTR lpCompletionKey, LPOVERLAPPED lpOverlapped, DWORD dwBytesTransferred)
{
    ClientContext* pConnClient = (ClientContext*)lpCompletionKey;
    RecvIoContext* pRecvIoCtx = (RecvIoContext*)lpOverlapped;

    //加锁
    pConnClient->appendToBuffer(pRecvIoCtx->m_recvBuf, dwBytesTransferred);

    notifyPackageReceived();

    //解数据包
    pConnClient->decodePacket();

    ////投递send请求
    //IoContext* pSendIoCtx = pConnClient->getIoContext(PostType::SEND_EVENT);
    //postSend(pSendIoCtx);

    echo(pConnClient);

    //投递recv请求
    PostResult result = postRecv(pConnClient);
    if (PostResult::PostResultFailed == result)
    {
        CloseClient(pConnClient);
        removeClientContext(pConnClient);
    }

    return true;
}

bool IocpServer::handleSend(ULONG_PTR lpCompletionKey, LPOVERLAPPED lpOverlapped, DWORD dwBytesTransferred)
{
    ClientContext* pConnClient = (ClientContext*)lpCompletionKey;
    IoContext* pIoCtx = (IoContext*)lpOverlapped;

    DWORD n = -1;

    LockGuard lk(&pConnClient->m_csLock);

    pConnClient->m_outBuf.remove(dwBytesTransferred);
    if (0 == pConnClient->m_outBuf.getBufferLen())
    {
        notifyWriteCompleted();
        pConnClient->m_outBuf.clear();

        if (!pConnClient->m_outBufQueue.empty())
        {
            pConnClient->m_outBuf.copy(pConnClient->m_outBufQueue.front());
            pConnClient->m_outBufQueue.pop();
        }
    }
    if (0 != pConnClient->m_outBuf.getBufferLen())
    {
        pIoCtx->m_wsaBuf.buf = (PCHAR)pConnClient->m_outBuf.getBuffer();
        pIoCtx->m_wsaBuf.len = pConnClient->m_outBuf.getBufferLen();

        PostResult result = postSend(pConnClient);
        if (PostResult::PostResultFailed == result)
        {
            CloseClient(pConnClient);
            removeClientContext(pConnClient);
        }
    }
    return false;
}

bool IocpServer::handleClose(ULONG_PTR lpCompletionKey)
{
    ClientContext* pConnClient = (ClientContext*)lpCompletionKey;
    CloseClient(pConnClient);
    removeClientContext(pConnClient);
    return true;
}

void IocpServer::addClientContext(ClientContext* pConnClient)
{
    LockGuard lk(&m_csConnList);
    m_connList.emplace_back(pConnClient);
}

void IocpServer::removeClientContext(ClientContext* pConnClient)
{
    LockGuard lk(&m_csConnList);
    m_connList.remove(pConnClient);
    delete pConnClient;
}

void IocpServer::removeAllClientContext()
{
    LockGuard lk(&m_csConnList);
    for_each(m_connList.begin(), m_connList.end(),
        [](ClientContext* it) { delete it; });
    m_connList.erase(m_connList.begin(), m_connList.end());
}

void IocpServer::CloseClient(ClientContext* pConnClient)
{
    SOCKET s;

    {
        LockGuard lk(&pConnClient->m_csLock);
        s = pConnClient->m_socket;
        pConnClient->m_socket = INVALID_SOCKET;
    }

    if (INVALID_SOCKET != s)
    {
        notifyDisconnected();

        if (!Net::setLinger(s))
            return;

        int ret = CancelIoEx((HANDLE)s, NULL);
        if (0 == ret)
        {
            cout << "CancelIoEx failed with error: " << WSAGetLastError() << endl;
            return;
        }

        closesocket(s);

        InterlockedDecrement(&m_nConnClientCnt);
    }
}

void IocpServer::echo(ClientContext* pConnClient)
{
    IoContext* pSendIoCtx = pConnClient->m_sendIoCtx;
    RecvIoContext* pRecvIoCtx = pConnClient->m_recvIoCtx;
    assert(nullptr != pRecvIoCtx);

    pConnClient->m_outBuf.copy(pConnClient->m_inBuf);
    pSendIoCtx->m_wsaBuf.buf = (PCHAR)pConnClient->m_outBuf.getBuffer(0);
    pSendIoCtx->m_wsaBuf.len = pConnClient->m_outBuf.getBufferLen();

    pConnClient->m_inBuf.remove(pConnClient->m_inBuf.getBufferLen());

    postSend(pConnClient);
}

void IocpServer::notifyNewConnection(ClientContext * pConnClient)
{
}

void IocpServer::notifyDisconnected()
{
}

void IocpServer::notifyPackageReceived()
{
}

void IocpServer::notifyWritePackage()
{
}

void IocpServer::notifyWriteCompleted()
{
}

