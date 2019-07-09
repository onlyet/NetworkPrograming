#include "pch.h"
#include "IoContext.h"
#include <iostream>

using namespace std;

IoContext::IoContext(PostType type, SOCKET s) :
    m_postType(type), m_socket(s)
    //, m_buf(nullptr)
{
    SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
    //SecureZeroMemory(m_ioBuf, IO_BUF_SIZE);
    //m_wsaBuf.buf = m_ioBuf;
    //m_wsaBuf.len = IO_BUF_SIZE;
}

IoContext::~IoContext()
{
    cout << "IoContext::~IoContext()" << endl;
    
    ////将buf放回内存池
    //delete m_buf;
    //m_buf = nullptr;
}

IoContext* IoContext::newIoContext(PostType type, SOCKET s)
{
    IoContext* pIoCtx = new IoContext(type, s);

    return pIoCtx;
}

void IoContext::newBuffer()
{
    //m_buf = new Buffer();
    //m_wsaBuf.buf = (char*)m_buf->begin();
    //m_wsaBuf.len = m_buf->length();
}

void IoContext::resetBuffer()
{
    SecureZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
    //SecureZeroMemory(m_ioBuf, IO_BUF_SIZE);
}
