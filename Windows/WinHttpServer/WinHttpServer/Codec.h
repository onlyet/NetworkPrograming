#ifndef __CODEC_H__
#define __CODEC_H__

#include "Slice.h"
//#include "Buffer.h"
#include <string>
#include <unordered_map>

struct CodecBase 
{
    // > 0 解析出完整消息，消息放在msg中，返回已扫描的字节数
    // == 0 解析部分消息
    // < 0 解析错误
    //virtual int tryDecode(Slice data, Slice &msg) = 0;
    //virtual void encode(Slice msg, Buffer &buf) = 0;
    //virtual CodecBase *clone() = 0;
};

////以\r\n结尾的消息
//struct LineCodec : public CodecBase 
//{
//    int tryDecode(Slice data, Slice &msg) override;
//    void encode(Slice msg, Buffer &buf) override;
//    CodecBase *clone() override { return new LineCodec(); }
//};
//
////给出长度的消息
//struct LengthCodec : public CodecBase 
//{
//    int tryDecode(Slice data, Slice &msg) override;
//    void encode(Slice msg, Buffer &buf) override;
//    CodecBase *clone() override { return new LengthCodec(); }
//};


struct HttpCodec : public CodecBase
{
    enum HttpState
    {
        HTTP_OK,
        HTTP_BAD_REQUEST,
        Http_Header_Incomplete,
        Http_Body_Incomplete,

        Http_Invalid_Request_Line,  //400 (Bad Request) error or a 301 (Moved Permanently)
        
    };

    void tryDecode(Slice msg);

    bool getHeader(Slice data, Slice& header);

    bool getLine(Slice data, Slice& line);

    HttpState decodeStartLine(Slice& line);
    HttpState parseHeader();
    HttpState getBody();
    HttpState decodeBody();

    HttpState handleGet();
    HttpState handlePost();
    HttpState handleUrl();

    bool informUnimplemented();
    bool informUnsupported();

    std::string getHeaderField(const std::string& strKey);

private:
    HttpState                                       m_state;
    std::unordered_map<std::string, std::string>    m_header;
    size_t                                          m_nHeaderLength;    //HTTP消息头部长度，headerLen+bodyLen=wholeMsg
};

#include <map>
void test()
{
    string http;

    //存储请求行
    string cmdLine = "";
    //存储消息头各字段和值
    map<string, string> kvs;

    size_t pos = 0, posk = 0, posv = 0;
    string k = "", v = "";
    char c;
    //http保存了上述GET请求
    while (pos != http.size())
    {
        c = http.at(pos);
        if (c == ':')
        {

            //非请求行，且消息头名称未解析
            if (!cmdLine.empty() && k.empty())
            {

                //存储消息头名称
                k = http.substr(posk, pos - posk);

                //跳过冒号和空格
                posv = pos + 2;
            }
        }

        //行尾
        else if (c == '\r' || c == '\n')
        {

            //尚未解析到消息头字段名称，且请求行也未解析过
            if (k.empty() && cmdLine.empty())
            {
                //本行应是请求行，保存之
                cmdLine = http.substr(posk, pos - posk);
            }
            else
            {
                //已解析了消息头字段名称，尚未解析字段值
                if (!k.empty() && v.empty())
                {

                    //存储字段值
                    v = http.substr(posv, pos - posv);
                }
            }
            posk = pos + 1;
        }

        if (!k.empty() && !v.empty() && !cmdLine.empty())
        {

            //保存消息头字段名称和值
            kvs.insert(make_pair(k, v));
            k = "";
            v = "";
        }
        ++pos;
    }

}

#endif // !__CODEC_H__
