#include "pch.h"
#include "ClientContext.h"
#include "IoContext.h"
#include "Net.h"
#include <assert.h>
#include "Codec.h"
#include "Buffer.h"

ListenContext::ListenContext(short port, const std::string& ip)
{
    SecureZeroMemory(&m_addr, sizeof(SOCKADDR_IN));
    m_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr);
    //m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    m_addr.sin_port = htons(port);

    m_socket = Net::WSASocket_();
    assert(SOCKET_ERROR != m_socket);
}

ClientContext::ClientContext(const SOCKET& socket) :
    m_socket(socket)
    , m_recvIoCtx(new RecvIoContext())
    , m_sendIoCtx(new IoContext(PostType::SEND_EVENT))
    , m_nPendingIoCnt(0)
{
    SecureZeroMemory(&m_addr, sizeof(SOCKADDR_IN));
    InitializeCriticalSection(&m_csLock);
}

ClientContext::~ClientContext()
{
    delete m_recvIoCtx;
    delete m_sendIoCtx;
    m_recvIoCtx = nullptr;
    m_sendIoCtx = nullptr;
    LeaveCriticalSection(&m_csLock);
}

void ClientContext::appendToBuffer(PBYTE pInBuf, size_t len)
{
    m_inBuf.write((PBYTE)pInBuf, len);
}

void ClientContext::appendToBuffer(const std::string& inBuf)
{
    m_inBuf.write(inBuf);
}

bool ClientContext::decodePacket()
{
    Slice header;
    HttpCodec::HttpState state;

    state = m_pCodec->getHeader(m_inBuf, header);
    
    Slice line;

    Slice startLine;
    state = m_pCodec->decodeHeader(header, startLine);


    return false;
}
