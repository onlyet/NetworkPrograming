#include "pch.h"
#include "DataPacket.h"
#include "Net.h"
#include "ClientContext.h"

void DataPacket::append(ClientContext* pConClient, const char* inBuf, size_t len)
{
    pConClient->appendToBuffer(inBuf, len);
}

//bool DataPacket::parse(const std::string& data)
//{
//    return false;
//}

bool DataPacket::parse(ClientContext* pConnClient)
{
    EnterCriticalSection(&pConnClient->m_csInBuf);

    LeaveCriticalSection(&pConnClient->m_csInBuf);

    return true;
}
