#ifndef __IOCP_SERVER_H__
#define __IOCP_SERVER_H__

#include <list>
#include <vector>
#include "Global.h"

struct ListenContext;
struct IoContext;
struct ClientContext;

class IocpServer
{
public:

    IocpServer(short listenPort, int maxConnectionCount = 10000);
    IocpServer(const IocpServer&) = delete;
    IocpServer& operator=(const IocpServer&) = delete;
    ~IocpServer();

    bool start();
    bool stop();
    bool shutdown();
    //bool init();
    //bool unInit();

protected:
    //必须要static _beginthreadex才能访问
    static unsigned WINAPI IocpWorkerThread(LPVOID arg);

    HANDLE associateWithCompletionPort(SOCKET s, ULONG_PTR completionKey);
    bool getAcceptExPtr();
    bool getAcceptExSockaddrs();
    bool setKeepAlive(SOCKET s, IoContext* pIoCtx, int time = 30, int interval = 10);

    bool createListenClient(short listenPort);
    bool createIocpWorker();
    bool exitIocpWorker();
    bool initAcceptIoContext();

    bool postAccept(IoContext* pIoCtx);
    PostResult postRecv(ClientContext* pConnClient);
    PostResult postSend(ClientContext* pConnClient);
    bool postParse(ClientContext* pConnClient, IoContext* pIoCtx);

    bool handleAccept(ClientContext* pListenClient, IoContext* pIoCtx);
    bool handleRecv(ClientContext* pConnClient, IoContext* pIoCtx);
    bool handleSend(ClientContext* pConnClient, IoContext* pIoCtx);
    bool handleParse(ClientContext* pConnClient, IoContext* pIoCtx);
    bool handleClose(ClientContext* pConnClient, IoContext* pIoCtx);

    //线程安全
    void addClientContext(ClientContext* pConnClient);
    void removeClientContext(ClientContext* pConnClient);
    void removeAllClientContext();
    void CloseClient(ClientContext* pConnClient);

    bool decodePacket();

    void echo(ClientContext* pConnClient);


private:
    bool                        m_bIsShutdown;          //关闭时，退出工作线程

    short                       m_listenPort;
    HANDLE                      m_hComPort;              //完成端口
    HANDLE                      m_hExitEvent;           //退出线程事件

    void*                       m_lpfnAcceptEx;		    //acceptEx函数指针
    void*                       m_lpfnGetAcceptExAddr;  //GetAcceptExSockaddrs函数指针

    int                         m_nWorkerCnt;           //io工作线程数量
    DWORD                       m_nConnClientCnt;       //已连接客户端数量
    DWORD                       m_nMaxConnClientCnt;    //最大客户端数量

    ListenContext*              m_pListenCtx;            //监听上下文
    std::list<ClientContext*>   m_connList;             //已连接客户端列表
    CRITICAL_SECTION            m_csConnList;           //保护连接列表

    std::vector<HANDLE>         m_hWorkerThreads;       //工作线程句柄列表
    std::vector<IoContext*>     m_acceptIoCtxList;      //接收连接的IO上下文列表

};

#endif // !__IOCP_SERVER_H__

