#ifndef __NET_H__
#define __NET_H__

//#include "Buffer.h"

//struct ClientContext;

struct Net
{
    static bool init();
    static bool unInit();

    static SOCKET WSASocket_();
    static int bind(SOCKET s, const LPSOCKADDR_IN pAddr);
    static int listen(SOCKET s, int backlog = SOMAXCONN);

    //static int 

    static bool setKeepAlive(SOCKET socket, bool on);
};

#endif // !__NET_H__


