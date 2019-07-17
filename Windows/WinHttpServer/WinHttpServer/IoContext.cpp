#include "pch.h"
#include "IoContext.h"
#include <iostream>

using namespace std;

IoContext::IoContext(PostType type) :
    m_postType(type)
{
    SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
}

IoContext::~IoContext()
{
    cout << "IoContext::~IoContext()" << endl;
}

void IoContext::resetBuffer()
{
    SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
}

AcceptIoContext::AcceptIoContext(SOCKET acceptSocket)
    : IoContext(PostType::ACCEPT_EVENT)
    , m_acceptSocket(acceptSocket)
{
    SecureZeroMemory(m_accpetBuf, IO_BUF_SIZE);
    m_wsaBuf.buf = (PCHAR)m_accpetBuf;
    m_wsaBuf.len = IO_BUF_SIZE;
}

AcceptIoContext::~AcceptIoContext()
{
}

void AcceptIoContext::resetBuffer()
{
    SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
    SecureZeroMemory(&m_accpetBuf, IO_BUF_SIZE);
}

RecvIoContext::RecvIoContext()
    : IoContext(PostType::RECV_EVENT)
{
    SecureZeroMemory(&m_recvBuf, IO_BUF_SIZE);
    m_wsaBuf.buf = (PCHAR)m_recvBuf;
    m_wsaBuf.len = IO_BUF_SIZE;
}

RecvIoContext::~RecvIoContext()
{
}

void RecvIoContext::resetBuffer()
{
    SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
    SecureZeroMemory(&m_recvBuf, IO_BUF_SIZE);

}
