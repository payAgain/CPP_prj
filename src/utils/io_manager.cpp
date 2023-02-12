//
// Created by YiMing D on 2023/1/27.
//

#include "io_manager.h"
#include "sys/epoll.h"
#include "string.h"
#include "d_exception.h"
#include "fcntl.h"
#include "log.h"

namespace dreamer {

static Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

// static method
IOManager* IOManager::GetThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}



IOManager::IOManager(size_t threads, bool use_caller, const std::string &name)
                : Scheduler(threads, use_caller, name) {
    m_epfd = epoll_create(5000);
    DREAMER_ASSERT(m_epfd > 0);

    int rt = pipe(m_tickleFds);
    DREAMER_ASSERT(!rt);

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_tickleFds[0];

    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    DREAMER_ASSERT(!rt);

    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    DREAMER_ASSERT(!rt);

    contextResize(32);

    start();
}
IOManager::~IOManager() {
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
}

void IOManager::contextResize(size_t size) {
    m_fdContexts.resize(size);

    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}

int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
    FdContext* fd_ctx = nullptr;
    ReadLock lock(m_mutex);
    
    D_SLOG_DEBUG(g_logger) << "fd: " << fd << "try to add event" << event << " with cb" << &cb;
    // if (fd > size) resize the vector
    if((int)m_fdContexts.size() > fd) {
        fd_ctx = m_fdContexts[fd];
        lock.unlock();
    } else {
        lock.unlock();
        WriteLock lock2(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_fdContexts[fd];
    }

    MutexLock lock2(fd_ctx->mutex);
    //  if fd_ctx has contain same events throw error
    if(DREAMER_UNLIKELY(fd_ctx->events & event)) {
        D_SLOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                    << " event=" << (EPOLL_EVENTS)event
                    << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events;
        DREAMER_ASSERT(!(fd_ctx->events & event));
    }

    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    // set event type
    epevent.events = EPOLLET | (EPOLL_EVENTS)fd_ctx->events | (EPOLL_EVENTS)event;
    // throw data pass value to another thread
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        D_SLOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->events;
        return -1;
    }

    ++m_pendingEventCount;
    fd_ctx->events = (Event)(fd_ctx->events | event);
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    DREAMER_ASSERT(!event_ctx.scheduler
                && !event_ctx.fiber
                && !event_ctx.cb);

    event_ctx.scheduler = Scheduler::GetThis();
    if(cb) {
        event_ctx.cb = std::move(cb);
    } else {
        event_ctx.fiber = Fiber::GetThis();
        DREAMER_ASSERT2(event_ctx.fiber->getState() == Fiber::EXEC
                      ,"state=" << event_ctx.fiber->getState());
    }
    return 0;
}

bool IOManager::delEvent(int fd, Event event) {
    ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    MutexLock lock2(fd_ctx->mutex);
    // if del event not exist  false
    if(DREAMER_UNLIKELY(!(fd_ctx->events & event))) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | (EPOLL_EVENTS)new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        D_SLOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    --m_pendingEventCount;
    fd_ctx->events = new_events;
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}

bool IOManager::cancelEvent(int fd, Event event) {
    ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    MutexLock lock2(fd_ctx->mutex);
    if(DREAMER_UNLIKELY(!(fd_ctx->events & event))) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | (EPOLL_EVENTS)new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        D_SLOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}

