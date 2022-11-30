//
// Created by YiMing D on 2022/11/24.
//

#include <unistd.h>
#include "my_thread.h"
#include <sys/syscall.h>

namespace dreamer {

int32_t get_thread_id() {
    static __thread uint64_t id;
    if (id != 0) return id;
    #if defined(__linux__)
    id = static_cast<pid_t>(::syscall(SYS_gettid));
    #elif __APPLE__
    pthread_threadid_np(pthread_self(), &id);
    #endif
    return id;
}

std::string get_thread_name() {
    auto t = pthread_self();
    char buf[100];
    pthread_getname_np(t, buf, sizeof (buf));
    return buf;
}

int set_thread_name(const char* _name) {
    int ret;
    #if defined(__linux__)
    ret = pthread_setname_np(pthread_self(), _name);
    #elif __APPLE__
    ret = pthread_setname_np(_name);
    #endif
    return ret;
}

// implement thread guard
#if defined(__linux__)
int ThreadGuard::set_thread_name(const char* _name) {
    return pthread_setname_np(t.native_handle(), _name);
}
#endif
}