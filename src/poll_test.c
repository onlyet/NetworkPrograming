#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <poll.h>

#define MAX_EVENT 1024
#define BUF_SIZE 1024

void error_handle(const char* err_msg)
{
	puts(err_msg);
	exit(1);
}

void set_nonblocking(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	if (flag < 0)
	{
		perror("fcntl 1 error\n");
		return;
	}
	if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) < 0)
	{
		perror("fcntl 2 error\n");
		return;
	}

}

int main(int argc, char* argv[])
{
	int i, listenfd, connfd, epfd, triggered_fd, addr_len;
	int nread, nwrite;
	struct sockaddr_in clnt_addr, srv_addr;
	struct epoll_event cared_event, triggered_events[MAX_EVENT];
	char send_buf[BUF_SIZE], recv_buf[BUF_SIZE];

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error_handle("socket error\n");

	//set_nonblocking(listenfd);

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	//srv_addr.sin_addr.s_addr = inet_addr(argv[0]);
	//srv_addr.sin_addr.s_addr = inet_addr(INADDR_ANY);	//为什么错？
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	srv_addr.sin_port = htons(atoi(argv[1]));
	int err;
	if ((err = bind(listenfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr))) < 0)
	{
		printf("err = %d\n", err);
		printf("errno = %d\n", errno);
		error_handle("bind error\n");
	}

	if (listen(listenfd, 10) < 0)
		error_handle("listen error\n");

	if ((connfd = accept(listenfd, (struct sockaddr*)&clnt_addr, &addr_len)) < 0)
	{
		printf("errno = %d\n", errno);
		error_handle("accept error\n");
	}

	printf("begin\n");

	struct pollfd event;
	event.fd = connfd;
	event.events = POLLOUT | POLLERR;

	int nready = poll(&event, 1, -1);
	if (nready == 1 && event.revents == POLLOUT)
	{
		printf("POLLOUT\n");
		return 0;
	}
	else if (nready == 1 && event.revents == POLLERR)
	{
		printf("POLLERR\n");
		return 0;
	}
	else
		printf("nready = %d, revents = %d\n", nready, event.revents);	//revents = 32表示无效的poller请求

	printf("end\n");
	return 0;
}
