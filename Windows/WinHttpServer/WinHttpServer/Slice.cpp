#include "pch.h"
#include "Slice.h"

Slice::Slice()
    : m_pb("")
{
    m_pe = m_pb;
}

Slice::Slice(const char* b, const char* e)
    : m_pb(b), m_pe(e) {}

Slice::Slice(const char* d, size_t n)
    : m_pb(d), m_pe(d + n) {}

Slice::Slice(const std::string& s)
    : m_pb(s.data()), m_pe(s.data() + s.size()) {}

Slice::Slice(const char * s)
    : m_pb(s), m_pe(s + strlen(s)) {}

const char* Slice::data() const
{
    return m_pb;
}

const char* Slice::begin() const
{
    return m_pb;
}

const char* Slice::end() const
{
    return m_pe;
}

char Slice::front()
{
    return *m_pb;
}

char Slice::back()
{
    return m_pe[-1];
}

size_t Slice::size() const
{
    return m_pe - m_pb;
}

bool Slice::empty() const
{
    return m_pe == m_pb;
}

void Slice::resize(size_t sz)
{
    m_pe = m_pb + sz;
}


void Slice::clear()
{
    m_pe = m_pb = "";
}

//bool Slice::operator<(const Slice& rhs)
//{
//    return compare(rhs) < 0;
//}
//
//bool Slice::operator==(const Slice& rhs)
//{
//        return ((size() == rhs.size()) && (memcmp(data(), rhs.data(), size()) == 0));
//}
//
//bool Slice::operator!=(const Slice& rhs)
//{
//    return *this == rhs;
//}

Slice Slice::eatWord()
{
    const char *b = m_pb;
    //跳过空白字符
    while (b < m_pe && isspace(*b)) 
    {
        ++b;
    }
    const char *e = b;
    //直到空格或\r
    while (e < m_pe && !isspace(*e)) 
    {
        ++e;
    }
    m_pb = e;
    return Slice(b, e);
}

Slice Slice::eatLine()
{
    const char* p = m_pb;
    while (m_pb < m_pe && *m_pb != '\r' && *m_pb != '\n') 
    {
        m_pb++;
    }
    return Slice(p, m_pb - p);
}

Slice Slice::eat(size_t sz)
{
    if (sz > size())
        sz = size();

    Slice s(m_pb, sz);
    m_pb += sz;
    return s;
}

Slice Slice::sub(int boff, int eoff) const
{
    Slice s(*this);
    s.m_pb += boff;
    s.m_pe += eoff;
    return s;
}

Slice & Slice::trimSpace()
{
    while (m_pb < m_pe && isspace(*m_pb))
        m_pb++;

    while (m_pb < m_pe && isspace(m_pe[-1]))
        m_pe--;

    return *this;
}

char Slice::operator[](size_t n) const
{
    return m_pb[n];
}

std::string Slice::toString() const
{
    return std::string(m_pb, m_pe);
}

Slice::operator std::string() const
{
    return std::string(m_pb, m_pe);
}

int Slice::compare(const Slice& b) const
{
    size_t sz = size(), bsz = b.size();
    const int min_len = (sz < bsz) ? sz : bsz;
    int r = memcmp(m_pb, b.m_pb, min_len);
    if (r == 0) 
    {
        if (sz < bsz) r = -1;
        else if (sz > bsz) r = +1;
    }
    return r;
}

bool Slice::starts_with(const Slice& x) const
{
    return (size() >= x.size() && memcmp(m_pb, x.m_pb, x.size()) == 0);
}

bool Slice::end_with(const Slice& x) const
{
    return (size() >= x.size() && memcmp(m_pe - x.size(), x.m_pb, x.size()) == 0);
}

std::vector<Slice> Slice::split(char ch) const
{
    std::vector<Slice> r;
    const char* pb = m_pb;
    for (const char* p = m_pb; p < m_pe; p++) 
    {
        if (*p == ch)
        {
            r.push_back(Slice(pb, p));
            pb = p + 1;
        }
    }
    if (m_pe != m_pb)
        r.push_back(Slice(pb, m_pe));

    return r;
}

bool operator<(const Slice& x, const Slice& y)
{
    return x.compare(y) < 0;
}

bool operator==(const Slice& x, const Slice& y)
{
    return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
}

bool operator!=(const Slice& x, const Slice& y)
{
    return !(x == y);
}
