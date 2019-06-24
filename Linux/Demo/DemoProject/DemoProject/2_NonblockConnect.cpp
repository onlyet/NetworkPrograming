#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

int main(int argc, char * *argv)
{
    int             cln_sock = 0;

    cln_sock            = socket(AF_INET, SOCK_STREAM, 0);

    if (-1 == cln_sock)
    {
        printf(" create socket error\n");
        return - 1;
    }

    // 取文件状态标志
    int flags = fcntl(cln_sock, F_GETFL, 0);
    if ( -1 == fcntl(cln_sock, F_SETFL, flags | O_NONBLOCK) )
    {
        printf("fcntl error \n");
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port  = htons(9999);

    int iconnectRet = connect(cln_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    bool bConnect = false;
	struct timeval timeout;

    if (iconnectRet == 0)
    {
        printf("connect directly success\n");
    }
    else
    {
        if(errno == EINPROGRESS)
        {
		    int times = 0;
			//struct timeval timeout;
            while (times++ < 5)
            {
				fd_set readFds, writeFds;
			
				FD_ZERO(&readFds);
				FD_ZERO(&writeFds);
			
				FD_SET(cln_sock, &readFds);
				FD_SET(cln_sock, &writeFds);
				
                timeout.tv_sec = 3;
                timeout.tv_usec = 0;

                int result = select(cln_sock + 1, &readFds, &writeFds, NULL, &timeout);
                if(result == -1)
                {
                    printf("select error\n");
                    break;
                }
                else if(result == 0)
                {
                    printf("select time out\n");
                }
                else
                {
                    if ( FD_ISSET(cln_sock, &readFds) || FD_ISSET(cln_sock, &writeFds) )
                   {
					    connect(cln_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); // ?
						if(EISCONN == errno)
						{
							printf(" connect succeed\n");
							bConnect = true;
							break;
						}	
						else
						{
							printf(" connect fail\n");  // 这里不起效果，因为 linux socket 一直是可写
						}
                   }
                }
            }
        }
    }

    if(bConnect)
    {
        printf("noblock connect succeed\n");
		// to do 
		close(cln_sock);
    }
	else
	{
		close(cln_sock);
	}	


    return 0;
}

