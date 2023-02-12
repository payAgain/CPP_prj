#include "log.h"
#include "d_socket.h"
#include "d_address.h"

static dreamer::Logger::ptr s_logger = DREAMER_SYSTEM_LOGGER();

int main() {
    auto conn_addr = dreamer::Address::LookupAny("127.0.0.1:8080");
    auto conn_sock = dreamer::Socket::CreateTCP(conn_addr);
    if (conn_sock->connect(conn_addr)) {
        conn_sock->send("Hello", 5);
    }
}