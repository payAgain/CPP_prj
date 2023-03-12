//
// Created by YiMing D on 2023/1/17.
//
#include "fiber.h"
#include "basic_log.h"
#include "log.h"
#include "d_exception.h"
#include "atomic"
#include "config.h"
#include "scheduler.h"

namespace dreamer {
static Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};
//Semaphore semaphore;
// 当前正在执行的协程
static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr thread_context = nullptr;

static ConfigVar<uint32_t>::ptr g_shared_stack_size =
        DREAMER_ROOT_CONFIG()->look_up("fiber.shared_stack_size", "fiber shared stack size", 64u);
struct shared_stack {
    Fiber** st;
    uint32_t cap;
    int32_t pos;
    shared_stack(uint32_t capacity) : st((Fiber **)malloc(sizeof (Fiber *) * capacity))
                                    , cap(capacity)
                                    , pos(-1) {
        D_SLOG_DEBUG(g_logger) << "shared stack base= " << st;
    }
    
    ~shared_stack() {
        free(st);
    }

    void push(Fiber* fb) {
        if (pos == cap - 1) {
            D_SLOG_ERROR(g_logger) << "shared stack overFlow";
            throw std::out_of_range("shared stack overFlow");
        }
        pos++;
        auto t = st + pos;
        *t = fb;
        D_SLOG_DEBUG(g_logger) << "current shared stack pos =" << pos << " base= " << st;
    }

    bool isEmpty() {
        return pos < 0;
    }

    Fiber* pop() {
        if (pos < 0) {
            std::out_of_range("shared stack is empty");

        }
        Fiber* tp = *(st + pos);
        D_SLOG_DEBUG(g_logger) << "pop shared stack pos =" << pos << " base= " << st;
        pos --;
        return tp;
    }
};

static thread_local shared_stack thread_shared_stack(g_shared_stack_size->get_value());

static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
        DREAMER_ROOT_CONFIG()->look_up("fiber.stack_size", "fiber stack size", 128u * 1024);

class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

// static
uint64_t Fiber::GetFiberId() { return t_fiber ? t_fiber->m_id : 0; }

void Fiber::SetThis(dreamer::Fiber *f) {
    t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }
    thread_context.reset(new Fiber);
    t_fiber = thread_context.get();
    return t_fiber->shared_from_this();
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    DREAMER_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        D_SLOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                               << " fiber_id=" << cur->getId()
                               << std::endl
                               << dreamer::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        D_SLOG_ERROR(g_logger) << "Fiber Except"
                               << " fiber_id=" << cur->getId()
                               << std::endl
                               << dreamer::BacktraceToString();
    }

    // 防止主协程无法被析构 引用计数不被清0
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();
//
    DREAMER_ASSERT2(false, "never reach fiber mainfunc end");
//    semaphore.notify();
}


void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    DREAMER_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}


void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    DREAMER_ASSERT(cur->m_state == EXEC);
    cur->m_state = HOLD;
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}


// member function
Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if (getcontext(&m_ctx)) {
        BOOST_ASSERT(false);
    }

    ++s_fiber_count;
    m_id = ++s_fiber_id;
    D_SLOG_DEBUG(g_logger) << "主协程创建成功 id: " << m_id << " ctx: " << &m_ctx;
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize)
        : m_id(++s_fiber_id), m_cb(std::move(cb)) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->get_value();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if (getcontext(&m_ctx)) {
        DREAMER_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);

    D_SLOG_DEBUG(g_logger) << "工作线程被创建 id: " << m_id << " ctx: " << &m_ctx;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {
        DREAMER_ASSERT(m_state == TERM
                     || m_state == EXCEPT
                     || m_state == INIT);

        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        // main fiber release when main fiber is destory, set running fiber nullptr
        DREAMER_ASSERT(!m_cb);
        DREAMER_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    D_SLOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
                              << " total=" << s_fiber_count;
}

//void Fiber::call() {
//    SetThis(Scheduler::GetMainFiber());
//    DREAMER_ASSERT2(m_state != EXEC, "正在运行的线程不能被换入");
//    m_state = EXEC;
////    GetThis()->m_ctx.uc_link = &Scheduler::GetMainFiber()->m_ctx;
//    if (swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
//        DREAMER_ASSERT2(false, "swap context error");
//    }
//}
//void Fiber::back() {
//    // 将执行协程切换为主协程
//    SetThis(Scheduler::GetMainFiber());
//    if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
//        DREAMER_ASSERT2(false, "协程换回失败");
//    }
//}

void Fiber::swapIn() {
    thread_shared_stack.push(GetThis().get());
    auto tmp = GetThis();
    SetThis(this);
    DREAMER_ASSERT2(m_state != EXEC, "正在运行的线程不能被换入");
    m_state = EXEC;
    D_SLOG_DEBUG(g_logger) << "当前正在执行的协程 id :" << tmp->m_id << " From ctx: "
                           << &tmp->m_ctx << " 前往的协程id: " << m_id <<  " to ctx: " << &m_ctx;
    if (swapcontext(&tmp->m_ctx, &m_ctx)) {
        DREAMER_ASSERT2(false, "swap context error");
    }
}

void Fiber::swapOut() {
    // 将执行协程切换为主协程
    auto pre = thread_shared_stack.pop();
    SetThis(pre);
    D_SLOG_DEBUG(g_logger) << "当前正在执行的协程 id :" << m_id << " From ctx: " << &m_ctx << " 前往的协程ID: " << pre->m_id <<  " to ctx: " << &pre->m_ctx;
    if (swapcontext(&m_ctx, &pre->m_ctx)) {
        DREAMER_ASSERT2(false, "协程换回失败");
    }
}

void Fiber::reset(std::function<void()> cb) {
//    semaphore.wait();
    DREAMER_ASSERT(m_stack);
    DREAMER_ASSERT(m_state == TERM
                 || m_state == EXCEPT
                 || m_state == INIT);
    m_cb = std::move(cb);
    if(getcontext(&m_ctx)) {
        DREAMER_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
//    m_ctx.uc_link = &GetThis()->m_ctx;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}



}