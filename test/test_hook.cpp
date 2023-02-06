#include "d_hook.h"
#include "log.h"
#include "io_manager.h"
#include "string.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

void test_sleep() {
    dreamer::IOManager iom(1);
    iom.schedule([](){
        sleep(2);
        D_SLOG_INFO(g_logger) << "sleep 2";
    });

    iom.schedule([](){
        sleep(3);
        D_SLOG_INFO(g_logger) << "sleep 3";
    });
    D_SLOG_INFO(g_logger) << "test_sleep";
}
void test_sock() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "110.242.68.66", &addr.sin_addr.s_addr);

    D_SLOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    D_SLOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

    if(rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    D_SLOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    D_SLOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    buff.resize(rt);
    D_SLOG_INFO(g_logger) << buff;
}
int main() {
    // test_sleep();
    dreamer::IOManager iom;
    iom.schedule(test_sock);
}