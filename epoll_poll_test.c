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

#define MAX_EVENT 1024
#define BUF_SIZE 1024

void error_handle(const char* err_msg)
{
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
	int i, listenfd, connfd, epfd, triggered_fd, nready, addr_len;
	int nread, nwrite;
	struct sockaddr_in clnt_addr, srv_addr;
	struct epoll_event cared_event, triggered_events[MAX_EVENT];
	char send_buf[BUF_SIZE], recv_buf[BUF_SIZE];
	
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error_handle("socket error\n");

	set_nonblocking(listenfd);

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	//srv_addr.sin_addr.s_addr = inet_addr(argv[0]);
	srv_addr.sin_addr.s_addr = inet_addr(INADDR_ANY);
	srv_addr.sin_port = htons(atoi(argv[1]));
	if (bind(listenfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0)
		error_handle("bind error\n");

	if (listen(listenfd, 10) < 0)
		error_handle("listen error\n");

	if ((epfd = epoll_create1(EPOLL_CLOEXEC)) < 0)
		error_handle("epoll_create1 error\n");

	cared_event.data.fd = listenfd;
	cared_event.events = EPOLLIN | EPOLLERR;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &cared_event) < 0)
		error_handle("epoll_ctl error\n");

	while (1)
	{
		nready = epoll_wait(epfd, triggered_events, MAX_EVENT, -1);
		if (nready == -1)
		{
			printf("epoll_wait error\n");
			break;
		}
		for (i = 0; i < nready; ++i)
		{
			triggered_fd = triggered_events[i].data.fd;
			if (triggered_events[i].events & EPOLLIN)
			{
				if (triggered_fd == listenfd)
				{
					addr_len = sizeof(clnt_addr);
					if ((connfd = accept(listenfd, (struct sockaddr*)&clnt_addr, &addr_len)) < 0)
						error_handle("accept error\n");

					set_nonblocking(connfd);

					cared_event.data.fd = connfd;
					cared_event.events = EPOLLIN;

					if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &cared_event) < 0)
						error_handle("epoll_ctl error\n");
					printf("connected client fd = %d\n", connfd);
				}
				else  //连接fd
				{
					if ((nread = read(triggered_fd, recv_buf, BUF_SIZE)) < 0)
					{
						if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
							continue;
						else
							error_handle("read error\n");
					}
					else if (nread == 0)
					{
						printf("client closed\n");
						close(triggered_fd);
					}
					else
					{
						if ((nwrite = write(triggered_fd, send_buf, sizeof(send_buf))) < 0)
						{
							if (nwrite == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
							{
								cared_event.data.fd = triggered_fd;
								cared_event.events = EPOLLOUT;
								if (epoll_ctl(epfd, EPOLL_CTL_MOD, triggered_fd, &cared_event) < 0)
									error_handle("epoll_ctl error\n");
							}
							else
								error_handle("write error\n");
						}
					}
				}

			}
			else if(triggered_events[i].events & EPOLLOUT)
			{
				//if((nwrite = write(triggered_fd, send_buf, sizeof(send_buf))) < 0)

			}
			else
			{
				perror("epoll_wait error\n");
				return;
			}
		}

	}
	close(listenfd);
	close(epfd);

	return 0;
}
