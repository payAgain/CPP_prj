#include "sys/epoll.h"
#include <cstring>
#include "fcntl.h"
#include <unistd.h>
#include "log.h"
#include "d_thread.h"
#include "boost/beast.hpp"

#define N 20
static dreamer::Logger::ptr logger = DREAMER_SYSTEM_LOGGER();
int fd[2];

void pr() {
    std::string a = "data: loop: ";
    for(int i=0;;i++) {
        a.push_back('c');
        write(fd[1], a.c_str(), a.size());
        sleep(1);
    }
}

int main() {
    if (pipe2(fd, O_NONBLOCK)!=0) {
        D_SLOG_INFO(logger) << "pipe create fail";
        return -1;
    }
    dreamer::Thread t(pr, "pr");
    int epfd;
    epfd = epoll_create(N);
    epoll_event ev, evs[N];
    memset(&ev, 0, sizeof (ev));
    ev.events = EPOLLIN;
    ev.data.ptr = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd[0], &ev);
    for(;;) {
        int rt = epoll_wait(epfd, evs, N, 0);
        for(int i=0;i<rt;i++) {
            if (evs[i].data.ptr == fd) {
                char a[256];
                while(read(fd[0], a, 256) > 0) D_SLOG_INFO(logger) << "recive data:" << a;
            }
        }
    }
}