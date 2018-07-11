#include <stdio.h>
#include <sys/socket.h>
#include <netinet\in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <string>

//class EventHandle {
//public:
//	EventHandle() {}
//	virtual ~EventHandle() {}
//	virtual int GetHandle() = 0;
//
//private:
//	int fd;
//};

#define MAX_EVENT 1024
#define errorif(cond, err_msg) {if(cond) perror(err_msg);}

int poller(char *argv[])
{
	int ret;
	int listenfd, epfd;
	struct sockaddr_in srv_addr;
	epoll_event triggered_event[MAX_EVENT], ev;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	errorif(listenfd < 0, "socket");
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &srv_addr.sin_addr);
	srv_addr.sin_port = htons(atoi(argv[2]));
	ret = bind(listenfd, (sockaddr*)&srv_addr, sizeof(srv_addr));
	errorif(ret < 0, "bind");
	ret = listen(listenfd, 20);
	errorif(ret < 0, "listen");

	epfd = epoll_create1(EPOLL_CLOEXEC);
	errorif(epfd < 0, "epoll_create1");
	ev.data.fd = listenfd;
	ev.events = EPOLLIN | EPOLLERR;

	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
	errorif(ret < 0, "epoll_ctl");

	while (1) {
		int n = epoll_wait(epfd, triggered_event, MAX_EVENT, -1);
		errorif(ret < 0, "epoll_wait");
		for (int i = 0; i < n; ++i) {
			if ((EPOLLIN | EPOLLERR) & triggered_event[i].events) {
				triggered_event[i].data.fd
			}
			else if (EPOLLOUT & triggered_event[i].events) {

			}
			else {

			}
		}
	}

	close(listenfd);
	close(epfd);
	return 0;
}