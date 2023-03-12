#include "basic_log.h"
#include "d_socket.h"
#include "log.h"
#include "server/d_tcp_server.h"
#include <cstring>
#include <memory>
#include <sstream>

namespace dreamer {
static Logger::ptr logger = DREAMER_SYSTEM_LOGGER();
MServer::MServer(std::string name, uint64_t lc, uint64_t io, uint64_t hd)
    : m_name(name), m_listenCount(lc), m_ioCount(io), m_handleCount(hd) {}
MServer::~MServer() {
  D_SLOG_INFO(logger) << "MServer: " << m_name << " try to destroy"; 
  if (!m_stopped) {
    stop();
  }
  for (auto &t : m_socks)
    t->close();
}
std::string MServer::dumpStatus() {
  std::stringstream ss;
  ss << "Server Name: " << m_name << "\nType :" << m_type
     << "\n IO thread: " << m_ioCount << "\nhandler thread:" << m_handleCount
     << "\nlisten thread: " << m_listenCount;
  return ss.str();
}
void MServer::bind(dreamer::Address::ptr addr) {
  auto socket = Socket::CreateTCPSocket();
  socket->bind(addr);
  m_socks.emplace_back(socket);
}
void MServer::start() {
  if (!m_listen) {
    m_listen = std::make_shared<IOManager>(m_listenCount, false,
                                           std::string(m_name + "listen"));
    m_handler = std::make_shared<IOManager>(m_handleCount, false,
                                            std::string(m_name + "handler"));
    // m_io = std::make_shared<IOManager>(m_ioCount, false, m_name + "io");
  }
  for (auto &sk : m_socks) {
    m_listen->schedule([this, sk] { this->accpet(sk); });
  }
}
void MServer::stop() {
  m_stopping = true;
  //   m_io->stop();
  m_handler->stop();
  m_listen->stop();
  m_stopped = true;
}

void MServer::handleClinet(Socket::ptr client) {
//   while (client->checkConnected()) {
    char buf[64];
    D_SLOG_INFO(logger) << "read";
    int rt = client->recv(buf, 64);
    D_SLOG_INFO(logger) << "clinet fd" << client->getSocket()
                        << " send msg: " << buf;
    std::string echo = "nihao";
    client->send(echo.c_str(), echo.size());
    D_SLOG_INFO(logger) << "send";
//   }
}
void MServer::accpet(Socket::ptr sk) {
  sk->listen();
  D_SLOG_INFO(logger) << "start listen sk: " << sk;
  auto rt = sk->accept();
  if (rt) {
    D_SLOG_INFO(logger) << "accept client" << rt;
    m_handler->schedule([this, rt] { this->handleClinet(rt); });
  }
}

} // namespace dreamer