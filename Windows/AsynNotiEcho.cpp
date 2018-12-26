#include <WinSock2.h>
#include <stdio.h>
#include <string.h>
//#include <winsock2.h>

#define BUF_SIZE 100

void CompressSockets(SOCKET hSockArr[], int idx, int total);
void CompressEvents(WSAEVENT hEventArr[], int idx, int total);
void ErrorHandling(char* msg);

int main(int argc, char* argv[])
{
	WSADATA wsaData = { 0 };
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;

	SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS] = { 0 };
	WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS] = { 0 };
	WSAEVENT newEvent;
	WSANETWORKEVENTS netEvents;

	int numofClntSock = 0;
	int strLen, i;
	int posInfo, startIdx;
	int clntAdrLen;
	char msg[BUF_SIZE] = { 0 };

	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup error");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen error");

	newEvent = WSACreateEvent();
	if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
		ErrorHandling("WSAEventSelect erorr");
	
	hSockArr[numofClntSock] = hServSock;
	hEventArr[numofClntSock] = newEvent;
	++numofClntSock;

	while (1)
	{
		posInfo = WSAWaitForMultipleEvents(numofClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);	//×èÈû
		startIdx = posInfo - WSA_WAIT_EVENT_0;

		for (i = startIdx; i < numofClntSock; ++i)
		{
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);	//Á¢¼´·µ»Ø
			if (sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT)
				continue;
			else
			{
				sigEventIdx = i;
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);
				if (netEvents.lNetworkEvents & FD_ACCEPT)
				{
					if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
					{
						puts("accept error");
						break;
					}
					clntAdrLen = sizeof(clntAdr);
					hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAdr, &clntAdrLen);
					newEvent = WSACreateEvent();
					if (WSAEventSelect(hClntSock, newEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
						ErrorHandling("WSAEventSelect erorr");
					hSockArr[numofClntSock] = hClntSock;
					hEventArr[numofClntSock] = newEvent;
					++numofClntSock;
					printf("connected new cliend fd:%d\n", hClntSock);
				}
				if (netEvents.lNetworkEvents & FD_READ)
				{
					if (netEvents.iErrorCode[FD_READ_BIT] != 0)
					{
						puts("read error");
						break;
					}
					memset(msg, 0, BUF_SIZE);
					strLen = recv(hSockArr[sigEventIdx], msg, BUF_SIZE - 1, 0);
					printf("recv fd:%d, msg:%s\n", hSockArr[sigEventIdx], msg);
					send(hSockArr[sigEventIdx], msg, strLen, 0);
				}
				if (netEvents.lNetworkEvents & FD_CLOSE)
				{
					if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0)
					{
						puts("close error");
						break;
					}
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);
					--numofClntSock;
					CompressSockets(hSockArr, sigEventIdx, numofClntSock);
					CompressEvents(hEventArr, sigEventIdx, numofClntSock);
				}
			}
		}
	}
	WSACleanup();
	system("pause");
	return 0;
}

void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
	int i;
	for (i = idx; i < total; ++i)
		hSockArr[i] = hSockArr[i + 1];
}

void CompressEvents(WSAEVENT hEventArr[], int idx, int total)
{
	int i;
	for (i = idx; i < total; ++i)
		hEventArr[i] = hEventArr[i + 1];
}

void ErrorHandling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

