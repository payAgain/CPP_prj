//
// Created by YiMing D on 2023/1/13.
//
#include "my_thread.h"
#include "log.h"

void func() {
    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << "Thread id: " << dreamer::Thread::GetThisThread()->getThreadId()
            << " Thread name: " << dreamer::Thread::GetThreadName();
}

int main() {
//    std::vector<dreamer::Thread::ptr> t;
    std::vector<dreamer::Thread> e;
    for(int i = 0; i < 5; i++) {
//        t.emplace_back(new dreamer::Thread(func, "t_name" + std::to_string(i)));
        e.emplace_back(func, "e_name" + std::to_string(i));
    }

    for(int i = 0; i < 5; i++) {
//        t[i]->join();
        e[i].join();
    }
}