#include "server/tcp_server.h"
#include "log.h"
#include "io_manager.h"

void run() {
    auto addr = dreamer::Address::LookupAny("0.0.0.0:8080");
    //auto addr2 = dreamer::UnixAddress::ptr(new dreamer::UnixAddress("/tmp/unix_addr"));
    std::vector<dreamer::Address::ptr> addrs;
    addrs.push_back(addr);
    //addrs.push_back(addr2);

    dreamer::TcpServer::ptr tcp_server(new dreamer::TcpServer);
    std::vector<dreamer::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
    
}
int main(int argc, char** argv) {
    dreamer::IOManager iom(1);
    iom.schedule(run);
    return 0;
}
