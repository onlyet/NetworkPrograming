#include "Common.h"
#include <WinSock2.h>
#include <iostream>

using namespace std;

//×èÈûconnect
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
        //³É¹¦
    }
    else if (SOCKET_ERROR == ret)
    {
        int ret = WSAGetLastError();
    }
}


//·Ç×èÈûconnect
bool nonblockingConnect(const char* ip, short port, int timeout = 3)
{
    SOCKET sock;
    SOCKADDR_IN srvAddr;
    memset(&srvAddr, 0, sizeof(SOCKADDR_IN));
    srvAddr.sin_addr.s_addr = htonl(atoi(""));
    srvAddr.sin_port = 6000;
    srvAddr.sin_family = AF_INET;

    int ret = connect(sock, (PSOCKADDR)&srvAddr, sizeof(SOCKADDR_IN));
    if (ret == 0)
    {
        cout << "connect success" << endl;
        return true;
    }
    else if (ret == SOCKET_ERROR || WSAGetLastError() != WSAEWOULDBLOCK)
    {
        cout << "can not connect to server";
        return false;
    }

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sock, &wfds);
    timeval tv{ timeout, 0 };

    ret = select(sock + 1, nullptr, &wfds, nullptr, &tv);
    if (ret != 1)
    {
        cout << "can not connect to server";
        return false;
    }
    cout << "connect success" << endl;
    return true;
}