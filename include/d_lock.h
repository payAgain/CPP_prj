//
// Created by YiMing D on 2023/1/15.
//

#ifndef DREAMER_D_LOCK_H
#define DREAMER_D_LOCK_H
#include "nocopyable.h"
#include "pthread.h"

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

typedef ReadLockImpl<RWMutex> ReadLock;
typedef WriteLockImpl<RWMutex> WriteLock;
typedef MutexLockImpl<Mutex> MutexLock;

}

#endif //DREAMER_D_LOCK_H
