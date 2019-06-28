#ifndef __IOCP_SERVER_H__
#define __IOCP_SERVER_H__

#include <WinSock2.h>
#include <list>
#include <vector>

constexpr int IO_BUF_SIZE = 8192;

enum PostType
{
    ACCEPT_EVENT,
    RECV_EVENT,
    SEND_EVENT
};

struct IoContext
{
    OVERLAPPED      m_overlapped;			//每一个重叠io操作都要有一个OVERLAPPED结构
    WSABUF          m_wsaBuf;				//重叠io需要的buf
	char            m_ioBuf[IO_BUF_SIZE];
    PostType        m_postType;
	SOCKET			m_socket;				//当前进行IO操作的socket，postAccept后再补充一个socket，给后面连接的客户端

	IoContext(PostType type) :
       m_postType(type)
    {
        SecureZeroMemory(this, sizeof(IoContext));
        m_wsaBuf.buf = m_ioBuf;
        m_wsaBuf.len = IO_BUF_SIZE;
    }
};

//一个socket有多个重叠操作
struct ClientContext
{
    SOCKET					m_socket;
    SOCKADDR_IN				m_addr;

	string					m_inBuf;
	string					m_outBuf;

	std::list<IoContext*>	m_ioCtxs;

	IoContext* createIoContext(PostType type)
	{
		IoContext* ioCtx = new IoContext(type);
		m_ioCtxs.emplace_back(ioCtx);
		return ioCtx;
	}

	void removeIoContext(IoContext* pIoCtx)
	{
		m_ioCtxs.remove(pIoCtx);
	}

};



class IocpServer
{
public:

    IocpServer(short listenPort);
    IocpServer(const IocpServer&) = delete;
    IocpServer& operator=(const IocpServer&) = delete;
    ~IocpServer();

    bool init();
    bool start();
    bool stop();



protected:
	//必须要static _beginthreadex才能访问
	static unsigned WINAPI IocpWorkerThread(LPVOID arg);

	bool createListenClient(short listenPort);
	bool createIocpWorker();

	bool postAccept(IoContext* pIoCtx);
    bool postRecv(IoContext* pIoCtx);
    bool postSend(IoContext* pIoCtx);

    bool handleAccept(ClientContext* pListenClient, IoContext* pIoCtx);
    bool handleRecv(ClientContext* pConnClient, IoContext* pIoCtx);
    bool handleSend(ClientContext* pConnClient, IoContext* pIoCtx);


private:
	short						m_listenPort;
	HANDLE						m_comPort;			//完成端口
	ClientContext*				m_pListenClient;
	std::list<ClientContext*>	m_ConnClients;

	void*						m_lpfnAcceptEx;		//acceptEx函数指针
    void*                       m_lpfnGetAcceptExAddr;//GetAcceptExSockaddrs函数指针

	int							m_nWorkerCnt;
};











#endif // !__IOCP_SERVER_H__

