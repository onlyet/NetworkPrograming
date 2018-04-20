#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define MAX_BUF_SIZE 1024

void error_handling(const char *err)
{
	fputs(err, stdout);
	fputc('\n', stdout);
	exit(1);
}

int main(int argc, char *argv[])
{
	int fd;
	ssize_t ret;
	struct sockaddr_in srv_addr, cln_addr, from_addr;
	char buf[MAX_BUF_SIZE];

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		error_handling("socket error\n");

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	srv_addr.sin_port = htons(atoi(argv[2]));

	socklen_t srv_addr_len = sizeof(srv_addr);
	int addr_sz = sizeof(from_addr);

	while (1)
	{
		memset(&from_addr, 0, addr_sz);

		printf("输入发送内容\n");
		printf("输入'q'或'Q'退出\n");
		/*fgets(buf, MAX_BUF_SIZE, stdin);
		if (!strcmp("q\n", buf) || !strcmp("Q\n", buf))
			break;*/
		strcpy(buf, "hello");

		//scanf("%s", buf);
		if ((ret = sendto(fd, buf, strlen(buf), 0, (struct sockaddr*)&srv_addr, srv_addr_len)) == -1)
			error_handling("sendto error\n");
		if ((ret = recvfrom(fd, buf, ret, 0, (struct sockaddr*)&from_addr, &addr_sz)) == -1)
			error_handling("recvfrom error\n");

		buf[ret] = '\0';

		printf("message from server: %s\n", buf);
	}

	close(fd);

	return 0;
} 
