#ifndef __LOCK_GUARD_H__
#define __LOCK_GUARD_H__

class LockGuard
{
public:
    LockGuard(LPCRITICAL_SECTION lock)
        : m_lock(lock)
    {
        EnterCriticalSection(m_lock);
    }

    ~LockGuard()
    {
        LeaveCriticalSection(m_lock);
        m_lock = nullptr;
    }

private:
    LPCRITICAL_SECTION    m_lock;
};

#endif // !__LOCK_GUARD_H__
