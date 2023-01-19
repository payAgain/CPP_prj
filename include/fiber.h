//
// Created by YiMing D on 2023/1/17.
//

#ifndef DREAMER_FIBER_H
#define DREAMER_FIBER_H

#include "memory"
#include "functional"
#include "ucontext.h"

namespace dreamer {

class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    typedef std::shared_ptr<Fiber> ptr;
    // 协程状态
    enum State {
        INIT,
        READY,
        EXEC,
        HOLD,
        TERM,
        EXCEPT
    };
private:
    // 创建主协程
    Fiber();
public:
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();

    void reset(std::function<void()> cb);
    void swapIn();
    void swapOut();
//    void call();
//    void back();

    uint64_t getId() const { return m_id;}
    State getState() const { return m_state;}
public:
    static void SetThis(Fiber* f);

    //获取当前运行的协程 如果不存在 则创建主协程
    static Fiber::ptr GetThis();

    //切换到后台，并且设置为Ready状态
    static void YieldToReady();
    //切换到后台，并且设置为Hold状态
    static void YieldToHold();

    static uint64_t TotalFibers();

// 主协程运行 执行当前协程的cb
    static void MainFunc();

//    static void CallerMainFunc();

    static uint64_t GetFiberId();

private:
    /// 协程id
    uint64_t m_id = 0;
    /// 协程运行栈大小
    uint32_t m_stacksize = 0;
    /// 协程状态
    State m_state = INIT;
    /// 协程上下文
    ucontext_t m_ctx;
    /// 协程运行栈指针
    void* m_stack = nullptr;
    /// 协程运行函数
    std::function<void()> m_cb;
};

}

#endif //DREAMER_FIBER_H
