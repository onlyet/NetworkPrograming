#include "pch.h"
#include "Codec.h"

#include <iostream>
//#include <string>
#include <sstream>
#include <string.h>

using namespace std;

//int LineCodec::tryDecode(Slice data, Slice & msg)
//{
//    if (data.size() == 1 && data[0] == 0x04)
//    {
//        msg = data;
//        return 1;
//    }
//    for (size_t i = 0; i < data.size(); i++)
//    {
//        if (data[i] == '\n')
//        {
//            if (i > 0 && data[i - 1] == '\r')
//            {
//                msg = Slice(data.data(), i - 1);
//                return i + 1;
//            }
//            else
//            {
//                msg = Slice(data.data(), i);
//                return i + 1;
//            }
//        }
//    }
//    return 0;
//}

HttpCodec::HttpState HttpCodec::getLine(Slice data, Slice& line)
{
    m_state = HTTP_BAD_REQUEST;

    if (data.size() < 2)
        return m_state;

    for (int i = 0; i < data.size() - 2; ++i)
    {
        if ('\r' == data[i] && 0 == memcmp("\r\n", data.begin() + i, 2))
        {
            line = Slice(data.begin(), i);
            m_state = HTTP_OK;
        }
    }
    return m_state;
}

HttpCodec::HttpState HttpCodec::decodeStartLine(Slice& line)
{
    m_state = HTTP_BAD_REQUEST;
    try
    {

        Slice method = line.eatWord();
        if (method.empty())
        {
            cout << "invalid http method" << endl;
            return m_state;
        }
        Slice url = line.eatWord();
        if (url.empty())
        {
            cout << "invalid http url" << endl;
            return m_state;
        }
        Slice version = line.eatWord();
        if (version.empty())
        {
            cout << "invalid http version" << endl;
            return m_state;
        }
        //m_http.insert(string("method"), string(method.begin(), method.end()));
        m_http["method"] = method;
        m_http["url"] = url;
        m_http["version"] = version;

        if (Slice("HTTP/1.0") != version && Slice("HTTP/1.1") != version)
        {
            cout << "invalid http version" << endl;
            return m_state;
        }

        if (Slice("GET") != method || Slice("POST") != method)
        {
            cout << "invalid http method" << endl;
            return m_state;
        }
        //strcasecmp

    }
    catch (std::exception& e)
    {
        cout << "exception: " << e.what() << endl;
    }
    return m_state;
}

HttpCodec::HttpState HttpCodec::getHeader(Slice data, Slice& header)
{
    m_state = HTTP_BAD_REQUEST;

    if (data.size() < 4)
        return m_state;

    for (int i = 0; i < data.size() - 4; ++i)
    {
        //ÅÐ¶ÏÊÇ·ñ´æÔÚ¿ÕÐÐ
        if ('\r' == data[i] && 0 == memcmp("\r\n\r\n", data.begin() + i, 4))
        {
           header = Slice(data.begin(), i);
           m_state = HTTP_OK;
        }
    }
    return m_state;
}

HttpCodec::HttpState HttpCodec::decodeHeader(Slice header, Slice& line)
{
    HttpCodec::HttpState ret;
    Slice startLine;
    ret = getLine(header, startLine);

    ret = decodeStartLine(startLine);


    return HttpState();
}



