#include "http/http_server.h"
#include "d_address.h"
#include "io_manager.h"
#include <unistd.h>

void run() {
    dreamer::http::HttpServer::ptr server(new dreamer::http::HttpServer);
    dreamer::Address::ptr addr = dreamer::Address::LookupAny("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    server->start();
}
int main() {
    dreamer::IOManager iom(1);
    iom.schedule(run);
}