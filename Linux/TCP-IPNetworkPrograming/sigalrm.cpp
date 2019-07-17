#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void sig_handler(int signum)
{
	printf("in handler\n");
	sleep(1);
	printf("handler return\n");
}

int main(int argc, char **argv)
{
	char buf[100];
	int ret;
	struct sigaction action, old_action;

	action.sa_handler = sig_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	/* 版本1:不设置SA_RESTART属性
	* 版本2:设置SA_RESTART属性 */
	action.sa_flags |= SA_RESTART;

	sigaction(SIGALRM, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN) {
		sigaction(SIGALRM, &action, NULL);
	}
	alarm(3);

	bzero(buf, 100);

	ret = read(0, buf, 100);
	if (ret == -1) {
		perror("read error");
	}
	printf("read %d bytes, strerrno: %s\n", ret, strerror(errno));
	printf("%s", buf);

	return 0;
}
