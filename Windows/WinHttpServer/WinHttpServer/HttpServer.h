#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include "IocpServer.h"

class HttpServer : IocpServer
{
public:
    HttpServer(short listenPort, int maxConnectionCount = 10000);
    HttpServer(const HttpServer&);
    ~HttpServer();

protected:
    void notifyPackageReceived(ClientContext* pConnClient) override;
};

#endif // !__HTTP_SERVER_H__
