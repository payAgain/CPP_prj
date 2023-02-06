//
// Created by YiMing D on 2023/1/20.
//

#include "scheduler.h"
#include "log.h"
#include "d_exception.h"
#include "d_hook.h"

namespace dreamer {

static Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

// 全局调度器
static thread_local Scheduler* t_scheduler = nullptr;
// 调度器当前协程
static thread_local Fiber* t_scheduler_fiber = nullptr;

// constructor
Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
        : m_name(name) {
    DREAMER_ASSERT(threads > 0);

    if(use_caller) {
        // 在创建调度器的地方创建主协程
        dreamer::Fiber::GetThis();
        --threads;

        // 确保只能存在一个调度器
        DREAMER_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

//        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        m_rootFiber.reset(new Fiber([this] { run(); }));
        dreamer::Thread::SetThreadName(m_name);

        t_scheduler_fiber = m_rootFiber.get();
//        t_scheduler_fiber = Fiber::GetThis().get();
        m_rootThread = dreamer::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } else {
        m_rootThread = -1;
    }
    m_threadCount = threads;
}
Scheduler::~Scheduler() {
    // 当m_stopping为true可以析构
    DREAMER_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}


// static
Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}
void Scheduler::setThis() {
    t_scheduler = this;
}
Fiber* Scheduler::GetMainFiber() {
    return t_scheduler_fiber;
}

// member func
void Scheduler::start() {
    MutexLock lock(m_mutex);
    if(!m_stopping) {
        return;
    }
    m_stopping = false;
    DREAMER_ASSERT(m_threads.empty());

    // 根据线程池大小创建线程 将线程ID放入ID池
    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread([this] { run(); }
                , m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getThreadId());
    }
    D_SLOG_INFO(g_logger) << "Scheduler创建成功";
    // 这里需要手动释放锁
    // 因为如果当前执行线程SwapIn走后，锁不会释放掉
    lock.unlock();

//    if(m_rootFiber) {
//        m_rootFiber->swapIn();
////        m_rootFiber->call();
//        D_SLOG_INFO(g_logger) << "call out " << m_rootFiber->getState();
//    }
}
void Scheduler::run() {
    D_SLOG_DEBUG(g_logger) << m_name << " run";
    D_SLOG_DEBUG(g_logger) << "run方法的协程id: " << Fiber::GetThis()->m_id;
    set_hook_enable(true);
    setThis();
    if(dreamer::GetThreadId() != m_rootThread) {
        t_scheduler_fiber = Fiber::GetThis().get();
    }

    // 什么都不做的时候，就执行idle 具体做什么 取决于调度器的子类
    // 占住CPU轮空 或者 sleep让出CPU
    Fiber::ptr idle_fiber(new Fiber([this] { idle(); }));
    // 实际执行的fiber 在Task只有cb的情况下使用
    Fiber::ptr cb_fiber;

    Task ft;
    while(true) {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            // 处理任务队列的任务 需要加锁
            // 这个锁的粒度是否有些大
            MutexLock lock(m_mutex);
            auto it = m_tasks.begin();
            while(it != m_tasks.end()) {
                // 指定了运行线程，但是当前线程并不满足要求 跳过
                if(it->thread_id != -1 && it->thread_id != dreamer::GetThreadId()) {
                    ++it;
                    // 需要通知别人去处理
                    tickle_me = true;
                    continue;
                }

                DREAMER_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                // 将任务队列的任务复制给当前的协程，并从任务队列中删除
                ft = *it;
                m_tasks.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_tasks.end();
        }
        if(tickle_me) {
            tickle();
        }

        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT)) {
            ft.fiber->swapIn();
            --m_activeThreadCount;

            // 如果协程调用了 YIELTOREAD 那么重新将协程放入任务队列
            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if(ft.fiber->getState() != Fiber::TERM
                      && ft.fiber->getState() != Fiber::EXCEPT) {
                // 执行完成后 状态变为HOLD
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        } else if(ft.cb) {
            // 如果是空指针，那么就创建新协程，如果不存在，那么就复用相应的协程
            D_SLOG_DEBUG(g_logger) << "当前运行的线程为: " << Fiber::GetThis()->m_id
                                     << "  当前协程的状态" << &Fiber::GetThis()->m_ctx;
//            if(cb_fiber) {
//                cb_fiber->reset(ft.cb);
//            } else {
                cb_fiber.reset(new Fiber(ft.cb));
//            }
            // 重置任务
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if(cb_fiber->getState() == Fiber::EXCEPT
                      || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {//if(cb_fiber->getState() != Fiber::TERM) {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        } else {
            // 没有任务的情况下 进入idle状态 执行idle
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }
            // idle结束 run函数结束
            if(idle_fiber->getState() == Fiber::TERM) {
                D_SLOG_INFO(g_logger) << "idle fiber term";
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM
               && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
        }
    }
}
void Scheduler::stop() {
    m_autoStop = true;
    // 当且仅有一个主线程的情况
    if(m_rootFiber
       && m_threadCount == 0
       && (m_rootFiber->getState() == Fiber::TERM
           || m_rootFiber->getState() == Fiber::INIT)) {
        D_SLOG_INFO(g_logger) << this->getName() << " stopped";
        m_stopping = true;

        if(stopping()) {
            return;
        }
    }

    //bool exit_on_this_fiber = false;
    // 当设置了use_call 只能通过主线程进行释放
    if(m_rootThread != -1) {
        DREAMER_ASSERT(GetThis() == this);
    } else {
        DREAMER_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }
    // 当调用了use_caller时， rootFiber需要单独的tickle 不包含在threadCount
    if(m_rootFiber) {
        tickle();
    }

    if(m_rootFiber) {
        //while(!stopping()) {
        //    if(m_rootFiber->getState() == Fiber::TERM
        //            || m_rootFiber->getState() == Fiber::EXCEPT) {
        //        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        //        SYLAR_LOG_INFO(g_logger) << " root fiber is term, reset";
        //        t_fiber = m_rootFiber.get();
        //    }
        //    m_rootFiber->call();
        //}
        if(!stopping()) {
            m_rootFiber->swapIn();
        }
    }
    // 加锁 将成员的线程函数置空
    std::vector<Thread::ptr> thrs;
    {
        MutexLock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }
    //if(exit_on_this_fiber) {
    //}
}
bool Scheduler::stopping() {
    // m_tasks是一个list 所以需要加锁
    MutexLock lock(m_mutex);
    return m_autoStop && m_stopping
           && m_tasks.empty() && m_activeThreadCount == 0;
}
void Scheduler::tickle() {
//    D_SLOG_INFO(g_logger) << "tickle_before";
//    start();
    D_SLOG_INFO(g_logger) << "tickle_after";
}
void Scheduler::idle() {
    D_SLOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        dreamer::Fiber::YieldToHold();
    }
}
}