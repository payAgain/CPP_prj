//
// Created by YiMing D on 2022/11/24.
//

#include <unistd.h>
#include "my_thread.h"
#include <sys/syscall.h>

namespace dreamer {

int32_t get_thread_id() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

std::string get_thread_name() {
    auto t = pthread_self();
    char buf[100];
    pthread_getname_np(t, buf, sizeof (buf));
    return buf;
}
}