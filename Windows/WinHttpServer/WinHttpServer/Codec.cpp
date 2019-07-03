#include "pch.h"
#include "Codec.h"

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

HttpCodec::HttpState HttpCodec::getHeader(Slice data, Slice& header)
{
    if (data.size() < 4)
        return HTTP_HEADER_INCORRECT;

    for (int i = 0; i < data.size() - 4; ++i)
    {
        //ÅÐ¶ÏÊÇ·ñ´æÔÚ¿ÕÐÐ
        if ('\r' == data[i] && 0 == memcmp("\r\n\r\n", data.begin(), 4))
        {
           header = Slice(data.begin(), i + 4);
        }
        else
        {
            m_state = HTTP_HEADER_INCORRECT;
            return m_state;
        }
    }

    return HTTP_HEADER_INCORRECT;
}

HttpCodec::HttpState HttpCodec::decodeHeader(Slice header)
{


    return HttpState();
}

