#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 100
#define READ 3
#define WRITE 5

const char port[] = "5000";

typedef struct
{
	SOCKET m_sock;
	SOCKADDR_IN m_addr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode;
} PER_IO_DATA, *LPPER_IO_DATA;

unsigned WINAPI EchoThreadMain(void* CompletionPort);

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

//#define 

int main(int argc, char* argv[])
{
	WSADATA wsaData = { 0 };
	HANDLE comPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;
	//SOCKET listenSock;
	//SOCKADDR_IN servAddr;
	DWORD recvBytes;
	DWORD flags = 0;

	LPPER_HANDLE_DATA listenHandleInfo;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	int ret;
	DWORD dwBytes;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup error");

	//根据CPU核数创建IO线程
	comPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&sysInfo);
	for (DWORD i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
	{
		_beginthreadex(NULL, 0, EchoThreadMain, comPort, 0, NULL);
	}

	listenHandleInfo = new PER_HANDLE_DATA;
	//具有重叠功能的socket
	listenHandleInfo->m_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	//关联监听socket和完成端口
	CreateIoCompletionPort((HANDLE)listenHandleInfo->m_sock, comPort, (ULONG_PTR)listenHandleInfo, 0);

	SecureZeroMemory(&listenHandleInfo->m_addr, sizeof(SOCKADDR));
	listenHandleInfo->m_addr.sin_family = AF_INET;
	//listenHandleInfo->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenHandleInfo->m_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	listenHandleInfo->m_addr.sin_port = htons(atoi(port));

	if (bind(listenHandleInfo->m_sock, (SOCKADDR*)&listenHandleInfo->m_addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
		ErrorHandling("bind error");

	if (listen(listenHandleInfo->m_sock, 5) == SOCKET_ERROR)
		ErrorHandling("listen error");

	ret = WSAIoctl(listenHandleInfo->m_sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&dwBytes, NULL, NULL);
	if (ret == SOCKET_ERROR) 
    {
		wprintf(L"WSAIoctl failed with error: %u\n", WSAGetLastError());
		closesocket(listenHandleInfo->m_sock);
		WSACleanup();
		return 1;
	}
	//AcceptEx()

	while (1)
	{
		SOCKET clntSock;
		SOCKADDR_IN clntAddr;
		int addrLen = sizeof(clntAddr);

		//listenSock是阻塞的
		clntSock = accept(listenHandleInfo->m_sock, (SOCKADDR*)&clntAddr, &addrLen);
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->m_sock = clntSock;
		memcpy(&handleInfo->m_addr, &clntAddr, addrLen);

		//将客户套接字与完成端口关联
		CreateIoCompletionPort((HANDLE)clntSock, comPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(ioInfo, 0, sizeof(PER_IO_DATA));
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->rwMode = READ;
		//ioInfo->overlapped->hEvent为空
		int ret = WSARecv(handleInfo->m_sock, &ioInfo->wsaBuf, 1, &recvBytes, &flags, &ioInfo->overlapped, NULL);
		if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "WSARecv failed" << endl;
			break;
		}
	}

	closesocket(listenHandleInfo->m_sock);
	WSACleanup();
	system("pause");
	return 0;
}

unsigned WINAPI EchoThreadMain(void* pComPort)
{
	HANDLE comPort = (HANDLE)pComPort;
	SOCKET sock;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	while (1)
	{
		//从完成端口取出一个IO完成包
		//为了使用其它字段，将OVERLAPPED结构与其它字段封装在一个结构体，然后传递结构体首地址（也是第一个成员OVERLAPPED的地址）
		GetQueuedCompletionStatus(comPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		sock = handleInfo->m_sock;
		if (ioInfo->rwMode == READ)
		{
			//printf("msg receive from client %d\n", sock);
            cout << "msg from client fd: " << sock << ", address: " <<
                inet_ntoa(handleInfo->m_addr.sin_addr) << ":" << ntohs(handleInfo->m_addr.sin_port) <<
                ", msg: " << ioInfo->buffer << endl;

			if (bytesTrans == 0)
			{
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				continue;
			}

			memset(&ioInfo->overlapped, 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->rwMode = WRITE;
			int ret = WSASend(sock, &ioInfo->wsaBuf, 1, NULL, 0, &ioInfo->overlapped, NULL);
			if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				cout << "WSASend failed" << endl;
				break;
			}

			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(ioInfo, 0, sizeof(PER_IO_DATA));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			ret = WSARecv(sock, &ioInfo->wsaBuf, 1, NULL, &flags, &ioInfo->overlapped, NULL);
			if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				cout << "WSARecv failed" << endl;
				break;
			}
		}
		else
		{
			printf("msg sended to client %d\n", sock);
			free(ioInfo);
		}
	}
	return 0;
}
