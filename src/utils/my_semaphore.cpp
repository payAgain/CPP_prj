//
// Created by YiMing D on 2023/1/14.
//

#include "my_semaphore.h"
#include "log.h"

namespace dreamer {

Semaphore::Semaphore(long count) {
    m_sem = dispatch_semaphore_create(count);
//    if () {
//        D_SLOG_ERROR(DREAMER_STD_ROOT_LOGGER()) << "Sem init Error";
//        throw std::logic_error("sem init error");
//    }
}

Semaphore::~Semaphore() {
    dispatch_release(m_sem);
}

//Semaphore& Semaphore::operator=(Semaphore&& sem) {
//    m_sem = sem.m_sem;
//    m_sem = nullptr;
//}

void Semaphore::wait() {
    dispatch_semaphore_wait(m_sem, DISPATCH_TIME_FOREVER);
//    if (sem_wait(&m_sem)) {
//        D_SLOG_ERROR(DREAMER_STD_ROOT_LOGGER()) << "sem_wait Error";
//        throw std::logic_error("sem_wait error");
//    }
}

void Semaphore::notify() {
    dispatch_semaphore_signal(m_sem);
//    if (sem_post(&m_sem)) {
//        D_SLOG_ERROR(DREAMER_STD_ROOT_LOGGER()) << "sem_post Error";
//        throw std::logic_error("sem_post error");
//    }
}


}