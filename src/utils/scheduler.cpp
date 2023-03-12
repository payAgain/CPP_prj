//
// Created by YiMing D on 2023/1/20.
//

#include "scheduler.h"
#include "basic_log.h"
#include "d_exception.h"
#include "d_hook.h"
#include "log.h"

namespace dreamer {

static Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

// 全局调度器
static thread_local Scheduler *t_scheduler = nullptr;
// 调度器当前协程
static thread_local Fiber *t_scheduler_fiber = nullptr;

// constructor
Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
    : m_name(name) {
  DREAMER_ASSERT(threads > 0);

  if (use_caller) {
    // 在创建调度器的地方创建主协程
    dreamer::Fiber::GetThis();
    --threads;

    // 确保只能存在一个调度器
    DREAMER_ASSERT(GetThis() == nullptr);
    t_scheduler = this;
    m_rootFiber.reset(new Fiber([this] { run(); }));
    dreamer::Thread::SetThreadName(m_name);

    t_scheduler_fiber = m_rootFiber.get();
    m_rootThread = dreamer::GetThreadId();
    m_threadIds.push_back(m_rootThread);
  } else {
    m_rootThread = -1;
  }
  m_threadCount = threads;
}
Scheduler::~Scheduler() {
  // D_SLOG_INFO(g_logger) << "scheduler : " << m_name << " try to destroy";
  // 当m_stopping为true可以析构
  DREAMER_ASSERT(m_stopped);
  if (GetThis() == this) {
    t_scheduler = nullptr;
  }
}

// static
Scheduler *Scheduler::GetThis() { return t_scheduler; }
void Scheduler::setThis() { t_scheduler = this; }
Fiber *Scheduler::GetMainFiber() { return t_scheduler_fiber; }

// member func
void Scheduler::start() {
  MutexLock lock(m_mutex);
  if (m_stopping) {
    D_SLOG_WARN(g_logger) << "The Scheduler is stopping now";
    return;
  }
  m_stopping = false;
  DREAMER_ASSERT(m_threads.empty());

  // 根据线程池大小创建线程 将线程ID放入ID池
  m_threads.resize(m_threadCount);
  for (size_t i = 0; i < m_threadCount; ++i) {
    m_threads[i].reset(
        new Thread([this] { run(); }, m_name + "_" + std::to_string(i)));
    m_threadIds.push_back(m_threads[i]->getThreadId());
  }
  D_SLOG_INFO(g_logger) << "Scheduler创建成功";
  // 这里需要手动释放锁
  // 因为如果当前执行线程SwapIn走后，锁不会释放掉
  lock.unlock();
}
void Scheduler::run() {
  D_SLOG_DEBUG(g_logger) << m_name << " run";
  D_SLOG_DEBUG(g_logger) << "run方法的协程id: " << Fiber::GetThis()->m_id;
  set_hook_enable(true);
  setThis();
  if (dreamer::GetThreadId() != m_rootThread) {
    t_scheduler_fiber = Fiber::GetThis().get();
  }

  // 什么都不做的时候，就执行idle 具体做什么 取决于调度器的子类
  // 占住CPU轮空 或者 sleep让出CPU
  // 实际执行的fiber 在Task只有cb的情况下使用
  Fiber::ptr idle_fiber(new Fiber([this] { idle(); }));
  Task ft;
  while (true) {
    ft.reset();
    bool tickle_me = false;
    bool is_active = false;
    {
      // 处理任务队列的任务 需要加锁
      // 这个锁的粒度是否有些大
      MutexLock lock(m_mutex);
      auto it = m_tasks.begin();
      while (it != m_tasks.end()) {
        // 指定了运行线程，但是当前线程并不满足要求 跳过
        if (it->thread_id != -1 && it->thread_id != dreamer::GetThreadId()) {
          ++it;
          // 需要通知别人去处理
          tickle_me = true;
          continue;
        }

        DREAMER_ASSERT(it->fiber);
        if (it->fiber && it->fiber->getState() == Fiber::EXEC) {
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
    if (tickle_me) {
      tickle();
    }

    if (ft.fiber && (ft.fiber->getState() != Fiber::TERM &&
                     ft.fiber->getState() != Fiber::EXCEPT)) {
      ft.fiber->swapIn();
      --m_activeThreadCount;

      // 如果协程调用了 YIELTOREAD 那么重新将协程放入任务队列
      if (ft.fiber->getState() == Fiber::READY) {
        schedule(ft.fiber);
      } else if (ft.fiber->getState() != Fiber::TERM &&
                 ft.fiber->getState() != Fiber::EXCEPT) {
        // 执行完成后 状态变为HOLD
        ft.fiber->m_state = Fiber::HOLD;
      }
      ft.reset();
    } else {
      // 没有任务的情况下 进入idle状态 执行idle
      if (is_active) {
        --m_activeThreadCount;
        continue;
      }

      ++m_idleThreadCount;
      idle_fiber->swapIn();
      --m_idleThreadCount;
      // idle结束 run函数结束
      if (idle_fiber->getState() == Fiber::TERM) {
        D_SLOG_INFO(g_logger) << "idle fiber term";
        break;
      }
      if (idle_fiber->getState() != Fiber::TERM &&
          idle_fiber->getState() != Fiber::EXCEPT) {
        idle_fiber->m_state = Fiber::HOLD;
      }
    }
  }
}
void Scheduler::stop() {
  m_stopping = true;
  // 当且仅有一个主线程的情况
  if (m_rootFiber && m_threadCount == 0 &&
      (m_rootFiber->getState() == Fiber::TERM ||
       m_rootFiber->getState() == Fiber::INIT)) {
    D_SLOG_INFO(g_logger) << this->getName() << " stopped";
    if (!stopping()) {
      m_rootFiber->swapIn();
      m_stopped = true;
      return;
    }
  }
  // 当设置了use_call 只能通过主线程进行释放
  if (m_rootThread != -1) {
    DREAMER_ASSERT(GetThis() == this);
  } else {
    DREAMER_ASSERT(GetThis() != this);
  }
  for (size_t i = 0; i < m_threadCount; ++i) {
    tickle();
  }
  // 当调用了use_caller时， rootFiber需要单独的tickle 不包含在threadCount
  if (m_rootFiber) {
    tickle();
  }
  if (m_rootFiber) {
    if (!stopping()) {
      m_rootFiber->swapIn();
    }
  }
  // 加锁 将成员的线程函数置空
  std::vector<Thread::ptr> thrs;
  {
    MutexLock lock(m_mutex);
    thrs.swap(m_threads);
  }

  // waiting for thread join
  for (auto &i : thrs) {
    i->join();
  }
  if (stopping()) {
    m_stopped = true;
  }
}
bool Scheduler::stopping() {
  // m_tasks是一个list 所以需要加锁
  MutexLock lock(m_mutex);
  return m_stopping && m_tasks.empty() && m_activeThreadCount == 0;
}
void Scheduler::tickle() {}
void Scheduler::idle() {
  D_SLOG_INFO(g_logger) << "idle";
  while (!m_stopping) {
    sleep(2);
    dreamer::Fiber::YieldToHold();
  }
}
void Scheduler::logWarn() {
  D_SLOG_WARN(g_logger) << "Scheduler is stopping, can't add new Task!";
}
std::ostream &Scheduler::dump(std::ostream &os) {
  os << "[Scheduler name=" << m_name << " size=" << m_threadCount
     << " active_count=" << m_activeThreadCount
     << " idle_count=" << m_idleThreadCount << " stopping=" << m_stopping
     << " ]" << std::endl
     << "    ";
  for (size_t i = 0; i < m_threadIds.size(); ++i) {
    if (i) {
      os << ", ";
    }
    os << m_threadIds[i];
  }
  return os;
}

} // namespace dreamer