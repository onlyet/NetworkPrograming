#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <WinSock2.h>
//#include <Windows.h>

#define BUF_SIZE 100
#define READ 3
#define WRITE 5

typedef struct
{
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode;
} PER_IO_DATA, *LPPER_IO_DATA;

unsigned WINAPI EchoThreadMain(void* CompletionPort);
void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData = { 0 };
	HANDLE comPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;
	SOCKET servSock;
	SOCKADDR_IN servAddr;
	DWORD recvBytes;
	DWORD flags = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup error");

	//根据CPU核数创建IO线程
	comPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&sysInfo);
	for (int i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
	{
		_beginthreadex(NULL, 0, EchoThreadMain, comPort, 0, NULL);
	}
	//具有重叠功能的套接字
	servSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = PF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind error");

	if (listen(servSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen error");

	while (1)
	{
		SOCKET clntSock;
		SOCKADDR_IN clntAddr;
		int addrLen = sizeof(clntAddr);

		//servSock是阻塞的
		clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &addrLen);
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = clntSock;
		memcpy(&handleInfo->clntAddr, &clntAddr, addrLen);

		//将客户套接字与完成端口关联
		CreateIoCompletionPort((HANDLE)clntSock, comPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(ioInfo, 0, sizeof(PER_IO_DATA));
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->rwMode = READ;
		//ioInfo->overlapped->hEvent为空
		WSARecv(handleInfo->hClntSock, &ioInfo->wsaBuf, 1, &recvBytes, &flags, &ioInfo->overlapped, NULL);
	}
	system("pause");
	return 0;
}

unsigned WINAPI EchoThreadMain(void * pComPort)
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
		sock = handleInfo->hClntSock;
		if (ioInfo->rwMode == READ)
		{
			printf("msg receive from client %d\n", sock);
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
			WSASend(sock, &ioInfo->wsaBuf, 1, NULL, 0, &ioInfo->overlapped, NULL);

			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(ioInfo, 0, sizeof(PER_IO_DATA));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &ioInfo->wsaBuf, 1, NULL, &flags, &ioInfo->overlapped, NULL);
		}
		else
		{
			printf("msg sended to client %d\n", sock);
			free(ioInfo);
		}
	}
	return 0;
}
