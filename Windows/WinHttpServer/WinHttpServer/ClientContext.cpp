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
{
    SecureZeroMemory(&m_addr, sizeof(SOCKADDR_IN));
}

ClientContext::~ClientContext()
{
    if (INVALID_SOCKET != m_socket)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    //std::for_each(m_ioCtxs.begin(), m_ioCtxs.end(),
    //    [](const std::pair<PostType, IoContext*>& pr) { delete pr.second; });
    //m_ioCtxs.erase(m_ioCtxs.begin(), m_ioCtxs.end());
}

//IoContext* ClientContext::getIoContext(PostType type)
//{
//    std::map<PostType, IoContext*>::iterator it = m_ioCtxs.find(type);
//    if (it != m_ioCtxs.end())
//    {
//        return m_ioCtxs[type];
//    }
//
//    SOCKET ioSocket;
//    if (ACCEPT_EVENT == type)
//    {
//        ioSocket = INVALID_SOCKET;
//    }
//    else
//    {
//        ioSocket = m_socket;
//    }
//    IoContext* ioCtx = new IoContext(type, ioSocket);
//    m_ioCtxs.insert(std::make_pair(type, ioCtx));
//    return ioCtx;
//}
//
//void ClientContext::removeIoContext(IoContext* pIoCtx)
//{
//    m_ioCtxs.erase(pIoCtx->m_postType);
//}

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
    //Slice header;
    //HttpCodec::HttpState state;

    //state = m_pCodec->getHeader(m_inBuf, header);
    //
    //state = m_pCodec->decodeHeader(header);


    return false;
}
