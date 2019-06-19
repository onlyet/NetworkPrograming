#include "Common.h"
#include <WinSock2.h>

//阻塞connect
void nonblockingConnect()
{
    SOCKET sock;
    SOCKADDR_IN srvAddr;
    memset(&srvAddr, 0, sizeof(SOCKADDR_IN));
    srvAddr.sin_addr.s_addr = htonl(atoi(""));
    srvAddr.sin_port = 6000;
    srvAddr.sin_family = AF_INET;

    int ret = connect(sock, (PSOCKADDR)&srvAddr, sizeof(SOCKADDR_IN));
    if (0 == ret)
    {
        //成功
    }
    else if (SOCKET_ERROR == ret)
    {
        int ret = WSAGetLastError();
    }
}


//非阻塞connect
void nonblockingConnect()
{
    SOCKET sock;
    SOCKADDR_IN srvAddr;
    memset(&srvAddr, 0, sizeof(SOCKADDR_IN));
    srvAddr.sin_addr.s_addr = htonl(atoi(""));
    srvAddr.sin_port = 6000;
    srvAddr.sin_family = AF_INET;

    int ret = connect(sock, (PSOCKADDR)&srvAddr, sizeof(SOCKADDR_IN));
    if (0 == ret)
    {
        //成功
    }
    else if (SOCKET_ERROR == ret)
    {
        int err = WSAGetLastError();
        if (WSAEWOULDBLOCK == err)
        {
            fd_set wfds;
            
            select(sock + 1, )
        }
    }
}