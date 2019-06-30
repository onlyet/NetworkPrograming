#ifndef __NET_H__
#define __NET_H__

#include <string>
#include <list>

constexpr int IO_BUF_SIZE = 8192;

enum PostType
{
    ACCEPT_EVENT,
    RECV_EVENT,
    SEND_EVENT
};

struct IoContext
{
    OVERLAPPED      m_overlapped;			//每一个重叠io操作都要有一个OVERLAPPED结构
    WSABUF          m_wsaBuf;				//重叠io需要的buf
    char            m_ioBuf[IO_BUF_SIZE];
    PostType        m_postType;
    SOCKET			m_socket;				//当前进行IO操作的socket，postAccept后再补充一个socket，给后面连接的客户端

    IoContext(PostType type) :
        m_postType(type)
    {
        SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
        SecureZeroMemory(m_ioBuf, IO_BUF_SIZE);
        m_wsaBuf.buf = m_ioBuf;
        m_wsaBuf.len = IO_BUF_SIZE;
    }
    //清空buffer
    void resetBuffer()
    {
        SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
        SecureZeroMemory(m_ioBuf, IO_BUF_SIZE);
    }
};

//一个socket有多个重叠操作
struct ClientContext
{
    SOCKET					m_socket;
    SOCKADDR_IN				m_addr;

    std::string				m_inBuf;
    std::string				m_outBuf;

    std::list<IoContext*>	m_ioCtxs;

    ClientContext(const SOCKET& socket = INVALID_SOCKET) :
        m_socket(socket)
    {
    }

    IoContext* createIoContext(PostType type)
    {
        IoContext* ioCtx = new IoContext(type);
        m_ioCtxs.emplace_back(ioCtx);
        return ioCtx;
    }

    void removeIoContext(IoContext* pIoCtx)
    {
        m_ioCtxs.remove(pIoCtx);
    }

};

struct Net
{
    static bool init();
    static bool unInit();
};

#endif // !__NET_H__


