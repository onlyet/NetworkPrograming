#include "pch.h"
#include "IoContext.h"

IoContext::IoContext(PostType type) : 
    m_postType(type)
{
    SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
    SecureZeroMemory(m_ioBuf, IO_BUF_SIZE);
    m_wsaBuf.buf = m_ioBuf;
    m_wsaBuf.len = IO_BUF_SIZE;
}

void IoContext::resetBuffer()
{
    SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
    SecureZeroMemory(m_ioBuf, IO_BUF_SIZE);
}
