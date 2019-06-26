#include "pch.h"
#include "Net.h"

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
