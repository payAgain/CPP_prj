#pragma once

#include "../d_address.h"
#include "../d_socket.h"
#include "../io_manager.h"
#include "../nocopyable.h"
#include <cstdint>
#include <memory>
#include <ostream>

namespace dreamer {
class MServer : NoCopyable {
public:
  typedef std::shared_ptr<MServer> ptr;
  MServer(std::string name, uint64_t lc, uint64_t io, uint64_t hd);
  MServer() = default;
  virtual ~MServer();

  std::string dumpStatus();
  void bind(dreamer::Address::ptr addr);
  void start();
  void stop();
  bool checkStopping() const { return m_stopping; }

protected:
  virtual void handleClinet(Socket::ptr client);
  virtual void accpet(Socket::ptr sk);

private:
  std::vector<Socket::ptr> m_socks;

  IOManager::ptr m_io;
  IOManager::ptr m_handler;
  IOManager::ptr m_listen;

  uint64_t m_listenCount = 1;
  uint64_t m_ioCount = 1;
  uint64_t m_handleCount = 2;

  uint64_t m_clientTimeOut = -1;
  std::string m_name = "Default";
  std::string m_type = "TCP";
  bool m_stopped = false;
  bool m_stopping = false;
};

inline std::ostream &operator<<(std::ostream &os, MServer::ptr ms) {
  return os << ms->dumpStatus();
}

} // namespace dreamer