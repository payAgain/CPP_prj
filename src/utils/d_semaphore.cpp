//
// Created by YiMing D on 2023/1/14.
//

#include "d_semaphore.h"
#include "log.h"

namespace dreamer {

Semaphore::Semaphore(long count) {
    m_sem = dispatch_semaphore_create(count);
}

Semaphore::~Semaphore() {
    dispatch_release(m_sem);
}


void Semaphore::wait() {
    dispatch_semaphore_wait(m_sem, DISPATCH_TIME_FOREVER);
}

void Semaphore::notify() {
    long rt = dispatch_semaphore_signal(m_sem);
}


}