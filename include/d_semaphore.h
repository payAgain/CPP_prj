//
// Created by YiMing D on 2023/1/14.
//

#ifndef DREAMER_D_SEMAPHORE_H
#define DREAMER_D_SEMAPHORE_H
#include "nocopyable.h"
#include "semaphore.h"
#include "dispatch/dispatch.h"

namespace dreamer {
class Semaphore : NoCopyable {
public:
    explicit Semaphore(long count = 0);
    ~Semaphore();
//    Semaphore& operator=(Semaphore&& sem);

    void wait();
    void notify();
private:
    dispatch_semaphore_t m_sem;
};
}

#endif //DREAMER_D_SEMAPHORE_H
