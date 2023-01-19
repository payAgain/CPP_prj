//
// Created by YiMing D on 2023/1/17.
//
#include "fiber.h"
#include "log.h"
#include "my_exception.h"
#include "atomic"
#include "config.h"

namespace dreamer {
static Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

// 当前正在执行的协程
static thread_local Fiber* t_fiber = nullptr;
// 主协程
static thread_local Fiber::ptr t_threadFiber = nullptr;

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
uint64_t Fiber::GetFiberId() { return s_fiber_id; }

void Fiber::SetThis(dreamer::Fiber *f) {
    t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);
    DREAMER_ASSERT2(t_fiber == main_fiber.get(), "当前运行的线程需要和主线程一致")
    t_threadFiber = main_fiber;
    D_SLOG_INFO(g_logger) << "主协程创建成功";
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
//    auto raw_ptr = cur.get();
//    cur.reset();
//    raw_ptr->swapOut();

//    DREAMER_ASSERT2(false, "never reach fiber_id=" + std::to_string(cur->getId()));
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

    D_SLOG_DEBUG(g_logger) << "Fiber::Fiber main";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize)
        : m_id(++s_fiber_id), m_cb(std::move(cb)) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->get_value();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if (getcontext(&m_ctx)) {
        DREAMER_ASSERT2(false, "getcontext");
    }

//    m_ctx.uc_link = nullptr;
    m_ctx.uc_link = &t_threadFiber->m_ctx;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
}

Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {
        DREAMER_ASSERT(m_state == TERM
                     || m_state == EXCEPT
                     || m_state == INIT);

        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
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

void Fiber::swapIn() {
    SetThis(this);
    DREAMER_ASSERT2(m_state != EXEC, "正在运行的线程不能被换入");
    m_state = EXEC;
    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        DREAMER_ASSERT2(false, "swap context error");
    }
}

void Fiber::swapOut() {
    // 将执行协程切换为主协程
    SetThis(t_threadFiber.get());
    if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        DREAMER_ASSERT2(false, "协程换回失败");
    }
}

void Fiber::reset(std::function<void()> cb) {
    DREAMER_ASSERT(m_stack);
    DREAMER_ASSERT(m_state == TERM
                 || m_state == EXCEPT
                 || m_state == INIT);
    m_cb = std::move(cb);
    if(getcontext(&m_ctx)) {
        DREAMER_ASSERT2(false, "getcontext");
    }

//    m_ctx.uc_link = nullptr;
    m_ctx.uc_link = &t_threadFiber->m_ctx;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}



}