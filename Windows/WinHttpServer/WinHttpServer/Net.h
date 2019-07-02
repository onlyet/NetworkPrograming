#ifndef __NET_H__
#define __NET_H__

#include "Buffer.h"

#include <string>
#include <map>
#include <algorithm>

//设置成4K的倍数
constexpr int IO_BUF_SIZE = 8192;

enum PostType
{
    ACCEPT_EVENT,
    RECV_EVENT,
    SEND_EVENT,
    PARSE_EVNET,    //数据包解析
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

    std::string				m_inBuf;        //数据包输入缓冲区，非线程安全，所以加锁
    std::string				m_outBuf;
    CRITICAL_SECTION        m_csInBuf;
    //CONDITION_VARIABLE      m_cvInBuf;
    //Buffer                  m_inBuf;

    std::map<PostType, IoContext*> m_ioCtxs;

    ClientContext(const SOCKET& socket = INVALID_SOCKET) :
        m_socket(socket)
    {
        //InitializeCriticalSection(&m_csInBuf);
    }

    ~ClientContext()
    {
        if (INVALID_SOCKET != m_socket)
        {
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
        }
        std::for_each(m_ioCtxs.begin(), m_ioCtxs.end(), [](const IoContext*& ioCtx) { delete ioCtx; });
        m_ioCtxs.erase(m_ioCtxs.begin(), m_ioCtxs.end());
    }

    IoContext* getIoContext(PostType type)
    {
        std::map<PostType, IoContext*>::iterator it = m_ioCtxs.find(type);
        if (it != m_ioCtxs.end())
        {
            return m_ioCtxs[type];
        }
        IoContext* ioCtx = new IoContext(type);
        m_ioCtxs.insert(std::make_pair(type, ioCtx));
        return ioCtx;
    }

    void removeIoContext(IoContext* pIoCtx)
    {
        m_ioCtxs.erase(pIoCtx->m_postType);
    }

    void appendToBuffer(const char* inBuf, size_t len)
    {
        EnterCriticalSection(&m_csInBuf);
        m_inBuf.append(inBuf, len);
        LeaveCriticalSection(&m_csInBuf);
    }

    void appendToBuffer(const std::string& inBuf)
    {
        EnterCriticalSection(&m_csInBuf);
        m_inBuf.append(inBuf);
        LeaveCriticalSection(&m_csInBuf);
    }
};

struct ClientContext;

struct Net
{
    static bool init();
    static bool unInit();

    static bool associateWithCompletionPort(HANDLE completionPort, ClientContext* pConnClient);

    static bool setKeepAlive(SOCKET socket, bool on);
};

#endif // !__NET_H__


