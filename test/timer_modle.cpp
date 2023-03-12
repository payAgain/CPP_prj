#include "basic_log.h"
#include "d_thread.h"
#include "fcntl.h"
#include "log.h"
#include "queue"
#include "sys/epoll.h"
#include "utils.h"
#include <cstdint>
#include <cstring>
#include <functional>
#include <queue>
#include <unistd.h>
#include <valarray>
#include <vector>
#define N 20
static dreamer::Logger::ptr logger = DREAMER_SYSTEM_LOGGER();

int fd[2];
static int d = 0;
struct Task {
  Task(epoll_event e, uint64_t it, uint64_t stap)
      : ev(e), interval(it), nextStamp(stap + it) {
    id = ++d;
  }
  epoll_event ev;
  uint64_t interval;
  int id;
  uint64_t nextStamp; // last time excute
};
bool operator>(const Task &l, const Task &r) {
  return l.nextStamp > r.nextStamp;
}
std::priority_queue<Task, std::vector<Task>, std::greater<Task>> heap;
uint64_t getNextTime() {
  auto t = heap.top();
  return t.nextStamp;
}
int main() {
  if (pipe2(fd, O_NONBLOCK) != 0) {
    D_SLOG_INFO(logger) << "pipe create fail";
    return -1;
  }
  int epfd;
  epfd = epoll_create(N);
  epoll_event ev, evs[N];
  memset(&ev, 0, sizeof(ev));
  ev.events = EPOLLIN;
  ev.data.ptr = fd;
  epoll_ctl(epfd, EPOLL_CTL_ADD, fd[0], &ev);
  heap.push({ev, 3000, dreamer::GetCurrentMS()});
  heap.push({ev, 5000, dreamer::GetCurrentMS()});
  for (;;) {
    uint64_t time = getNextTime();
    D_SLOG_INFO(logger) << "next interval: " << time - dreamer::GetCurrentMS();
    int rt = epoll_wait(epfd, evs, N, time - dreamer::GetCurrentMS());
    if (rt == 0) {
      while (!heap.empty()) {
        uint64_t t = dreamer::GetCurrentMS();
        auto tsk = heap.top();
        if (tsk.nextStamp <= t) {
          D_SLOG_INFO(logger) << "timer trigger: " << tsk.id;
        } else {
          break;
        }
        heap.pop();
        tsk.nextStamp = t + tsk.interval;
        heap.push(tsk);
      }
    }
  }
}