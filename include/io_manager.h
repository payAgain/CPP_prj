//
// Created by YiMing D on 2023/1/27.
//

#ifndef DREAMER_IO_MANAGER_H
#define DREAMER_IO_MANAGER_H

#include "atomic"
#include "d_lock.h"
#include "d_timer.h"
#include "scheduler.h"

namespace dreamer {

class IOManager : public Scheduler, public TimerManager {
public:
  typedef std::shared_ptr<IOManager> ptr;
  typedef RWMutex RWMutexType;

  // The value is actual EPOLL_EVENTS
  enum Event {
    /// 无事件
    NONE = 0x0,
    /// 读事件 (EPOLLIN)
    READ = 0x1,
    /// 写事件 (EPOLLOUT)
    WRITE = 0x4,
  };

private:
  // 事件上下文类
  struct FdContext {
    typedef Mutex MutexType;
    //        typedef Spinlock MutexType;
    // 事件上下文
    struct EventContext {
      /// 事件执行的调度器
      Scheduler *scheduler = nullptr;
      /// 事件协程
      Fiber::ptr fiber;
      /// 事件的回调函数
      std::function<void()> cb;
      void clear() {
        scheduler = nullptr;
        fiber = nullptr;
        cb = nullptr;
      }
    };
    EventContext &getContext(Event event);
    void resetContext(EventContext &ctx);
    void triggerEvent(Event event);

    /// 读事件上下文
    EventContext read;
    /// 写事件上下文
    EventContext write;
    /// 事件关联的句柄
    int fd = 0;
    /// 当前的事件
    Event events = NONE;
    /// 事件的Mutex
    MutexType mutex;
  };

public:
  /**
   * @brief 构造函数
   * @param[in] threads 线程数量
   * @param[in] use_caller 是否将调用线程包含进去
   * @param[in] name 调度器的名   */
  IOManager(size_t threads = 1, bool use_caller = true,
            const std::string &name = "");

  /**
   * @brief 析构函数
   */
  ~IOManager();


  int addEvent(int fd, Event event, std::function<void()> cb = nullptr);


  bool delEvent(int fd, Event event);

  bool cancelEvent(int fd, Event event);

  bool cancelAll(int fd);

  /**
   * @brief 返回当前的IOManager
   */
  static IOManager *GetThis();

protected:
  void tickle() override;
  bool stopping() override;
  void idle() override;
  void onTimerInsertedAtFront() override;

  /**
   * @brief 重置socket句柄上下文的容器大小
   * @param[in] size 容量大小
   */
  void contextResize(size_t size);
  bool stopping(uint64_t &timeout);

private:
  /// epoll 文件句柄
  int m_epfd = 0;
  /// pipe 文件句柄
  int m_tickleFds[2];
  /// 当前等待执行的事件数量
  std::atomic<size_t> m_pendingEventCount = {0};
  /// IOManager的Mutex
  RWMutexType m_mutex;
  /// socket事件上下文的容器 The index is fd num
  std::vector<FdContext *> m_fdContexts;
};
} // namespace dreamer

#endif // DREAMER_IO_MANAGER_H
