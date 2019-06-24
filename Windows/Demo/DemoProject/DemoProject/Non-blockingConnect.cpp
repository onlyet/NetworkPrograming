#include "Common.h"
#include <WinSock2.h>
#include <iostream>

using namespace std;

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
bool nonblockingConnect(const char* ip, short port, int timeout = 3)
{
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET)
    {
        cout << "create socket failed, error: " << WSAGetLastError() << endl;
        return false;
    }

    //设置为非阻塞
    u_long nonBlock = 1;
    int ret = ioctlsocket(fd, FIONBIO, &nonBlock);
    if (ret == SOCKET_ERROR)
    {
        cout << "ioctlsocket failed" << endl;
        closesocket(fd);
        false;
    }

    SOCKADDR_IN srvAddr;
    memset(&srvAddr, 0, sizeof(SOCKADDR_IN));
    srvAddr.sin_addr.s_addr = inet_addr(ip);
    srvAddr.sin_port = htons(port);
    srvAddr.sin_family = AF_INET;

    ret = connect(fd, (PSOCKADDR)&srvAddr, sizeof(SOCKADDR_IN));
    if (ret == 0)
    {
        cout << "connect success" << endl;
        return true;
    }
    else if (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        cout << "can not connect to server, error: " << WSAGetLastError() << endl;
        return false;
    }

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
    timeval tv = { timeout, 0 };

    ret = select(fd + 1, nullptr, &wfds, nullptr, &tv);
    if (ret != 1)
    {
        cout << "can not connect to server";
        return false;
    }
    cout << "connect success" << endl;
    return true;
}