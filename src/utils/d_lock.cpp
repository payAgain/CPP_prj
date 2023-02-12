//
// Created by YiMing D on 2023/1/15.
//
#include "d_lock.h"
#include "scheduler.h"
#include "d_exception.h"

namespace dreamer {
FiberSemaphore::FiberSemaphore(size_t initial_concurrency)
    :m_concurrency(initial_concurrency) {
}

FiberSemaphore::~FiberSemaphore() {
    DREAMER_ASSERT(m_waiters.empty());
}

bool FiberSemaphore::tryWait() {
    DREAMER_ASSERT(Scheduler::GetThis());
    {
        MutexType::Lock lock(m_mutex);
        if(m_concurrency > 0u) {
            --m_concurrency;
            return true;
        }
        return false;
    }
}

void FiberSemaphore::wait() {
    DREAMER_ASSERT(Scheduler::GetThis());
    {
        MutexType::Lock lock(m_mutex);
        if(m_concurrency > 0u) {
            --m_concurrency;
            return;
        }
        m_waiters.push_back(std::make_pair(Scheduler::GetThis(), Fiber::GetThis()));
    }
    Fiber::YieldToHold();
}

void FiberSemaphore::notify() {
    MutexType::Lock lock(m_mutex);
    if(!m_waiters.empty()) {
        auto next = m_waiters.front();
        m_waiters.pop_front();
        next.first->schedule(next.second);
    } else {
        ++m_concurrency;
    }
}

void FiberSemaphore::notifyAll() {
    MutexType::Lock lock(m_mutex);
    for(auto& i : m_waiters) {
        i.first->schedule(i.second);
    }
    m_waiters.clear();
}
    

}