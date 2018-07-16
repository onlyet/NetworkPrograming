#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

//主状态机的两种状态：
enum CHECK_STATE {
	CHECK_STATE_REQUESTLINE = 0,	//当前正在分析请求行
	CHECK_STATE_HEADER				//当前正在分析头部字段
};

enum LINE_STATUS {
	LINE_OK = 0,					//读取到一个完整的行
	LINE_BAD,						//行出错
	LINE_OPEN						//行数据尚且不完整
};

enum HTTP_CODE {
	NO_REQUEST,						//请求不完整，还需继续读取
	GET_REQUEST,					//获得一个完整的客户请求
	BAD_REQUEST,					//请求语法有错
	FORBIDDEN_REQUEST,				//客户没有权限访问资源
	INTERNAL_ERROR,					//服务器内部出错
	CLOSED_REQUEST					//客户已经关闭连接了
};

static const char *szret[] = { "I get a correct result\n", "Something wrong\n" };

LINE_STATUS parse_line(char* buffer, int& checked_index, int& read_index)
{
	char temp;
	for (; checked_index < read_index; ++checked_index) {
		temp = buffer[checked_index];
		if (temp = '\r') {
			//如果字符'\r'碰巧是buffer中最后一个读入的数据，则这次分析没有读取到一个完整的行
			if ((checked_index + 1) == read_index) {
				return LINE_OPEN;
			}
			//成功的读取到一个完整的行
			else if (buffer[checked_index + 1] == '\n') {
				buffer[checked_index++] = '\0';
				buffer[checked_index++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		}
		else if (temp == '\n') {
			if ((checked_index > 1) && buffer[checked_index - 1] == '\r') {
				buffer[checked_index - 1] = '\0';
				buffer[checked_index++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		}
	}
	//所有内容都分析完毕也没遇到'\r'和'\n',则表示还需要继续读取客户数据才能进一步分析
	return LINE_OPEN;
}

HTTP_CODE parse_requestline(char* temp, CHECK_STATE& checkstate)
{
	//返回" \t"中的字符第一次出现的位置
	char *url = strpbrk(temp, " \t");
	//如果请求行中没有空白字符或'\t'字符，则HTTP请求有问题
	if (!url) {
		return BAD_REQUEST;
	}
	*url++ = '\0';

	char *method = temp;
	//判断两字符串是否相同，忽略大小写
	if (strcasecmp(method, "GET") == 0) {
		printf("The request method is GET\n");
	}
	else {
		return BAD_REQUEST;
	}

	//返回url匹配" \t"中的字符长度，直到不匹配为止
	//也就是如果第一个字符不匹配，返回0，不管后面的字符是否匹配
	url += strspn(url, " \t");
	char *version = strpbrk(url, " \t");
	if (!version) {
		return BAD_REQUEST;
	}
	*version++ = '\0';
	version += strspn(version, " \t");
	if (strcasecmp(version, "HTTP/1.1") != 0) {
		return BAD_REQUEST;
	}
	if (strncasecmp(url, "http://", 7) == 0) {
		url += 7;
		//返回字符‘/’第一次出现的位置
		url = strchr(url, '/');
	}
	if (!url || url[0] != '/') {
		return BAD_REQUEST;
	}
	printf("The request URL is: %s\n", url);
	checkstate = CHECK_STATE_HEADER;
	return NO_REQUEST;
}

HTTP_CODE parse_headers(char *temp)
{
	if (temp[0] == '\0') {
		return GET_REQUEST;
	}
	else if (strncasecmp(temp, "Host:", 5) == 0) {
		temp += 5;
		temp += strspn(temp, " \t");
		printf("the request host is: %s\n", temp);
	}
	else {
		printf("I can not handle this header\n");
	}
	return NO_REQUEST;
}

HTTP_CODE parse_content(char* buffer, int& checked_index,
	CHECK_STATE& checkstate, int& read_index, int& start_line)
{
	LINE_STATUS linestatus = LINE_OK;
	HTTP_CODE retcode = NO_REQUEST;
	while ((linestatus == parse_line(buffer, checked_index, read_index)) == LINE_OK) {
		char *temp = buffer + start_line;
		start_line = checked_index;
		switch (checkstate)
		{
		case CHECK_STATE_REQUESTLINE:
		{
			retcode = parse_requestline(temp, checkstate);
			if (retcode == BAD_REQUEST) {
				return BAD_REQUEST;
			}
			break;
		}
		case CHECK_STATE_HEADER:
		{
			retcode = parse_headers(temp);
			if (retcode == BAD_REQUEST) {
				return BAD_REQUEST;
			}
			else if (retcode == GET_REQUEST) {
				return GET_REQUEST;
			}
			break;
		}
		default:
		{
			return INTERNAL_ERROR;
		}
		}
	}
	if (linestatus == LINE_OPEN) {
		return NO_REQUEST;
	}
	else {
		return BAD_REQUEST;
	}
}

int main(int argc, char *argv[])
{
	if (argc <= 2) {
		printf("usage: %s ip_address port_number\n", argv[0]);
		return 1;
	}
	const char *ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);
	int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);
	ret = listen(listenfd, 5);
	assert(ret != -1);
	struct sockaddr_in client_address;
	socklen_t client_addrlength = sizeof(client_address);
	int fd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
	if (fd < 0) {
		printf("errno is: %d\n", errno);
	}
	else {
		char buffer[BUFFER_SIZE];
		memset(buffer, '\0', BUFFER_SIZE);
		int data_read = 0;
		int read_index = 0;
		int checked_index = 0;
		int start_line = 0;
		CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
		while (1) {
			data_read = recv(fd, buffer + read_index, BUFFER_SIZE - read_index, 0);
			if (data_read == -1) {
				printf("reading failed\n");
				break;
			}
			else if (data_read == 0) {
				printf("remote client has closed the connection\n");
				break;
			}
			read_index += data_read;
			HTTP_CODE result = parse_content(buffer, checked_index, checkstate, read_index, start_line);
			if (result == NO_REQUEST) {
				continue;
			}
			else if (result == GET_REQUEST) {
				send(fd, szret[0], strlen(szret[0]), 0);
				break;
			}
			else {
				send(fd, szret[1], strlen(szret[1]), 0);
				break;
			}
		}
		close(fd);
	}
	close(listenfd);
	return 0;
}












