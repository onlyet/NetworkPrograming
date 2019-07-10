#include "pch.h"
#include "Buffer.h"
#include <math.h>

Buffer::Buffer()
    : m_pBegin(nullptr)
    , m_pEnd(nullptr)
    , m_nSize(0)
{
}

Buffer::~Buffer()
{
    if (m_pBegin)
        VirtualFree(m_pBegin, 0, MEM_RELEASE);
}

void Buffer::clear()
{
    m_pEnd = m_pBegin;
    deallocateBuffer(1024);
}

UINT Buffer::remove(UINT nSize)
{
    if (nSize > getMemSize())
        return 0;

    if (nSize > getBufferLen())
        nSize = getBufferLen();

    if (nSize)
    {
        MoveMemory(m_pBegin, m_pBegin + nSize, getMemSize() - nSize);
        m_pEnd -= nSize;
    }
    deallocateBuffer(getBufferLen());
    return nSize;
}

UINT Buffer::read(PBYTE pData, UINT nSize)
{
    if (nSize > getMemSize())
        return 0;

    if (nSize > getBufferLen())
        nSize = getBufferLen();

    if (nSize)
    {
        CopyMemory(pData, m_pBegin, nSize);
        MoveMemory(m_pBegin, m_pBegin + nSize, getMemSize() - nSize);
        m_pEnd -= nSize;
    }
    deallocateBuffer(getBufferLen());
    return nSize;
}

BOOL Buffer::write(PBYTE pData, UINT nSize)
{
    reallocateBuffer(nSize + getBufferLen());
    CopyMemory(m_pEnd, pData, nSize);

    m_pEnd += nSize;

    return nSize;
}

BOOL Buffer::write(PCHAR pData, UINT nSize)
{
    return write((PBYTE)pData, nSize);
}

BOOL Buffer::write(const std::string& s)
{
    int nSize = s.size();
    return write((PBYTE)s.c_str(), nSize);
}

int Buffer::scan(PBYTE pScan, UINT nPos)
{
    if (nPos > getBufferLen())
        return -1;

    PBYTE pStr = (PBYTE)strstr((char*)(m_pBegin + nPos), (char*)pScan);
    int nOffset = 0;

    if (pStr)
        nOffset = (pStr - m_pBegin) + strlen((char*)pScan);

    return nOffset;
}

BOOL Buffer::insert(PBYTE pData, UINT nSize)
{
    reallocateBuffer(nSize + getBufferLen());

    MoveMemory(m_pBegin + nSize, m_pBegin, getMemSize() - nSize);
    CopyMemory(m_pBegin, pData, nSize);

    m_pEnd += nSize;

    return nSize;
}

BOOL Buffer::insert(const std::string& s)
{
    int nSize = s.size();
    return insert((PBYTE)s.c_str(), nSize);
}

void Buffer::copy(Buffer& buf)
{
    int nReSize = buf.getMemSize();
    int nSize = buf.getBufferLen();
    clear();
    reallocateBuffer(nReSize);

    m_pEnd = m_pBegin + nSize;

    CopyMemory(m_pBegin, buf.getBuffer(), buf.getBufferLen());
}

PBYTE Buffer::getBuffer(UINT nPos)
{
    return m_pBegin + nPos;
}

void Buffer::writeFile(const std::string& fileName)
{
}

UINT Buffer::getBufferLen()
{
    if (nullptr == m_pBegin)
        return 0;

    int nSize = m_pEnd - m_pBegin;
    return nSize;
}

UINT Buffer::reallocateBuffer(UINT nSize)
{
    if (nSize < getMemSize())
        return 0;

    //ÄÚ´æ¶ÔÆë
    UINT nNewSize = (UINT)ceil(nSize / 1024.0) * 1024;
    PBYTE pNewBuffer = (PBYTE)VirtualAlloc(NULL, nNewSize, MEM_COMMIT, PAGE_READWRITE);

    UINT nBufferLen = getBufferLen();
    CopyMemory(pNewBuffer, m_pBegin, nBufferLen);

    if (m_pBegin)
        VirtualFree(m_pBegin, 0, MEM_RELEASE);

    m_pBegin = pNewBuffer;
    m_pEnd = m_pBegin + nBufferLen;
    m_nSize = nNewSize;
    return m_nSize;
}

UINT Buffer::deallocateBuffer(UINT nSize)
{
    if (nSize < getBufferLen())
        return 0;

    UINT nNewSize = (UINT)ceil(nSize / 1024.0) * 1024;
    if (nNewSize < getMemSize())
        return 0;

    PBYTE pNewBuffer = (PBYTE)VirtualAlloc(NULL, nNewSize, MEM_COMMIT, PAGE_READWRITE);

    UINT nBufferLen = getBufferLen();
    CopyMemory(pNewBuffer, m_pBegin, nBufferLen);

    if (m_pBegin)
        VirtualFree(m_pBegin, 0, MEM_RELEASE);

    m_pBegin = pNewBuffer;
    m_pEnd = m_pBegin + nBufferLen;
    m_nSize = nNewSize;
    return m_nSize;
}

UINT Buffer::getMemSize()
{
    return m_nSize;
}
