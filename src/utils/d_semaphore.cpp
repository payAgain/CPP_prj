//
// Created by YiMing D on 2023/1/14.
//

#include "d_semaphore.h"
#include "log.h"

namespace dreamer {

Semaphore::Semaphore(long count) {
    sem_init(&m_sem, 0, count);
}

Semaphore::~Semaphore() {
    sem_destroy(&m_sem);
}


void Semaphore::wait() {
    sem_wait(&m_sem);
}

void Semaphore::notify() {
    long rt =sem_post(&m_sem);
}


}