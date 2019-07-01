#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <string>

class Buffer
{
public:
    Buffer()
    {
        InitializeCriticalSection(&m_cs);
    }

    ~Buffer()
    {
        DeleteCriticalSection(&m_cs);
    }

    void append(const char* buf, const size_t len)
    {
        EnterCriticalSection(&m_cs);
        m_buf.append(buf, len);
        LeaveCriticalSection(&m_cs);
    }

    void append(const std::string& buf)
    {
        EnterCriticalSection(&m_cs);
        m_buf.append(buf);
        LeaveCriticalSection(&m_cs);
    }


private:
    std::string         m_buf;
    CRITICAL_SECTION    m_cs;
};

#endif // !__BUFFER_H__
