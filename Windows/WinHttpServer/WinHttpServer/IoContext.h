#ifndef __IO_CONTEXT_H__
#define __IO_CONTEXT_H__

#include "Global.h"
#include "Buffer.h"

//为了保证以下结构体第一个变量成员是OVERLAPPED，不能使用虚函数（具有虚函数的类第一个变量是虚表指针）

struct IoContext
{
    IoContext(PostType type);
    ~IoContext();

    void resetBuffer();

    OVERLAPPED      m_overlapped;			//每一个重叠io操作都要有一个OVERLAPPED结构
    PostType        m_postType;
    WSABUF          m_wsaBuf;				//重叠io需要的buf
};

struct AcceptIoContext : public IoContext
{
    AcceptIoContext(SOCKET acceptSocket = INVALID_SOCKET);
    ~AcceptIoContext();

    void resetBuffer();

    SOCKET          m_acceptSocket;             //接受连接的socket
    BYTE            m_accpetBuf[IO_BUF_SIZE];   //用于acceptEx接收数据
};

struct RecvIoContext : public IoContext
{
    RecvIoContext();
    ~RecvIoContext();

    void resetBuffer();

    BYTE            m_recvBuf[IO_BUF_SIZE];
};

#endif // !__IO_CONTEXT_H__
