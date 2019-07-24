// WinHttpServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "IocpServer.h"
#include "HttpServer.h"
#include <iostream>

using namespace std;

int main()
{
    {
        IocpServer server(5001);
        //HttpServer server(5001);
        bool ret = server.start();
        if (!ret)
        {
            cout << "start failed" << endl;
            return 0;
        }

        while (1)
        {
            Sleep(1000);
        }
    }

    system("pause");
    return 0;
}
