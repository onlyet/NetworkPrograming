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
