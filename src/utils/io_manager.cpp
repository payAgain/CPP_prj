//
// Created by YiMing D on 2023/1/27.
//

#include "io_manager.h"
#include "sys/epoll.h"
#include "string.h"
#include "d_exception.h"
#include "fcntl.h"

namespace dreamer {

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




}
