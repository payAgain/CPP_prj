#ifndef __DREAMER_WORKER_H__
#define __DREAMER_WORKER_H__

#include "d_lock.h"
#include "singleton.h"
#include "log.h"
#include "scheduler.h"
#include "io_manager.h"
#include "nocopyable.h"

namespace dreamer {

class WorkerGroup : NoCopyable, public std::enable_shared_from_this<WorkerGroup> {
public:
    typedef std::shared_ptr<WorkerGroup> ptr;
    static WorkerGroup::ptr Create(uint32_t batch_size, Scheduler* s = Scheduler::GetThis()) {
        return std::make_shared<WorkerGroup>(batch_size, s);
    }

    WorkerGroup(uint32_t batch_size, Scheduler* s = Scheduler::GetThis());
    ~WorkerGroup();

    void schedule(std::function<void()> cb, int thread = -1);
    void schedule(const std::vector<std::function<void()> >& cbs);
    void waitAll();
private:
    void doWork(std::function<void()> cb);
private:
    uint32_t m_batchSize;
    bool m_finish;
    Scheduler* m_scheduler;
    FiberSemaphore m_sem;
};

class TimedWorkerGroup : NoCopyable, public std::enable_shared_from_this<TimedWorkerGroup> {
public:
    typedef std::shared_ptr<TimedWorkerGroup> ptr;
    static TimedWorkerGroup::ptr Create(uint32_t batch_size, uint32_t wait_ms, IOManager* s = IOManager::GetThis());

    TimedWorkerGroup(uint32_t batch_size, uint32_t wait_ms, IOManager* s = IOManager::GetThis());
    ~TimedWorkerGroup();

    void schedule(std::function<void()> cb, int thread = -1);
    void waitAll();
private:
    void doWork(std::function<void()> cb);
    void start();
    void onTimer();
private:
    uint32_t m_batchSize;
    bool m_finish;
    bool m_timedout;
    uint32_t m_waitTime;
    Timer::ptr m_timer;
    IOManager* m_iomanager;
    FiberSemaphore m_sem;
};

class WorkerManager {
public:
    WorkerManager();
    void add(Scheduler::ptr s);
    Scheduler::ptr get(const std::string& name);
    IOManager::ptr getAsIOManager(const std::string& name);

    template<class FiberOrCb>
    void schedule(const std::string& name, FiberOrCb fc, int thread = -1) {
        auto s = get(name);
        if(s) {
            s->schedule(fc, thread);
        } else {
            static Logger::ptr s_logger = DREAMER_SYSTEM_LOGGER();
            D_SLOG_ERROR(s_logger) << "schedule name=" << name
                << " not exists";
        }
    }

    template<class Iter>
    void schedule(const std::string& name, Iter begin, Iter end) {
        auto s = get(name);
        if(s) {
            s->schedule(begin, end);
        } else {
            static Logger::ptr s_logger = DREAMER_SYSTEM_LOGGER();
            D_SLOG_ERROR(s_logger) << "schedule name=" << name
                << " not exists";
        }
    }

    bool init();
    bool init(const std::map<std::string, std::map<std::string, std::string> >& v);
    void stop();

    bool isStoped() const { return m_stop;}
    std::ostream& dump(std::ostream& os);

    uint32_t getCount();
private:
    void add(const std::string& name, Scheduler::ptr s);
private:
    std::map<std::string, std::vector<Scheduler::ptr> > m_datas;
    bool m_stop;
};

typedef Singleton<WorkerManager> WorkerMgr;

}

#endif
