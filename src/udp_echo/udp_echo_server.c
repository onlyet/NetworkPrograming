#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
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
	char recv_buf[MAX_BUF_SIZE];
	struct sockaddr_in srv_addr, cln_addr;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		error_handling("socket error\n");

	memset(&srv_addr, 0, sizeof(srv_addr));

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	srv_addr.sin_port = htons(atoi(argv[1]));

	if (bind(fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1)
		error_handling("bind error\n");

	int cln_addr_len = sizeof(cln_addr);

	while (1)
	{
		//memset(&cln_addr, 0, cln_addr_len);
		if ((ret = recvfrom(fd, recv_buf, MAX_BUF_SIZE, 0, (struct sockaddr*)&cln_addr, &cln_addr_len)) == -1)
			error_handling("recvfrom error\n");

		if ((ret = sendto(fd, recv_buf, ret, 0, (struct sockaddr*)&cln_addr, cln_addr_len)) == -1)
			error_handling("sendto error\n");
	}

	close(fd);

	return 0;
}