bool IOManager::cancelAll(int fd) {
    ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    MutexLock lock2(fd_ctx->mutex);
    if(!fd_ctx->events) {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        D_SLOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    if(fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if(fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }

    DREAMER_ASSERT(fd_ctx->events == NONE);
    return true;
}

void IOManager::tickle() {
    if(!hasIdleThreads()) {
        return;
    }
    int rt = write(m_tickleFds[1], "T", 1);
    DREAMER_ASSERT(rt == 1);
}

bool IOManager::stopping(uint64_t& timeout) {
    timeout = getNextTimer();
    // return Scheduler::stopping() && m_pendingEventCount == 0;
    return timeout == ~0ull
        && m_pendingEventCount == 0
        && Scheduler::stopping();
}

bool IOManager::stopping() {
    uint64_t timeout = 0;
    return stopping(timeout);
}

void IOManager::idle() {
    D_SLOG_DEBUG(g_logger) << "idle";
    const uint64_t MAX_EVNETS = 256;
    epoll_event* events = new epoll_event[MAX_EVNETS]();
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
        delete[] ptr;
    });
    // return;
    while(true) {
        // sleep(1);
        uint64_t next_timeout = 0;
        if(DREAMER_UNLIKELY(stopping(next_timeout))) {
            D_SLOG_INFO(g_logger) << "name=" << getName()
                                     << " idle stopping exit";
            break;
        }

        int rt = 0;
        do {
            static const int MAX_TIMEOUT = 3000;
            if(next_timeout != ~0ull) {
                next_timeout = (int)next_timeout > MAX_TIMEOUT
                                ? MAX_TIMEOUT : next_timeout;
            } else {
                next_timeout = MAX_TIMEOUT;
            }
            rt = epoll_wait(m_epfd, events, MAX_EVNETS, (int)next_timeout);
            // when errno == 0 means system interupt occur, continue the idle
            if(rt < 0 && errno == EINTR) {
            } else {
                break;
            }
        } while(true);
        D_SLOG_DEBUG(g_logger) << "epoll ret = " << rt;
        
        std::vector<std::function<void()> > cbs;
        listExpiredCb(cbs);
        if(!cbs.empty()) {
            D_SLOG_DEBUG(g_logger) << "on timer cbs.size=" << cbs.size();
            schedule(cbs.begin(), cbs.end());
            cbs.clear();
        }

        if(DREAMER_UNLIKELY(rt == MAX_EVNETS)) {
           D_SLOG_INFO(g_logger) << "epoll wait events=" << rt;
        }

        for(int i = 0; i < rt; ++i) {
            epoll_event& event = events[i];
            if(event.data.fd == m_tickleFds[0]) {
                uint8_t dummy[256];
                while(read(m_tickleFds[0], dummy, sizeof(dummy)) > 0);
                continue;
            }

            FdContext* fd_ctx = (FdContext*)event.data.ptr;
            MutexLock lock(fd_ctx->mutex);
            if(event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
            }
            int real_events = NONE;
            if(event.events & EPOLLIN) {
                real_events |= READ;
            }
            if(event.events & EPOLLOUT) {
                real_events |= WRITE;
            }

            if((fd_ctx->events & real_events) == NONE) {
                continue;
            }

            int left_events = (fd_ctx->events & ~real_events);
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;

            int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if(rt2) {
                D_SLOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                    << op << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                    << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
            }

            //SYLAR_LOG_INFO(g_logger) << " fd=" << fd_ctx->fd << " events=" << fd_ctx->events
            //                         << " real_events=" << real_events;
            if(real_events & READ) {
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if(real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        }

        Fiber::YieldToHold();

        // Fiber::ptr cur = Fiber::GetThis();
        // auto raw_ptr = cur.get();
        // cur.reset();

        // raw_ptr->swapOut();
    }
}

void IOManager::onTimerInsertedAtFront() {
    tickle();
}

void IOManager::FdContext::resetContext(EventContext& ctx) {
    ctx.clear();
}

void IOManager::FdContext::triggerEvent(IOManager::Event event) {
    //SYLAR_LOG_INFO(g_logger) << "fd=" << fd
    //    << " triggerEvent event=" << event
    //    << " events=" << events;
    //SYLAR_ASSERT(events & event);
    if(DREAMER_UNLIKELY(!(events & event))) {
        D_SLOG_ERROR(g_logger) << "fd=" << fd
            << " triggerEvent event=" << event
            << " events=" << events
            << "\nbacktrace:\n"
            << dreamer::BacktraceToString(100, 2, "    ");
        return;
    }
    // set trigger events 0
    events = (Event)(events & ~event);
    EventContext& ctx = getContext(event);
    if(ctx.cb) {
        ctx.scheduler->schedule(ctx.cb);
    } else {
        ctx.scheduler->schedule(ctx.fiber);
    }
    // 
    ctx.clear(); 
    return;
}

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(Event event) {
    switch(event) {
        case IOManager::READ:
            return read;
        case IOManager::WRITE:
            return write;
        default:
            DREAMER_ASSERT2(false, "getContext");
        }
        throw std::invalid_argument("getContext invalid event");
    }
}
