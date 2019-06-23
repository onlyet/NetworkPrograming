#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <fcntl.h>
	
using namespace std;

//阻塞connect
void blockingConnect()
{
	int sock;
	sockaddr_in srvAddr;
	memset(&srvAddr, 0, sizeof(sockaddr_in));
	srvAddr.sin_addr.s_addr = htonl(atoi(""));
	srvAddr.sin_port = 6000;
	srvAddr.sin_family = AF_INET;

	int ret = connect(sock, (sockaddr*)&srvAddr, sizeof(sockaddr_in));
	if (ret == 0)
	{
		//成功
	}
	else if (ret == -1)
	{
        int ret = errno;
	}
}


//非阻塞connect
bool nonblockingConnect(const char* ip, short port, int timeout = 3)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        cout << "create socket failed" << endl;
        return false;
    }

    //设置非阻塞
    int flag = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);

	sockaddr_in srvAddr;
	memset(&srvAddr, 0, sizeof(sockaddr_in));
	srvAddr.sin_addr.s_addr = htonl(atoi(ip));
	srvAddr.sin_port = port;
	srvAddr.sin_family = AF_INET;

	int ret = connect(fd, (sockaddr*)&srvAddr, sizeof(sockaddr_in));
	if (ret == 0)
	{
		cout << "connect success" << endl;
		return true;
	}
	else if (ret == -1 || errno != EINPROGRESS)
	{
        cout << "can not connect to server, errno: " << errno << endl;
		return false;
	}

	fd_set rfds, wfds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_SET(fd, &rfds);
    FD_SET(fd, &wfds);
	timeval tv{ timeout, 0 };

	ret = select(fd + 1, &rfds, &wfds, nullptr, &tv);
	if (ret <= 0)
	{
		cout << "can not connect to server" << endl;
		return false;
	}

    if (FD_ISSET(fd, &rfds) || FD_ISSET(fd, &wfds))
    {
        //重新connect，如果EISCONN则表明连接已建立
        ret = connect(fd, (sockaddr*)&srvAddr, sizeof(sockaddr_in));
        if (errno == EISCONN)
        {
            cout << "connect success" << endl;
            return true;
        }
    }

    cout << "can not connect to server" << endl;
	return false;
}