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

void HttpCodec::tryDecode(Slice msg)
{
    Slice data; // Slice data(m_inBuf.data(), m_inBuf.len());
    Slice header;
    getHeader(data, header);
    if (header.empty())
    {
        m_state = Http_Header_Incomplete;
        return;
    }

    Slice startLine = header.eatLine();
    //decodeStartLine(startLine);
    Slice method = startLine.eatWord();
    Slice url = startLine.eatWord();
    Slice version = startLine.eatWord();
    if (version.empty())
    {
        m_state = Http_Header_Incomplete;
        return;
    }

    m_header.insert(make_pair("method", method));
    m_header.insert(make_pair("url", url));
    m_header.insert(make_pair("version", version));

    while (!header.empty())
    {
        //É¾³ý\r\n
        header.eat(2);
        Slice line = header.eatLine();
        Slice key = line.eatWord();
        Slice value = line.eatWord();

        if(!key.empty() && !line.empty() && key.back() == ':')
        {
            //É¾³ý:
            m_header.insert(make_pair(key.sub(0, -1), value));
        }
        else if (line.empty())
        {
            //Ö»ÓÐstart-line£¿
        }
        else
        {
            m_state = Http_Header_Incomplete;
            return;
        }
    }

    //data.eat(m_nHeaderLength);
    string contentLength = getHeaderField("content-length");
    if (contentLength.empty())
    {
        m_state = Http_Header_Incomplete;
        return;
    }

    if (data.size() < m_nHeaderLength + atoi(contentLength.c_str()))
    {
        m_state = Http_Body_Incomplete;
        return;
    }
    Slice body(data.data() + m_nHeaderLength, atoi(contentLength.c_str()));

}

bool HttpCodec::getLine(Slice data, Slice& line)
{
    size_t sz = data.size();
    if (sz < 2)
        return false;

    const char* pb = data.begin();
    for (size_t i = 0; i <= sz - 2; ++i)
    {
        if ('\r' == data[i] && 0 == memcmp("\r\n", pb + i, 2))
        {
            line = Slice(pb, i);
            return true;
        }
    }
    return false;
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
        //m_header.insert(string("method"), string(method.begin(), method.end()));
        m_header["method"] = method.toString();
        m_header["url"] = url.toString();
        m_header["version"] = version.toString();

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

bool HttpCodec::getHeader(Slice data, Slice& header)
{
    size_t sz = data.size();
    if (sz < 4)
        return false;

    for (size_t i = 0; i <= sz - 4; ++i)
    {
        const char* pb = data.data();
        if ('\r' == data[i] && memcmp("\r\n\r\n", pb + i, 4))
        {
            header = Slice(pb, i);
            m_nHeaderLength = i + 4;
            return true;
        }
    }
    return false;
}

HttpCodec::HttpState HttpCodec::parseHeader()
{
    const string& method = m_header["method"];
    if ("GET" != method || "POST" != method)
    {
        informUnimplemented();
        return m_state;
    }
    const string& version = m_header["version"];
    if ("HTTP/1.1" != version)
    {
        informUnsupported();
        return m_state;
    }


}

bool HttpCodec::informUnimplemented()
{
    return false;
}

bool HttpCodec::informUnsupported()
{
    // 505 (HTTP Version Not Supported)
    return false;
}

std::string HttpCodec::getHeaderField(const std::string& strKey)
{
    auto it = m_header.find(strKey);
    return it != m_header.end() ? m_header[strKey] : "";
}



