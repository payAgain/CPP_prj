//
// Created by YiMing D on 2023/1/20.
//

#ifndef DREAMER_SCHEDULER_H
#define DREAMER_SCHEDULER_H

#include "memory"
#include "d_lock.h"
#include "fiber.h"
#include "d_thread.h"
#include "vector"
#include "list"

namespace dreamer {
class Scheduler {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    // use_caller 执行调度器的线程是否被纳入调度
    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "UNKNOWN");
    virtual ~Scheduler();

    // 获取线程变量 正在执行的调度器
    static Scheduler* GetThis();
    // 正在执行的协程
    static Fiber* GetMainFiber();


    void start();
    void stop();

    const std::string& getName() { return m_name; }

    // 将任务加入调度队列
    template<class Task>
    void schedule(Task fc, int thread = -1) {
        bool need_tickle = false;
        {
            MutexLock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
        }
        if(need_tickle) {
            tickle();
        }
    }

    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end)  {
        bool need_tickle = false;
        {
            MutexLock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(*begin, -1) || need_tickle;
                ++begin;
            }
        }
        if(need_tickle) {
            tickle();
        }
    }
protected:
    virtual void tickle();
    // 协程调度函数 线程运行函数
    void run();
    // @brief 返回是否可以停止
    virtual bool stopping();
    // @brief 协程无任务可调度时执行idle协程
    virtual void idle();
    // @brief 设置当前的协程调度器
    void setThis();
    // 是否有空闲线程
    bool hasIdleThreads() { return m_idleThreadCount > 0;}

private:
    template<class TASK>
    bool scheduleNoLock(TASK fc, int thread) {
        bool need_tickle = m_tasks.empty();
        Task ft(fc, thread);
        if(ft.fiber || ft.cb) {
            m_tasks.push_back(ft);
        }
        return need_tickle;
    }

    struct Task{
        Fiber::ptr fiber;
        std::function<void()> cb;
        // 在指定的线程上运行 这里使用long是为了使用-1作为特殊值
        long thread_id;

        Task(std::function<void()> _cb, long id) : cb(std::move(_cb)), thread_id(id) {}
        Task(Fiber::ptr _fiber, long id) : fiber(std::move(_fiber)), thread_id(id) {}
        Task() : thread_id(-1) {}
        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread_id = -1;
        }
    };

    // 互斥锁
    MutexType m_mutex;
    // 线程池
    std::vector<Thread::ptr> m_threads;
    // 任务队列
    std::list<Task> m_tasks;
    // use_caller为true时有效, 调度协程
    Fiber::ptr m_rootFiber;
    // 调度器名称
    std::string m_name;
protected:
    /// 协程下的线程id数组
    std::vector<int> m_threadIds;
    /// 线程数量
    size_t m_threadCount = 0;
    /// 工作线程数量
    std::atomic<size_t> m_activeThreadCount = {0};
    /// 空闲线程数量
    std::atomic<size_t> m_idleThreadCount = {0};
    /// 是否正在停止
    bool m_stopping = true;
    /// 是否自动停止
    bool m_autoStop = false;
    /// 主线程id(use_caller)
    int m_rootThread = 0;
};

}

#endif //DREAMER_SCHEDULER_H
