//
// Created by YiMing D on 2022/11/24.
//

#ifndef DREAMER_MY_THREAD_H
#define DREAMER_MY_THREAD_H

#include "thread"
#include "nocopyable.h"
#include "functional"
#include "memory"
#include "pthread.h"
#include "my_semaphore.h"

namespace dreamer{

int32_t get_thread_id();
std::string get_thread_name();
int set_thread_name(const char* _name);

class Thread : NoCopyable {
public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();
//    Thread(Thread &&t);
//    Thread& operator=(Thread&& t);

    void join();
    void detach();

    uint64_t getThreadId() {return m_id;}
    const std::string& getThreadName() {return m_name;}

    static void* run(void* arg);
    static Thread* GetThisThread();
    static const std::string& GetThreadName();
    static void SetThreadName(const std::string& name);
private:
    uint64_t m_id;
    pthread_t m_thread;
    std::function<void()> m_cb;
    std::string m_name;
    bool m_joined_detached = false;
    Semaphore m_sem;
};


// 利用RAII对线程生命周期进行管理
//class ThreadGuard {
//    std::thread t;
//public:
//    ThreadGuard() noexcept=default;
//    template<typename Callable,typename ... Args>
//    explicit ThreadGuard(Callable&& func,Args&& ... args)
//                :t(std::forward<Callable>(func),std::forward<Args>(args)...) {}
//    explicit ThreadGuard(std::thread t_) noexcept
//                :t(std::move(t_)) {}
//    ThreadGuard(ThreadGuard&& other) noexcept
//                :t(std::move(other.t)) {}
//    #if defined(__linux__)
//    int set_thread_name(const char* _name);
//    #endif
//    ThreadGuard& operator=(ThreadGuard&& other) noexcept {
//        if (joinable()) {
//            join();
//        }
//        t = std::move(other.t);
//        return *this;
//    }
//    ThreadGuard& operator=(std::thread other) noexcept {
//        if(joinable())
//            join();
//        t=std::move(other);
//        return *this;
//    }
//    ~ThreadGuard() noexcept {
//        if(joinable())
//            join();
//    }
//    void swap(ThreadGuard& other) noexcept {
//        t.swap(other.t);
//    }
//    std::thread::id get_id() const noexcept {
//        return t.get_id();
//    }
//    bool joinable() const noexcept {
//        return t.joinable();
//    }
//    void join() {
//        t.join();
//    }
//    void detach() {
//        t.detach();
//    }
//    std::thread& as_thread() noexcept {
//        return t;
//    }
//    const std::thread& as_thread() const noexcept {
//        return t;
//    }
//};
}
#endif //DREAMER_MY_THREAD_H
