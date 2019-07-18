#include "HttpCodec.h"

using namespace std;

void HttpCodec::tryDecode(std::string msg)
{
    size_t sz = msg.size();
    if (sz < 4)
        return;

    for (int i = 0; i < msg.size(); ++i)
    {
        char c = msg.at(i);
        if ('\r' == c && memcmp("\r\n\r\n", msg.data() + i, 4))
        {
            string header;
            header.append(msg.data(), i + 1);
        }
    }
}

HttpCodec::HttpState HttpCodec::getLine(std::string data, std::string& line)
{
    return HttpState();
}
