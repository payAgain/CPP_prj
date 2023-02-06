#include "d_hook.h"
#include "log.h"
#include "dlfcn.h"
#include "fiber.h"
#include "io_manager.h"
#include "config.h"
#include "fd_manager.h"
#include "stdarg.h"
#include "d_exception.h"
/* 
*    Macro usage
*    三种特殊用法
*    #define Conn(x,y) (x)##(y) 连接两个字符串或数字
*
*    int n = Conn(123,456); 结果就是n=123456;
*
*    char* str = Conn(“asdf”, “adf”); 结果就是 str = “asdfadf”;
*
*    #define ToChar(x) #@x
*
*    给x加上单引号，结果返回是一个const char。举例说：
*    char a = ToChar(1);结果就是a=‘1’;
*    #define ToString(x) #x
*
*    给x加双引号(数字变字符串)
*    char* str = ToString(123132);就成了str=“123132”;
*
**/

// extern dreamer::Logger::ptr g_logger;
static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();
namespace dreamer {


static thread_local bool t_hook_enable = false;
static dreamer::ConfigVar<int>::ptr g_tcp_connect_timeout = 
            DREAMER_ROOT_CONFIG()->look_up("tcp.connect.timeout", "tcp connect timeout", 5000);


bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

bool hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return true;
    }
// sleep_f = (sleep_fun)sleep    
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
    return true;
}

static uint64_t s_connect_timeout = -1;
struct _HookIniter {
    _HookIniter() {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->get_value();

        g_tcp_connect_timeout->add_listener(1, [](const int& old_value, const int& new_value){
                D_SLOG_INFO(g_logger) << "tcp connect timeout changed from "
                                         << old_value << " to " << new_value;
                s_connect_timeout = new_value;
        });
    }
};

static _HookIniter s_hook_initer;

}

