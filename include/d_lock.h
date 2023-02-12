//
// Created by YiMing D on 2023/1/15.
//

#ifndef DREAMER_D_LOCK_H
#define DREAMER_D_LOCK_H
#include "nocopyable.h"
#include "pthread.h"
#include "atomic"
#include "list"
#include "fiber.h"


namespace dreamer {


template<class T>
class MutexLockImpl : NoCopyable {
public:
    MutexLockImpl(T &mutex) : m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }
    ~MutexLockImpl() {
        m_mutex.unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }
    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    bool m_locked;
    T& m_mutex;
};
template<class T>
class ReadLockImpl : NoCopyable {
public:
    ReadLockImpl(T &mutex) : m_mutex(mutex) {
        m_mutex.rlock();
        m_locked = true;
    }
    ~ReadLockImpl() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
    void rlock() {
        if (!m_locked) {
            m_mutex.rlock();
            m_locked = true;
        }
    }
    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    bool m_locked;
    T& m_mutex;
};
template<class T>
class WriteLockImpl : NoCopyable {
public:
    WriteLockImpl(T &mutex) : m_mutex(mutex) {
        m_mutex.wlock();
        m_locked = true;
    }
    ~WriteLockImpl() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
    void wlock() {
        if (!m_locked) {
            m_mutex.wlock();
            m_locked = true;
        }
    }
    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    bool m_locked;
    T& m_mutex;
};

class Mutex : NoCopyable{
public:
    typedef MutexLockImpl<Mutex> MutexLock;
    Mutex() {
        pthread_mutex_init(&m_lock, nullptr);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m_lock);
    }

    void lock() {
        pthread_mutex_lock(&m_lock);
    }
    void unlock() {
        pthread_mutex_unlock(&m_lock);
    }
private:
    pthread_mutex_t m_lock;
};


class RWMutex : NoCopyable{
public:
    typedef ReadLockImpl<RWMutex> ReadLock;
    typedef WriteLockImpl<RWMutex> WriteLock;
    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }
    ~RWMutex() {
        i = -99;
        pthread_rwlock_destroy(&m_lock);
    }

    void rlock() {
        i++;
        pthread_rwlock_rdlock(&m_lock);
    }
    void wlock() {
        i++;
        pthread_rwlock_wrlock(&m_lock);
    }
    void unlock() {
        i--;
        pthread_rwlock_unlock(&m_lock);
    }
private:
    int i = 0;
    pthread_rwlock_t m_lock;
};

// 
typedef ReadLockImpl<RWMutex> ReadLock;
typedef WriteLockImpl<RWMutex> WriteLock;
typedef MutexLockImpl<Mutex> MutexLock;


class Spinlock : NoCopyable {
public:
    /// 局部锁
    typedef MutexLockImpl<Spinlock> Lock;

    /**
     * @brief 构造函数
     */
    Spinlock() {
        pthread_spin_init(&m_mutex, 0);
    }

    /**
     * @brief 析构函数
     */
    ~Spinlock() {
        pthread_spin_destroy(&m_mutex);
    }

    /**
     * @brief 上锁
     */
    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    /**
     * @brief 解锁
     */
    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    /// 自旋锁
    pthread_spinlock_t m_mutex;
};

/**
 * @brief 原子锁
 */
class CASLock : NoCopyable {
public:
    /// 局部锁
    typedef MutexLockImpl<CASLock> Lock;

    /**
     * @brief 构造函数
     */
    CASLock() {
        m_mutex.clear();
    }

    /**
     * @brief 析构函数
     */
    ~CASLock() {
    }

    /**
     * @brief 上锁
     */
    void lock() {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }

    /**
     * @brief 解锁
     */
    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:
    /// 原子状态
    volatile std::atomic_flag m_mutex;
};


class Scheduler;
class FiberSemaphore : NoCopyable {
public:
    typedef Spinlock MutexType;

    FiberSemaphore(size_t initial_concurrency = 0);
    ~FiberSemaphore();

    bool tryWait();
    void wait();
    void notify();
    void notifyAll();

    size_t getConcurrency() const { return m_concurrency;}
    void reset() { m_concurrency = 0;}
private:
    MutexType m_mutex;
    std::list<std::pair<Scheduler*, Fiber::ptr>> m_waiters;
    size_t m_concurrency;
};



}

#endif //DREAMER_D_LOCK_H
