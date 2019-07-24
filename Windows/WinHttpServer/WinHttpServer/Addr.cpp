#include "pch.h"
#include "Addr.h"
#include <sstream>

using namespace std;

Addr::Addr(const SOCKADDR_IN& addr)
    : m_addr(addr)
{
}

std::string Addr::toString() const
{
    char peerAddrBuf[1024] = { 0 };
    inet_ntop(AF_INET, &m_addr.sin_addr, peerAddrBuf, 1024);
    ostringstream os;
    os << peerAddrBuf << ":" << ntohs(m_addr.sin_port);
    return os.str();
}
