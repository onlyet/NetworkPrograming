#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024

void CALLBACK ReadCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

//传给WSAOVERLAPPED
typedef struct
{
	SOCKET hClntSock;
	char buf[BUF_SIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA; 

int main(int argc, char* argv[])
{
	WSADATA wsaData = { 0 };
	SOCKET listenFd, recvFd;
	SOCKADDR_IN listenAddr, recvAddr;
	LPWSAOVERLAPPED lpOvLp;
	DWORD recvBytes;
	LPPER_IO_DATA hbInfo;
	unsigned long mode = 1;
	int recvAddrSz;
	DWORD flagInfo = 0;
	if (argc < 2) {
		printf("Usage: %s <PORT>\n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup error");

	listenFd = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	ioctlsocket(listenFd, FIONBIO, &mode);
	memset(&listenAddr, 0, sizeof(listenAddr));
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenAddr.sin_port = htons(atoi(argv[1]));

	if (bind(listenFd, (SOCKADDR*)&listenAddr, sizeof(listenAddr)) == SOCKET_ERROR)
		ErrorHandling("bind error");

	if(listen(listenFd, 5) == SOCKET_ERROR)
		ErrorHandling("listen error");

	recvAddrSz = sizeof(recvAddr);
	while (1)
	{
		//线程进入alertable wait状态
		//检查APC队列中是否有回调函数，有则取回线程，然后执行
		SleepEx(100, TRUE);
		recvFd = accept(listenFd, (SOCKADDR*)&recvAddr, &recvAddrSz);
		if (recvFd == INVALID_SOCKET)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			else
				ErrorHandling("accept error");
		}
		printf("connected new cliend fd:%d\n", recvFd);

		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		hbInfo->hClntSock = recvFd;
		hbInfo->wsaBuf.buf = hbInfo->buf;
		hbInfo->wsaBuf.len = BUF_SIZE;

		lpOvLp->hEvent = (HANDLE)hbInfo;
		WSARecv(recvFd, &hbInfo->wsaBuf, 1, &recvBytes, &flagInfo, lpOvLp, ReadCompRoutine);
	}
	closesocket(recvFd);
	closesocket(listenFd);
	WSACleanup();
	return 0;
}

//数据已经接收到内核缓冲区
void CALLBACK ReadCompRoutine(
	DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &hbInfo->wsaBuf;
	DWORD sendBytes;

	if (szRecvBytes == 0)
	{
		printf("client %d disconnected\n", hSock);
		closesocket(hSock);
		free(lpOverlapped->hEvent);
		free(lpOverlapped);
	}
	else
	{
		bufInfo->len = szRecvBytes;
		WSASend(hSock, bufInfo, 1, &sendBytes, flags, lpOverlapped, WriteCompRoutine);
	}
}

void CALLBACK WriteCompRoutine(
	DWORD dwError, DWORD szSendBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &hbInfo->wsaBuf;
	DWORD recvBytes;
	DWORD flagInfo = 0;
	WSARecv(hSock, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}




