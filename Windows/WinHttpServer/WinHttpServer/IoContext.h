#ifndef __IO_CONTEXT_H__
#define __IO_CONTEXT_H__

#include "Global.h"
#include "Buffer.h"

struct IoContext
{
    IoContext(PostType type, SOCKET s = INVALID_SOCKET);
    ~IoContext();

    static IoContext* newIoContext(PostType type, SOCKET s = INVALID_SOCKET);

    void newBuffer();

    void resetBuffer();

    OVERLAPPED      m_overlapped;			//每一个重叠io操作都要有一个OVERLAPPED结构
    PostType        m_postType;
    SOCKET			m_socket;				//当前进行IO操作的socket
    DWORD           m_dwBytesTransferred;   //本次io传输的字节数
    WSABUF          m_wsaBuf;				//重叠io需要的buf
    //char            m_ioBuf[IO_BUF_SIZE];
    //Buffer*         m_buf;
};


#endif // !__IO_CONTEXT_H__
