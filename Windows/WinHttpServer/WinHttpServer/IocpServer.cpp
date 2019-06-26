#include "pch.h"
#include "IocpServer.h"
#include "Net.h"
#include <iostream>

using namespace std;

IocpServer::~IocpServer()
{
}

bool IocpServer::init()
{
    if (!Net::init())
    {
        cout << "network initial failed" << endl;
        return false;
    }



    return false;
}

bool IocpServer::start()
{
    return false;
}

bool IocpServer::stop()
{
    return false;
}
