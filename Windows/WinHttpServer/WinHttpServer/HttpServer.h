#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include "IocpServer.h"

class HttpServer : public IocpServer
{
public:
    HttpServer(short listenPort, int maxConnectionCount = 10000);
    ~HttpServer() {}

protected:
    void notifyPackageReceived(ClientContext* pConnClient) override;
    void notifyDisconnected(SOCKET s, SOCKADDR_IN addr) override;


};

#endif // !__HTTP_SERVER_H__
