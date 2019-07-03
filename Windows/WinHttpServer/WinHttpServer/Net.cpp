#include "pch.h"
#include "Net.h"

#include <iostream>

using namespace std;

bool Net::init()
{
    WSADATA wsaData = { 0 };
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

bool Net::unInit()
{
    WSACleanup();
    return true;
}

SOCKET Net::WSASocket_()
{
    return ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
}

int Net::bind(SOCKET s, const LPSOCKADDR_IN pAddr)
{
    return ::bind(s, (LPSOCKADDR)pAddr, sizeof(SOCKADDR_IN));
}

int Net::listen(SOCKET s, int backlog)
{
    return ::listen(s, backlog);
}

bool Net::setKeepAlive(SOCKET socket, bool on)
{
    DWORD opt = on;
    int ret = setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt, sizeof(DWORD));
    if (0 != ret)
    {
        cout << "setsockopt failed" << endl;
        return false;
    }
    return true;
}
