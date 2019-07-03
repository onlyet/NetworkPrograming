#include "pch.h"
#include "ClientContext.h"
#include "IoContext.h"
#include "Codec.h"

ClientContext::ClientContext(const SOCKET& socket) :
    m_socket(socket)
{
}

ClientContext::~ClientContext()
{
    if (INVALID_SOCKET != m_socket)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    std::for_each(m_ioCtxs.begin(), m_ioCtxs.end(),
        [](const std::pair<PostType, IoContext*>& pr) { delete pr.second; });
    m_ioCtxs.erase(m_ioCtxs.begin(), m_ioCtxs.end());
}

IoContext * ClientContext::getIoContext(PostType type)
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

void ClientContext::removeIoContext(IoContext* pIoCtx)
{
    m_ioCtxs.erase(pIoCtx->m_postType);
}

void ClientContext::appendToBuffer(const char* inBuf, size_t len)
{
    EnterCriticalSection(&m_csInBuf);
    m_inBuf.append(inBuf, len);
    LeaveCriticalSection(&m_csInBuf);
}

void ClientContext::appendToBuffer(const std::string& inBuf)
{
    appendToBuffer(inBuf.data(), inBuf.size());
}

bool ClientContext::decodePacket()
{
    Slice header;
    HttpCodec::HttpState state;

    state = m_pCodec->getHeader(m_inBuf, header);
    
    state = m_pCodec->decodeHeader(header);


    return false;
}
