#include "basic_log.h"
#include "d_address.h"
#include "d_socket.h"
#include "io_manager.h"
#include "log.h"
#include <string>
#include <unistd.h>

static dreamer::Logger::ptr s_logger = DREAMER_SYSTEM_LOGGER();
int i = 0;
void hello() {
  auto conn_addr = dreamer::Address::LookupAny("127.0.0.1:8080");

  auto conn_sock = dreamer::Socket::CreateTCP(conn_addr);
  D_SLOG_INFO(s_logger) << "try to connect";
  if (conn_sock->connect(conn_addr)) {
    std::string s = "hello" + std::to_string(i);
    conn_sock->send(s.c_str(), s.size());
  }
}

int main() {
  dreamer::IOManager iom(1, true, "client");
  int j = 1;
  while (j--) {
    iom.schedule(hello);
  }
  iom.stop();
}