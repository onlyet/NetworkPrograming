#ifndef __HTTP_CODEC_H__
#define __HTTP_CODEC_H__

#include <string>

class HttpCodec
{
    enum HttpState
    {
        HTTP_OK,
        HTTP_BAD_REQUEST,

    };

    HttpState getLine(std::string data, std::string& line);
    HttpState decodeStartLine(std::string& line);
    HttpState getHeader(std::string data, std::string& header);
    HttpState decodeHeader(std::string header, std::string& line);
    HttpState getBody();
    HttpState decodeBody();

    HttpState handleGet();
    HttpState handlePost();
    HttpState handleUrl();



private:
    HttpState                               m_state;
    std::map<std::std::string, std::std::string>      m_http;
};

#endif // !__HTTP_CODEC_H__
#define __HTTP_CODEC_H__

