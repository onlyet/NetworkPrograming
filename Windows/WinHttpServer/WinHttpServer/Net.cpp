#include "pch.h"
#include "Net.h"

#include <iostream>

using namespace std;

bool Net::init()
{
    WSADATA wsaData = { 0 };
    return ::WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;

    //wsaData.wHighVersion != 2
        //wsaData.wVersion
}

bool Net::unInit()
{
    ::WSACleanup();
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

SOCKADDR_IN Net::getsockname(SOCKET s)
{
    SOCKADDR_IN addr;
    SecureZeroMemory(&addr, sizeof(SOCKADDR_IN));
    int addrLen = sizeof(SOCKADDR_IN);
    int ret = ::getsockname(s, (PSOCKADDR)&addr, &addrLen);
    if (SOCKET_ERROR == ret)
    {
        cout << "getsockname failed with error: " << WSAGetLastError() << endl;
    }
    return addr;
}

SOCKADDR_IN Net::getpeername(SOCKET s)
{
    SOCKADDR_IN addr;
    SecureZeroMemory(&addr, sizeof(SOCKADDR_IN));
    int addrLen = sizeof(SOCKADDR_IN);
    int ret = ::getpeername(s, (PSOCKADDR)&addr, &addrLen);
    if (SOCKET_ERROR == ret)
    {
        cout << "getpeername failed with error: " << WSAGetLastError() << endl;
    }
        return addr;
}

bool Net::setKeepAlive(SOCKET s, bool on)
{
    DWORD opt = on;
    int ret = ::setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt, sizeof(DWORD));
    if (SOCKET_ERROR == ret)
    {
        cout << "setsockopt failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool Net::setLinger(SOCKET s, bool on, int timeoutSecs)
{
    LINGER linger;
    linger.l_onoff = on;
    linger.l_linger = timeoutSecs;
    int ret = ::setsockopt(s, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(LINGER));
    if (SOCKET_ERROR == ret)
    {
        cout << "setsockopt failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool Net::updateAcceptContext(SOCKET listenSocket, SOCKET acceptSocket)
{
    int ret = ::setsockopt(acceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenSocket, sizeof(SOCKET));
    if (SOCKET_ERROR == ret)
    {
        cout << "setsockopt failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}
