#include "pch.h"
#include "IoContext.h"
#include <iostream>

using namespace std;

IoContext::IoContext(PostType type, SOCKET s) :
    m_postType(type), m_socket(s)
{
    SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
    SecureZeroMemory(m_ioBuf, IO_BUF_SIZE);
    m_wsaBuf.buf = m_ioBuf;
    m_wsaBuf.len = IO_BUF_SIZE;
}

IoContext::~IoContext()
{
    cout << "IoContext::~IoContext()" << endl;
}

void IoContext::resetBuffer()
{
    SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
    SecureZeroMemory(m_ioBuf, IO_BUF_SIZE);
}
