#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_SIZE 1024

void error_handling(char *message)
{
	fputs(message, stderr);
	fputs('\n', stderr);
	exit(1);
}

int main(int argc, char *argv[])
{
	char recv_buf[MAX_SIZE];
	int str_len;
	int recv_sock;
	struct sockaddr_in bind_addr;
	struct ip_mreq multicast_addr;

	if (argc != 3) {
		printf("usage: ./PROCESS <IP> <PORT>\n");
		exit(1);
	}

	if ((recv_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		error_handling("socket() error");

	bzero(&bind_addr, sizeof(bind_addr));
	bind_addr.sin_family = AF_INET;
	//bind_addr.sin_addr.s_addr = inet_addr(INADDR_ANY);
	bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_addr.sin_port = htons(atoi(argv[2]));

	if (bind(recv_sock, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) == -1)
		error_handling("bind() error");

	multicast_addr.imr_multiaddr.s_addr = inet_addr(argv[1]);
	//multicast_addr.imr_interface.s_addr = inet_addr(INADDR_ANY);
	multicast_addr.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_addr, sizeof(multicast_addr)) == -1)
		error_handling("setsockopt() error");

	if ((str_len = recvfrom(recv_sock, recv_buf, MAX_SIZE, 0, NULL, NULL)) == -1)
		error_handling("recvfrom() error");
	recv_buf[str_len] = '\0';
	fflush(stdout);

	while (1) {
		fputs(recv_buf, stdout);
		fflush(stdout);
		if ((str_len = recvfrom(recv_sock, recv_buf, MAX_SIZE, 0, NULL, NULL)) == -1)
			break;
		recv_buf[str_len] = '\0';
	}

	close(recv_sock);
	return 0;
}