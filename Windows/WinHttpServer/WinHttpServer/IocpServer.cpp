#include "pch.h"
#include "IocpServer.h"
#include "Net.h"
#include <iostream>

#include <process.h>
#include <MSWSock.h>

using namespace std;

constexpr int POST_ACCEPT_CNT = 10;

IocpServer::IocpServer(short listenPort) :
	m_listenPort(listenPort)
	, m_pListenClient(nullptr)
	, m_nWorkerCnt(0)
{
}

IocpServer::~IocpServer()
{
	if (INVALID_SOCKET != m_pListenClient->m_socket)
		closesocket(m_pListenClient->m_socket);

	Net::unInit();
}

bool IocpServer::init()
{
    if (!Net::init())
    {
        cout << "network initial failed" << endl;
        return false;
    }

	if (!createListenClient(m_listenPort))
		return false;

	if (!createIocpWorker())
		return false;


	//投递accept请求
	for (int i = 0; i < POST_ACCEPT_CNT; ++i)
	{
		IoContext* pListenIoCtx = m_pListenClient->createIoContext();
		if (!postAccept(pListenIoCtx))
		{
			m_pListenClient->removeIoContext(pListenIoCtx);
			return false;
		}
	}

    return true;
}

bool IocpServer::start()
{
    return false;
}

bool IocpServer::stop()
{
    return false;
}

unsigned WINAPI IocpServer::IocpWorkerThread(void* arg)
{


	return 0;
}

bool IocpServer::createListenClient(short listenPort)
{
	m_pListenClient = new ClientContext();

	//创建完成端口
	m_comPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_comPort)
		return false;

	//创建具有重叠功能的socket
	m_pListenClient->m_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (SOCKET_ERROR == m_pListenClient->m_socket)
	{
		cout << "create socket failed" << endl;
		return false;
	}

	//关联监听socket和完成端口
	CreateIoCompletionPort((HANDLE)m_pListenClient->m_socket, m_comPort, (ULONG_PTR)m_pListenClient, 0);

	SecureZeroMemory(&m_pListenClient->m_addr, sizeof(SOCKADDR));
	m_pListenClient->m_addr.sin_family = AF_INET;
	m_pListenClient->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_pListenClient->m_addr.sin_port = htons(listenPort);

	if (SOCKET_ERROR == bind(m_pListenClient->m_socket, (SOCKADDR*)&m_pListenClient->m_addr, sizeof(SOCKADDR)))
	{
		cout << "bind failed" << endl;
		return false;
	}

	if (SOCKET_ERROR == listen(m_pListenClient->m_socket, SOMAXCONN))
	{
		cout << "listen failed" << endl;
		return false;
	}

	//获取acceptEx函数指针
	DWORD dwBytes;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	int ret = WSAIoctl(m_pListenClient->m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&dwBytes, NULL, NULL);
	if (ret == SOCKET_ERROR)
	{
		cout << "WSAIoctl failed with error: " << WSAGetLastError();
		return false;
	}
	m_lpfnAcceptEx = lpfnAcceptEx;


	return true;
}

bool IocpServer::createIocpWorker()
{
	//根据CPU核数创建IO线程
	HANDLE hWorker;
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	for (DWORD i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
	{
		hWorker = (HANDLE)_beginthreadex(NULL, 0, &IocpServer::IocpWorkerThread, this,  0, NULL);
		if (NULL == hWorker)
		{
			CloseHandle(m_comPort);
			return false;
		}

		++m_nWorkerCnt;
	}

	return true;
}

bool IocpServer::postAccept(IoContext* pIoCtx)
{
	char* pBuf					= pIoCtx->m_wsaBuf.buf;
	ULONG nLen					= pIoCtx->m_wsaBuf.len;
	OVERLAPPED* pOverlapped		= &pIoCtx->m_overlapped;
	LPFN_ACCEPTEX lpfnAcceptEx	= (LPFN_ACCEPTEX)m_lpfnAcceptEx;

	pIoCtx->m_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (SOCKET_ERROR == pIoCtx->m_socket)
	{
		cout << "create socket failed" << endl;
		return false; 
	}

	DWORD dwRecvByte;
	if (FALSE == lpfnAcceptEx(m_pListenClient->m_socket, pIoCtx->m_socket, pBuf,
		nLen - ((sizeof(SOCKADDR) + 16) * 2), sizeof(SOCKADDR) + 16,
		sizeof(SOCKADDR) + 16, &dwRecvByte, pOverlapped))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			cout << "acceptEx failed" << endl;
			return false;
		}
	}

	return true;
}
