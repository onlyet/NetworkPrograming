#ifndef __DATA_PACKET_H__
#define __DATA_PACKET_H__

#include <string>

struct DataPacket
{
    static bool parse(std::string data);
};

#endif // !__DATA_PACKET_H__
