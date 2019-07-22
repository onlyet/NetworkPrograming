#include "pch.h"
#include "HttpMessage.h"

std::string HttpMessage::getHeaderField(const std::string& strKey)
{
    auto it = m_headers.find(strKey);
    return it != m_headers.end() ? m_headers[strKey] : "";
}

void HttpMessage::setHeader(std::string key, std::string value)
{
    auto it = m_headers.find(key);
    if (it != m_headers.end())
    {
        m_headers[key] = value;
    }
}
