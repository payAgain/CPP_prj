#include "io_manager.h"
#include "log.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include "string.h"

static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();
int sock;

void test_fiber() {
    D_SLOG_INFO(g_logger) << "test fiber";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "39.156.66.10", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        D_SLOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        dreamer::IOManager::GetThis()->addEvent(sock, dreamer::IOManager::READ, [](){
            D_SLOG_INFO(g_logger) << "read callback";
        });
        dreamer::IOManager::GetThis()->addEvent(sock, dreamer::IOManager::WRITE, [](){
            D_SLOG_INFO(g_logger) << "write callback";
            //close(sock);
            dreamer::IOManager::GetThis()->cancelEvent(sock, dreamer::IOManager::READ);
            close(sock);
        });
    } else {
        D_SLOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}

dreamer::Timer::ptr s_timer;
void test_timer() {
    dreamer::IOManager iom(2);
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        D_SLOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 3) {
            s_timer->reset(2000, true);
            s_timer->cancel();
        }
    }, true);
}

void test1() {
    dreamer::IOManager iom{2};
    iom.schedule(test_fiber);
    iom.stop();
}

int main() {
    test_timer();
}