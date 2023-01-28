//
// Created by YiMing D on 2022/11/24.
//

#include <unistd.h>
#include "d_thread.h"
#include "log.h"
#include <sys/syscall.h>

#include <utility>

namespace dreamer {

int32_t GetThreadId() {
    static __thread uint64_t id;
    if (id != 0) return id;
    #if defined(__linux__)
    id = static_cast<pid_t>(::syscall(SYS_gettid));
    #elif __APPLE__
    pthread_threadid_np(pthread_self(), &id);
    #endif
    return id;
}
std::string GetThreadName() {
    auto t = pthread_self();
    char buf[100];
    pthread_getname_np(t, buf, sizeof (buf));
    return buf;
}
int SetThreadName(const char* _name) {
    int ret;
    #if defined(__linux__)
    ret = pthread_setname_np(pthread_self(), _name);
    #elif __APPLE__
    ret = pthread_setname_np(_name);
    #endif
    return ret;
}

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "Unknown";

Thread* Thread::GetThisThread() {
    return t_thread;
}
const std::string& Thread::GetThreadName() {
    return t_thread_name;
}
void Thread::SetThreadName(const std::string &name) {
    if (t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}


//Thread::Thread(Thread &&t) {
//    m_id = t.m_id;
//    m_thread = t.m_thread;
//    m_name = t.m_name;
//    m_joined_detached = t.m_joined_detached;
//    m_cb.swap(t.m_cb);
//    m_sem = std::move(t.m_sem);
//    t.m_id = 0;
//    t.m_joined_detached = false;
//    t.m_thread = nullptr;
//    t.m_name = "";
//    t.m_cb = nullptr;
//}
//Thread& Thread::operator=(Thread&& t){
//    m_id = t.m_id;
//    m_thread = t.m_thread;
//    m_name = t.m_name;
//    m_joined_detached = t.m_joined_detached;
//    m_cb.swap(t.m_cb);
//    m_sem = std::move(t.m_sem);
//    t.m_id = 0;
//    t.m_joined_detached = false;
//    t.m_thread = nullptr;
//    t.m_name = "";
//    t.m_cb = nullptr;
//    return *this;
//}
Thread::Thread(std::function<void()> cb, const std::string& name)
            : m_sem(0), m_name(name), m_cb(std::move(cb)) {
    if (name.empty()) {
        m_name = "Unknown";
    }
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    m_sem.wait();
    if (rt) {
        D_SLOG_ERROR(DREAMER_SYSTEM_LOGGER()) << "pthread create error rt=" << rt
                                              << " thread name: " << m_name;
        throw std::logic_error("thread create fail");
    }
}
Thread::~Thread() {
    if (!m_joined_detached)
        if (m_thread) {
            int rt = pthread_detach(m_thread);
            if (rt) {
                D_SLOG_ERROR(DREAMER_SYSTEM_LOGGER()) << "pthread cancel error rt=" << rt
                                                      << " thread name: " << m_name;
    //            throw std::logic_error("thread cancel fail");
            }
        }
}


void* Thread::run(void* arg) {
    auto* thread = static_cast<Thread *>(arg);
    t_thread = thread;
    t_thread->m_id = GetThreadId();
    t_thread_name = thread->m_name;
    SetThreadName(t_thread->m_name.substr(0, 15).c_str());
    std::function<void()> cb;
    cb = std::move(t_thread->m_cb);
    thread->m_sem.notify();
    cb();
}
void Thread::join() {
    if (m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if (rt) {
            D_SLOG_ERROR(DREAMER_SYSTEM_LOGGER()) << "pthread join error rt=" << rt
                                                  << " thread name: " << m_name;
            throw std::logic_error("thread join fail");
        }
        m_joined_detached = true;
    }
}
void Thread::detach() {
    if (m_thread) {
        int rt = pthread_detach(m_thread);
        if (rt) {
            D_SLOG_ERROR(DREAMER_SYSTEM_LOGGER()) << "pthread detach error rt=" << rt
                                                  << " thread name: " << m_name;
            throw std::logic_error("thread detach fail");
        }
        m_joined_detached = true;
    }
}




// implement thread guard
//#if defined(__linux__)
//int ThreadGuard::SetThreadName(const char* _name) {
//    return pthread_setname_np(t.native_handle(), _name);
//}
//#endif

}




// implement thread guard
//#if defined(__linux__)
//int ThreadGuard::SetThreadName(const char* _name) {
//    return pthread_setname_np(t.native_handle(), _name);
//}
//#endif
