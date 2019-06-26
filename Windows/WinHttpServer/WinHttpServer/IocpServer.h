#ifndef __IOCP_SERVER_H__
#define __IOCP_SERVER_H__



constexpr auto IO_BUF_SIZE = 8192;

struct ClientContext
{
    SOCKET          m_socket;
    SOCKADDR_IN     m_addr;
};

struct IoContext
{
    OVERLAPPED      m_overlapped;
    WSABUF          m_wsaBuf;
    char            m_ioBuf[IO_BUF_SIZE];
    int             m_opType;
};


class IocpServer
{
    IocpServer();
    IocpServer(const IocpServer&) = delete;
    IocpServer& operator=(const IocpServer&) = delete;
    ~IocpServer();

    bool init();
    bool start();
    bool stop();
};











#endif // !__IOCP_SERVER_H__

