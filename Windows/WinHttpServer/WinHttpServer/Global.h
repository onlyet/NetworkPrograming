#ifndef __GLOBAL_H__
#define __GLOBAL_H__


//设置成4K的倍数
constexpr int IO_BUF_SIZE = 8192;
constexpr int ACCEPT_ADDRS_SIZE = (sizeof(SOCKADDR_IN) + 16) * 2;

enum PostType
{
    ACCEPT_EVENT,
    RECV_EVENT,
    SEND_EVENT,
    PARSE_EVNET,    //数据包解析
    CLOSE_EVENT,
};

#endif // !__GLOBAL_H__
