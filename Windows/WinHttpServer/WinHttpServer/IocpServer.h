#ifndef __IOCP_SERVER_H__
#define __IOCP_SERVER_H__

#include <list>
#include <vector>
#include "Global.h"
#include "Addr.h"

struct ListenContext;
struct IoContext;
struct AcceptIoContext;
struct ClientContext;

class IocpServer
{
    friend class HbTimer;

public:
    IocpServer(short listenPort, int maxConnectionCount = 10000);
    IocpServer(const IocpServer&) = delete;
    IocpServer& operator=(const IocpServer&) = delete;
    virtual ~IocpServer();

    bool start();
    bool stop();
    bool shutdown();
    //bool init();
    //bool unInit();

    bool send(ClientContext* pConnClient, PBYTE pData, UINT len);

protected:
    //必须要static _beginthreadex才能访问
    static unsigned WINAPI IocpWorkerThread(LPVOID arg);

    HANDLE associateWithCompletionPort(SOCKET s, ULONG_PTR completionKey);
    bool getAcceptExPtr();
    bool getAcceptExSockaddrs();
    bool setKeepAlive(ClientContext* pConnClient, LPOVERLAPPED lpOverlapped, int time = 1, int interval = 1);

    bool createListenClient(short listenPort);
    bool createIocpWorker();
    bool exitIocpWorker();
    bool initAcceptIoContext();

    bool postAccept(AcceptIoContext* pIoCtx);
    PostResult postRecv(ClientContext* pConnClient);
    PostResult postSend(ClientContext* pConnClient);

    bool handleAccept(LPOVERLAPPED lpOverlapped, DWORD dwBytesTransferred);
    bool handleRecv(ULONG_PTR lpCompletionKey, LPOVERLAPPED lpOverlapped, DWORD dwBytesTransferred);
    bool handleSend(ULONG_PTR lpCompletionKey, LPOVERLAPPED lpOverlapped, DWORD dwBytesTransferred);
    bool handleClose(ULONG_PTR lpCompletionKey);

    // Used to avoid access violation.
    void enterIoLoop(ClientContext* pClientCtx);
    int exitIoLoop(ClientContext* pClientCtx);

    void CloseClient(ClientContext* pConnClient);

    //管理已连接客户端链表，线程安全
    void addClient(ClientContext* pConnClient);
    void removeClient(ClientContext* pConnClient);
    void removeAllClients();

    ClientContext* allocateClientContext(SOCKET s);
    void releaseClientContext(ClientContext* pConnClient);

    void echo(ClientContext* pConnClient);

    //回调函数
    virtual void notifyNewConnection(ClientContext* pConnClient);
    //virtual void notifyDisconnected(ClientContext* pConnClient);
    virtual void notifyDisconnected(SOCKET s, Addr addr);
    virtual void notifyPackageReceived(ClientContext* pConnClient);
    virtual void notifyWritePackage();
    virtual void notifyWriteCompleted();

private:
    bool                            m_bIsShutdown;          //关闭时，退出工作线程

    short                           m_listenPort;
    HANDLE                          m_hComPort;             //完成端口
    HANDLE                          m_hExitEvent;           //退出线程事件
    HANDLE                          m_hWriteCompletedEvent; //postSend对应的写操作已完成，可以进行下一个投递

    void*                           m_lpfnAcceptEx;         //acceptEx函数指针
    void*                           m_lpfnGetAcceptExAddr;  //GetAcceptExSockaddrs函数指针

    int                             m_nWorkerCnt;           //io工作线程数量
    DWORD                           m_nConnClientCnt;       //已连接客户端数量
    DWORD                           m_nMaxConnClientCnt;    //最大客户端数量

    ListenContext*                  m_pListenCtx;           //监听上下文

    std::list<ClientContext*>       m_connectedClientList;  //已连接客户端链表
    std::list<ClientContext*>       m_freeClientList;       //空闲的ClientContext链表
    CRITICAL_SECTION                m_csClientList;         //保护客户端链表std::list<ClientContext*>


    std::vector<HANDLE>             m_hWorkerThreads;       //工作线程句柄列表
    std::vector<AcceptIoContext*>   m_acceptIoCtxList;      //接收连接的IO上下文列表

};

#endif // !__IOCP_SERVER_H__

