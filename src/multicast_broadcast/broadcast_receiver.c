#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE 30

int make_nonblocking(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) /* Failed? */
		return 0;
	/* Clear the blocking flag. */
	flags &= ~O_NONBLOCK;
	return fcntl(fd, F_SETFL, flags) != -1;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char *argv[])
{
	int recv_sock;
	struct sockaddr_in adr;
	int str_len;
	char buf[BUF_SIZE];
	if (argc != 2) {
		printf("Usage : %s <PORT>\n", argv[0]);
		exit(1);
	}

	recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	memset(&adr, 0, sizeof(adr));
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	adr.sin_port = htons(atoi(argv[1]));

	if (!make_nonblocking(recv_sock)) {
		exit(1);
	}


	if (bind(recv_sock, (struct sockaddr*)&adr, sizeof(adr)) == -1)
		error_handling("hind() error");
	while (1)
	{
		str_len = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0, NULL, 0);
		if (str_len < 0)
			break;
		buf[str_len] = 0;
		fputs(buf, stdout);
	}
	close(recv_sock);
	return 0;
}