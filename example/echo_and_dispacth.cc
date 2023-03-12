#include "log.h"
#include "io_manager.h"
#include "server/d_tcp_server.h"

static dreamer::Logger::ptr logger = DREAMER_SYSTEM_LOGGER();

auto addr = dreamer::Address::LookupAny("0.0.0.0:8080");

int main() {
    dreamer::MServer ms;
    ms.bind(addr);
    ms.start();
    while(!ms.checkStopping()) {sleep(1); }
}