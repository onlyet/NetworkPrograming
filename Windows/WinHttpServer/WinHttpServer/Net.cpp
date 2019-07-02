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

bool Net::associateWithCompletionPort(HANDLE completionPort, ClientContext* pConnClient)
{
    HANDLE hRet = CreateIoCompletionPort((HANDLE)pConnClient->m_socket, completionPort, (ULONG_PTR)pConnClient, 0);
    if (NULL == hRet)
    {
        cout << "failed to associate the accept socket with completion port" << endl;
        return false;
    }
    return true;
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
