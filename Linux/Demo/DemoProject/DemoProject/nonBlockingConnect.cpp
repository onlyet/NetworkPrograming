#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
	
using namespace std;

//阻塞connect
bool blockingConnect(const char* ip, short port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		cout << "create socket error";
		return false;
	}
	sockaddr_in srvAddr;
	memset(&srvAddr, 0, sizeof(sockaddr_in));
	srvAddr.sin_addr.s_addr = htonl(atoi(ip));
	srvAddr.sin_port = port;
	srvAddr.sin_family = AF_INET;

	//阻塞connect
	int ret = connect(fd, (sockaddr*)&srvAddr, sizeof(sockaddr_in));
	if (ret == 0)
	{
		cout << "connect successfully";
		return true;
	}
	if (errno != EINTR)
	{
		cout << "can not connect to server, errno: " << errno;
		return false;
	}

	//select()

	cout << "can not connect to server, errno: " << errno;
	return false;
}


//getsockopt判断连接
bool nonblockingConnect(const char* ip, short port, int timeout = 3)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        cout << "create socket failed" << endl;
        return false;
    }

    //设置非阻塞
	int flag = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1)
	{
		cout << "fcntl failed" << endl;
		close(fd);
		return false;
	}

	struct sockaddr_in srvAddr;
	memset(&srvAddr, 0, sizeof(struct sockaddr_in));
	//srvAddr.sin_addr.s_addr = htonl(atoi(ip));
	srvAddr.sin_addr.s_addr = inet_addr(ip);
	srvAddr.sin_port = htons(port);
	srvAddr.sin_family = AF_INET;

	int ret = connect(fd, (sockaddr*)&srvAddr, sizeof(struct sockaddr_in));
	if (ret == 0)
	{
		cout << "connect successfully" << endl;
		return true;
	}
	else if (ret == -1 && errno != EINPROGRESS && errno != EINTR)
	{
        cout << "can not connect to server, errno: " << errno << endl;
		close(fd);
		return false;
	}

	fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
	timeval tv = { timeout, 0 };

	ret = select(fd + 1, nullptr, &wfds, nullptr, &tv);
	if (ret <= 0)
	{
		cout << "can not connect to server" << endl;
		close(fd);
		return false;
	}

	if (FD_ISSET(fd, &wfds))
	{
		int error;
		socklen_t error_len = sizeof(int);
		ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
		if (ret == -1)
		{
			cout << "getsockopt failed" << endl;
			return false;
		}
		if (error == 0)
		{
			cout << "connect successfully" << endl;

			while (1)
			{
				int send_len = 0;
				const char buf[] = "hello\n";

				if ((send_len = send(fd, buf, sizeof(buf), 0)) == -1)
				{
					cout << "send failed";
					return false;
				}
				cout << "send len: " << send_len << endl;
				sleep(3);
			}

			return true;
		}
	}

    cout << "can not connect to server, errno: " << errno << endl;
	close(fd);
	return false;
}

//重connect判断连接
//本机虚拟机测试，第一次重connect返回值仍然是EINPROGRESS,第二次才返回EISCONN
bool nonblockingConnect_v2(const char* ip, short port, int timeout = 3)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		cout << "create socket failed" << endl;
		return false;
	}

	//设置非阻塞
	int flag = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1)
	{
		cout << "fcntl failed" << endl;
		close(fd);
		return false;
	}

	struct sockaddr_in srvAddr;
	memset(&srvAddr, 0, sizeof(struct sockaddr_in));
	//srvAddr.sin_addr.s_addr = htonl(atoi(ip));
	srvAddr.sin_addr.s_addr = inet_addr(ip);
	srvAddr.sin_port = htons(port);
	srvAddr.sin_family = AF_INET;

	int ret = connect(fd, (sockaddr*)&srvAddr, sizeof(struct sockaddr_in));
	if (ret == 0)
	{
		cout << "connect successfully" << endl;
		return true;
	}
	else if (ret == -1 && errno != EINPROGRESS)
	{
		cout << "can not connect to server, errno: " << errno << endl;
		close(fd);
		return false;
	}

	fd_set wfds;
	FD_ZERO(&wfds);
	FD_SET(fd, &wfds);
	timeval tv = { timeout, 0 };

	while (1)
	{
		ret = select(fd + 1, nullptr, &wfds, nullptr, &tv);
		if (ret <= 0)
		{
			cout << "can not connect to server" << endl;
			close(fd);
			return false;
		}

		if (FD_ISSET(fd, &wfds))
		{
			//重新connect，如果EISCONN则表明连接已建立
			ret = connect(fd, (sockaddr*)&srvAddr, sizeof(sockaddr_in));
			if (errno == EISCONN)
			{
				cout << "connect successfully" << endl;

				while (1)
				{
					int send_len = 0;
					const char buf[] = "hello\n";

					if ((send_len = send(fd, buf, sizeof(buf), 0)) == -1)
					{
						cout << "send failed";
						return false;
					}
					cout << "send len: " << send_len << endl;
					sleep(3);
				}

				return true;
			}
			else
			{
				cout << "repet while, errno:" << errno << endl;
			}
		}
	}

	cout << "can not connect to server, errno: " << errno << endl;
	close(fd);
	return false;
}

//poll判断连接
bool nonblockingConnect_v3(const char* ip, short port, int timeout = 3)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		cout << "create socket failed" << endl;
		return false;
	}

	//设置非阻塞
	int flag = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1)
	{
		cout << "fcntl failed" << endl;
		return false;
	}

	struct sockaddr_in srvAddr;
	memset(&srvAddr, 0, sizeof(struct sockaddr_in));
	srvAddr.sin_addr.s_addr = inet_addr(ip);
	srvAddr.sin_port = htons(port);
	srvAddr.sin_family = AF_INET;

	int ret = connect(fd, (sockaddr*)&srvAddr, sizeof(struct sockaddr_in));
	if (ret == 0)
	{
		cout << "connect successfully" << endl;
		return true;
	}
	else if (ret == -1 && errno != EINPROGRESS && errno != EINTR)
	{
		cout << "can not connect to server, errno: " << errno << endl;
		close(fd);
		return false;
	}

	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLOUT | POLLERR;

	ret = poll(&pfd, 1, timeout * 1000);
	if (ret == 1 && pfd.revents == POLLOUT)
	{
		cout << "connect successfully" << endl;

		while (1)
		{
			int send_len = 0;
			const char buf[] = "hello\n";

			if ((send_len = send(fd, buf, sizeof(buf), 0)) == -1)
			{
				cout << "send failed";
				return false;
			}
			cout << "send len: " << send_len << endl;
			sleep(3);
		}

		return true;
	}

	cout << "can not connect to server, errno: " << errno << endl;
	close(fd);
	return false;
}

int main()
{
	//nonblockingConnect("127.0.0.1", 5000);
	//nonblockingConnect_v2("127.0.0.1", 5000);
	nonblockingConnect_v3("104.18.93.225", 80);
	
	return 0;
}