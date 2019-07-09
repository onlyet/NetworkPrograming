#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <queue>
#include "LockGuard.h"

template<typename T>
class Mempool
{
public:
    Mempool() : m_nAllocSize(50)
    {
        InitializeCriticalSection(&m_csLock);
        alloc();
    }

    ~Mempool()
    {
        DeleteCriticalSection(&m_csLock);
        free();
    }

    void push(T* obj)
    {
        LockGuard lk(&m_csLock);
        m_mempool.push(obj);
    }

    T* pop()
    {
        LockGuard lk(&m_csLock);
        if (m_mempool.empty())
        {
            alloc();
        }
        T* pObj = m_mempool.front();
        m_mempool.pop();
        return pObj;
    }

    void setAllocSize(int allocSize)
    {
        LockGuard lk(&m_csLock);
        m_nAllocSize = setAllocSize;
    }

private:
    void alloc()
    {
        for (int i = 0; i < m_nAllocSize; ++i)
        {
            T* pObj = new T();
            m_mempool.push(pObj);
        }
    }
    void free()
    {
        while (!m_mempool.empty())
        {
            T* pObj = m_mempool.front();
            delete pObj;
            m_mempool.pop();
        }
    }

private:
    int                 m_nAllocSize;
    CRITICAL_SECTION    m_csLock;
    std::queue<T*>      m_mempool;
};

#endif // !__MEMPOOL_H__
