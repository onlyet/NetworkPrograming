#include "pch.h"
#include "Timer.h"
#include "IocpServer.h"
#include "ClientContext.h"
#include "LockGuard.h"
#include <iostream>

using namespace std;

unsigned HbTimer::checkHeartbeatWorker(LPVOID arg)
{
    IocpServer* pIocpServer = static_cast<IocpServer*>(arg);
    if (nullptr == pIocpServer)
        return 0;

    HANDLE hTimer = NULL;
    LARGE_INTEGER liDueTime;

    liDueTime.QuadPart = -10000000LL * 1;   //1√Î

    hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (NULL == hTimer)
    {
        cout << "CreateWaitableTimer failed" << endl;
        return 1;
    }

    if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
    {
        cout << "SetWaitableTimer failed" << endl;
        return 2;
    }

    while (1)
    {
        if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
        {
            cout << "WaitForSingleObject failed" << endl;
            return 1;
        }

        DWORD curTime = GetTickCount();
        DWORD timeout = 10 * 1000;

        std::list<ClientContext*>* pConnectedClientList = &pIocpServer->m_connectedClientList;

        LockGuard lk(&pIocpServer->m_csClientList);
        {
            for (auto it = pConnectedClientList->begin(); it != pConnectedClientList->end(); ++it)
            {
                ClientContext* pClientCtx = *it;
                LockGuard lk(&pClientCtx->m_csLock);
                {
                    if (curTime - pClientCtx->m_nLastHeartbeatTime > timeout)
                    {
                        //postClose
                        if (FALSE == PostQueuedCompletionStatus(pIocpServer->m_hComPort, 0, (ULONG_PTR)pClientCtx, NULL))
                        {
                            cout << "PostQueuedCompletionStatus failed with error: " << WSAGetLastError() << endl;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
        {
            cout << "SetWaitableTimer failed" << endl;
            return 2;
        }

    }

    return 0;
}
