#ifndef __DATA_PACKET_H__
#define __DATA_PACKET_H__

#include <string>

struct ClientContext;

struct DataPacket
{
    static void append(ClientContext* pConClient, const char* inBuf, size_t len);
    //static bool parse(const std::string& data);
    static bool parse(ClientContext* pConnClient);
};

#endif // !__DATA_PACKET_H__
