#ifndef __CODEC_H__
#define __CODEC_H__

#include "Slice.h"
//#include "Buffer.h"
#include "HttpMessage.h"
#include <string>
#include <unordered_map>

//struct CodecBase
//{
//    ////CodecBase() {}
//    virtual int tryDecode(Slice data, Slice &msg) = 0;
//};

//struct CodecBase 
//{
//    // > 0 解析出完整消息，消息放在msg中，返回已扫描的字节数
//    // == 0 解析部分消息
//    // < 0 解析错误
//    virtual int tryDecode(Slice data, Slice &msg) = 0;
//    //virtual void encode(Slice msg, Buffer &buf) = 0;
//    //virtual CodecBase *clone() = 0;
//};

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


struct HttpCodec
{
    //enum HttpState
    //{
    //    HTTP_OK,
    //    HTTP_BAD_REQUEST,
    //    Http_Header_Incomplete,
    //    Http_Body_Incomplete,

    //    Http_Invalid_Request_Line,  //400 (Bad Request) error or a 301 (Moved Permanently)
    //    
    //};

    enum statusType
    {
        continue_transfer = 100,
        switching_protocol = 101,
        ok = 200,
        created = 201,
        accepted = 202,
        non_authoritative_information = 203,
        no_content = 204,
        reset_content = 205,
        partial_content = 206,
        multiple_choices = 300,
        moved_permanently = 301,
        found = 302,
        see_other = 303,
        not_modified = 304,
        use_proxy = 305,
        temporary_redirect = 307,
        bad_request = 400,
        unauthorized = 401,
        payment_required = 402,
        forbidden = 403,
        not_found = 404,
        method_not_allowed = 405,
        not_acceptable = 406,
        proxy_authentication_required = 407,
        request_time_out = 408,
        conflict = 409,
        gone = 410,
        precondition_failed = 412,
        request_entity_too_large = 413,
        request_uri_too_large = 414,
        unsupported_media_type = 415,
        requested_range_not_satisfiable = 416,
        expectation_failed = 417,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503,
        gateway_timeout = 504,
        http_version_not_supported = 505,

        server_busy = 600
    };

    HttpCodec(PBYTE pData, UINT size);

    int tryDecode();
    std::string responseMessage() const;

    void writeResponse();

    bool getHeader(Slice data, Slice& header);

    bool parseStartLine();
    bool parseHeader();
    bool parseBody();

    //HttpState handleGet();
    //HttpState handlePost();

    bool informUnimplemented();
    bool informUnsupported();


private:
    //Slice                                           m_requestMessage;
    //std::string                                     m_responseMessage;
    //HttpState                                       m_state;
    //std::unordered_map<std::string, std::string>    m_header;
    //size_t                                          m_nHeaderLength;    //HTTP消息头部长度，headerLen+bodyLen=wholeMsg
    Slice               m_inBuf;
    std::string         m_outBuf;
    HttpRequest         m_req;
    HttpResponse        m_res;
};

#endif // !__CODEC_H__
