#ifndef __DATA_PACKET_H__
#define __DATA_PACKET_H__

#include "Buffer.h"

#include <string>

struct DataPacket
{
    static bool parse(Buffer data);
};

#endif // !__DATA_PACKET_H__
