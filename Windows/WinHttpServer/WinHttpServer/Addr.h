#ifndef __ADDR_H__
#define __ADDR_H__

#include <string>

class Addr
{
public:
    Addr() {}
    Addr(const SOCKADDR_IN& addr);

    std::string toString() const;

private:
    SOCKADDR_IN m_addr;
};

#endif // !__ADDR_H__