// 定时器条件的状态 定时器是否被关闭
struct timer_info {
    int cancelled = 0;
};

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
        uint32_t event, int timeout_so, Args&&... args) {
    if(!dreamer::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }

    dreamer::FdCtx::ptr ctx = dreamer::FdMgr::getInstance()->get(fd);
    // 如果fd不存在，直接调用原函数
    if(!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if(ctx->isClose()) {
        errno = EBADF;
        return -1;
    }
    // fd 不是socket 或者 用户设置了非阻塞
    if(!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    // 根据超时类型获获取超时时间
    uint64_t to = ctx->getTimeout(timeout_so);
    // Timer的条件变量
    std::shared_ptr<timer_info> tinfo = std::make_shared<timer_info>();

retry:
    // 尝试执行io
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    // 触发中断
    while(n == -1 && errno == EINTR) {
        n = fun(fd, std::forward<Args>(args)...);
    }
    // 资源暂时还未就绪 将io放入任务队列中
    if(n == -1 && errno == EAGAIN) {
        dreamer::IOManager* iom = dreamer::IOManager::GetThis();
        dreamer::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);
        // 设置了超时时间
        if(to != (uint64_t)-1) {
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                auto t = winfo.lock();
                // 超时后条件变量消失或者取消
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                // 超时后直接执行event
                iom->cancelEvent(fd, (dreamer::IOManager::Event)(event));
            }, winfo);
        }

        // 向epoll_wait 注册事件 当callback为null时， 注册的回调为当前的协程
        int rt = iom->addEvent(fd, (dreamer::IOManager::Event)(event));
        if(DREAMER_UNLIKELY(rt)) {
            // 如果时间注册失败 取消定时器，返回-1
            D_SLOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            if(timer) {
                timer->cancel();
            }
            return -1;
        } else {
            // 让出时间片 等待epoll唤醒 或者通过cancelEvent超时唤醒
            D_SLOG_DEBUG(g_logger) << "让出时间片 等待epoll唤醒 或者通过cancelEvent超时唤醒";
            dreamer::Fiber::YieldToHold();
            D_SLOG_DEBUG(g_logger) << "wake up";
            if(timer) {
                timer->cancel();
            }
            // 已经超时
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    
    return n;
}


extern "C" {
// declaration funciton pointer
// sleep_fun sleep_f = nullptr;
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if(!dreamer::t_hook_enable) {
        return sleep_f(seconds);
    }

    dreamer::Fiber::ptr fiber = dreamer::Fiber::GetThis();
    dreamer::IOManager* iom = dreamer::IOManager::GetThis();
    iom->addTimer(seconds * 1000, std::bind((void(dreamer::Scheduler::*)
            (dreamer::Fiber::ptr, int thread))&dreamer::IOManager::schedule
            ,iom, fiber, -1));
    dreamer::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if(!dreamer::t_hook_enable) {
        return usleep_f(usec);
    }
    dreamer::Fiber::ptr fiber = dreamer::Fiber::GetThis();
    dreamer::IOManager* iom = dreamer::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(dreamer::Scheduler::*)
            (dreamer::Fiber::ptr, int thread))&dreamer::IOManager::schedule
            ,iom, fiber, -1));
    dreamer::Fiber::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!dreamer::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;
    dreamer::Fiber::ptr fiber = dreamer::Fiber::GetThis();
    dreamer::IOManager* iom = dreamer::IOManager::GetThis();
    iom->addTimer(timeout_ms, std::bind((void(dreamer::Scheduler::*)
            (dreamer::Fiber::ptr, int thread))&dreamer::IOManager::schedule
            ,iom, fiber, -1));
    dreamer::Fiber::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if(!dreamer::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        return fd;
    }
    dreamer::FdMgr::getInstance()->get(fd, true);
    return fd;
}

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) {
    if(!dreamer::t_hook_enable) {
        struct timeval tv {int(timeout_ms / 1000), int(timeout_ms % 1000 * 1000)};
        socklen_t optlen = sizeof(tv);
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, optlen);
        return connect_f(fd, addr, addrlen);
    }
    dreamer::FdCtx::ptr ctx = dreamer::FdMgr::getInstance()->get(fd);
    if(!ctx || ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket()) {
        return connect_f(fd, addr, addrlen);
    }

    if(ctx->getUserNonblock()) {
        return connect_f(fd, addr, addrlen);
    }

    int n = connect_f(fd, addr, addrlen);
    if(n == 0) {
        return 0;
    } else if(n != -1 || errno != EINPROGRESS) {
        return n;
    }

    dreamer::IOManager* iom = dreamer::IOManager::GetThis();
    dreamer::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo = std::make_shared<timer_info>();
    std::weak_ptr<timer_info> winfo(tinfo);

    if(timeout_ms != (uint64_t)-1) {
        timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, dreamer::IOManager::WRITE);
        }, winfo);
    }

    int rt = iom->addEvent(fd, dreamer::IOManager::WRITE);
    if(rt == 0) {
        dreamer::Fiber::YieldToHold();
        if(timer) {
            timer->cancel();
        }
        if(tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    } else {
        if(timer) {
            timer->cancel();
        }
        D_SLOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, dreamer::s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = do_io(s, accept_f, "accept", dreamer::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {
        dreamer::FdMgr::getInstance()->get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    static const bool _dreamer_hook_init_ = dreamer::hook_init();
    (void)_dreamer_hook_init_;
    return do_io(fd, read_f, "read", dreamer::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", dreamer::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", dreamer::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", dreamer::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", dreamer::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", dreamer::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", dreamer::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", dreamer::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", dreamer::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", dreamer::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd) {
    if(!dreamer::t_hook_enable) {
        return close_f(fd);
    }

    dreamer::FdCtx::ptr ctx = dreamer::FdMgr::getInstance()->get(fd);
    if(ctx) {
        auto iom = dreamer::IOManager::GetThis();
        if(iom) {
            iom->cancelAll(fd);
        }
        dreamer::FdMgr::getInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ ) {
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                dreamer::FdCtx::ptr ctx = dreamer::FdMgr::getInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                dreamer::FdCtx::ptr ctx = dreamer::FdMgr::getInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if(FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg;
        dreamer::FdCtx::ptr ctx = dreamer::FdMgr::getInstance()->get(d);
        if(!ctx || ctx->isClose() || !ctx->isSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if(!dreamer::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            dreamer::FdCtx::ptr ctx = dreamer::FdMgr::getInstance()->get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

}