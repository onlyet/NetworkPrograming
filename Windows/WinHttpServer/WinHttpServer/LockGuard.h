#ifndef __LOCK_GUARD_H__
#define __LOCK_GUARD_H__

//CRITICAL_SECTION是可重入的，只要leave和enter的次数一致就行
//std::mutex是不能重入的，std::recursive_mutex是可重入的
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
