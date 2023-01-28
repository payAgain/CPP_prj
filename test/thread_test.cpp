//
// Created by YiMing D on 2023/1/13.
//
#include "d_thread.h"
#include "log.h"
#include "d_lock.h"
#include "d_exception.h"

void func() {
    D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "Thread id: " << dreamer::Thread::GetThisThread()->getThreadId()
                                         << " Thread name: " << dreamer::Thread::GetThreadName();
    DREAMER_ASSERT(false);
}
int cnt = 0;
int lm = 10;
dreamer::Semaphore sem_p;
dreamer::Semaphore sem_c;
void pr() {
    while(true) {
        sleep(1);
        while (cnt < lm) {
            cnt++;
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "pr product 1 cnt" << cnt;
        }
        sem_c.notify();
        sem_p.wait();
    }
}
void cm() {
    while(true) {
        sem_c.wait();
        while (cnt > 0) {
            cnt--;
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "pr consume 1 cnt" << cnt;
        }
        sem_p.notify();
    }

}

dreamer::RWMutex lock{};
dreamer::Mutex lock1{};
void test_mutex() {
//    dreamer::WriteLockImpl<dreamer::RWMutex> rw(lock);
    //    rwt.wlock();
    for(int j = 0; j < 100000; j++) {
//        dreamer::ReadLock rd(lock);
//        dreamer::ReadLockImpl<dreamer::RWMutex> rw(lock);
        dreamer::MutexLock mt(lock1);
        cnt++;
    }
//    rwt.unlock();
}

int main() {
    std::vector<dreamer::Thread::ptr> t;
    for(int i = 0; i < 1; i++) {
        t.emplace_back(new dreamer::Thread(func, "t_name" + std::to_string(i)));
    }
    for(int i = 0; i < 1; i++) {
        t[i]->join();
    }

    // 生产者消费者模型同步测试
//    dreamer::Thread t1(pr, "pr");
//    dreamer::Thread t2(cm, "cm");
//
//    t1.join();
//    t2.join();


//    std::vector<dreamer::Thread::ptr> t;
//    for(int i = 0; i < 5; i++) {
//        t.emplace_back(new dreamer::Thread(test_mutex, "t_name" + std::to_string(i)));
//    }
//    for(int i = 0; i < 5; i++) {
//        t[i]->join();
//    }
    D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << cnt;
}