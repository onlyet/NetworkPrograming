#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PATH_SIZE 512
#define MAX_SIZE 1024

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void *get_handler(void *fp)
{
	char path[PATH_SIZE];
	char request_line[MAX_SIZE];
	FILE *fp_write = (FILE*)fp;

	printf("enter request path:\n");
	//fgets会读取到换行符
	fgets(path, PATH_SIZE, stdin);
	//替换path的换行符为\0
	path[strlen(path) - 1] = '\0';
	printf("path: %s\n", path);

	sprintf(request_line, "GET %s HTTP/1.1\r\n", path);
	printf("request_line: %s\n", request_line);

	fputs(request_line, fp_write);
	
	fclose(fp);
	return NULL;
}

void *read_handler(void *fp)
{
	char read_buf[MAX_SIZE];
	FILE *fp_read = (FILE*)fp;

	//while (!feof(fp_read)) {
	//	fgets(read_buf, MAX_SIZE, fp_read);
	//	fputs(read_buf, stdout);
	//	fflush(stdout);
	//}
	fgets(read_buf, MAX_SIZE, fp_read);
	while (!feof(fp_read)) {
		fputs(read_buf, stdout);
		fgets(read_buf, MAX_SIZE, fp_read);
	}
	/*while (fgets(read_buf, MAX_SIZE, fp_read) != NULL) {
		fputs(read_buf, stdout);
		fflush(stdout);
	}*/
	//printf("read_buf: %s", read_buf);

	fclose(fp);
	return NULL;
}

int main(int argc, char *argv[])
{
	int clnt_sock;
	struct sockaddr_in serv_addr;
	pthread_t w_tid, r_tid;

	if ((clnt_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		error_handling("socket error");

	if (argc < 3) {
		printf("arguments: <IP, PORT>\n");
		exit(1);
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect error");

	FILE *write_fp = fdopen(dup(clnt_sock), "w");
	FILE *read_fp = fdopen(clnt_sock, "r");

	pthread_create(&w_tid, NULL, get_handler, (void*)write_fp);
	pthread_create(&r_tid, NULL, read_handler, (void*)read_fp);
	pthread_join(w_tid, NULL);
	pthread_join(r_tid, NULL);

	
	return 0;
}