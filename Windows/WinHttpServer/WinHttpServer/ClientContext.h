#ifndef __CLIENT_CONTEXT_H__
#define __CLIENT_CONTEXT_H__

#include "Global.h"
#include "Buffer.h"
#include "Codec.h"
#include <string>
#include <map>
#include <algorithm>
#include <queue>

struct ListenContext
{
    ListenContext(short port, const std::string& ip = "0.0.0.0");

    SOCKET          m_socket;   //监听socket
    SOCKADDR_IN     m_addr;     //监听地址
};

struct IoContext;
struct RecvIoContext;

struct ClientContext
{
    ClientContext(const SOCKET& socket = INVALID_SOCKET);
    ~ClientContext();

    void appendToBuffer(PBYTE pInBuf, size_t len);
    void appendToBuffer(const std::string& inBuf);

    bool decodePacket();

    SOCKET                              m_socket;       //客户端socket
    SOCKADDR_IN                         m_addr;         //客户端地址
    RecvIoContext*                      m_recvIoCtx;
    IoContext*                          m_sendIoCtx;
    HttpCodec*                          m_pCodec;
    Buffer                              m_inBuf;
    //Buffer                              m_outBuf;
    std::queue<Buffer>                  m_outBufList;
    CRITICAL_SECTION                    m_csLock;       //加锁，保护socket，链表
};

#endif // !__CLIENT_CONTEXT_H__

