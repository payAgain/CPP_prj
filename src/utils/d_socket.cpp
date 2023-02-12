#include "d_socket.h"
#include "io_manager.h"
#include "fd_manager.h"
#include "log.h"
#include "d_exception.h"
#include "d_hook.h"
#include <limits.h>
#include <string.h>


namespace dreamer {

static Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

// static
Socket::ptr Socket::CreateTCP(dreamer::Address::ptr address) {
    return std::make_shared<Socket>(address->getFamily(), TCP, 0);
}

Socket::ptr Socket::CreateUDP(dreamer::Address::ptr address) {
    Socket::ptr sock = std::make_shared<Socket>(address->getFamily(), UDP, 0);
    sock->newSock();
    sock->m_isConnected = true;
    return sock;
}

Socket::ptr Socket::CreateTCPSocket() {
    return std::make_shared<Socket>(IPv4, TCP, 0);
}

Socket::ptr Socket::CreateUDPSocket() {
    Socket::ptr sock = std::make_shared<Socket>(IPv4, UDP, 0);
    sock->newSock();
    sock->m_isConnected = true;
    return sock;
}

Socket::ptr Socket::CreateTCPSocket6() {
    return std::make_shared<Socket>(IPv6, TCP, 0);
}

Socket::ptr Socket::CreateUDPSocket6() {
    Socket::ptr sock = std::make_shared<Socket>(IPv6, UDP, 0);
    sock->newSock();
    sock->m_isConnected = true;
    return sock;
}


// member func
Socket::Socket(int family, int type, int protocol)
    :m_sock(-1)
    ,m_family(family)
    ,m_type(type)
    ,m_protocol(protocol)
    ,m_isConnected(false) {
}

Socket::~Socket() {
    close();
}

bool Socket::checkConnected() {
    if (m_family == TCP) {
        struct tcp_info info;
        int len = sizeof(info);
        getsockopt(m_sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
        m_isConnected = (info.tcpi_state == TCP_ESTABLISHED);
        return m_isConnected;
    }
    D_SLOG_WARN(g_logger) << "Udp is has no connect";
    return false;
}

int64_t Socket::getSendTimeout() {
    FdCtx::ptr ctx = FdMgr::getInstance()->get(m_sock);
    if(ctx) {
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::setSendTimeout(int64_t v) {
    struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
    auto fdctx = FdMgr::getInstance()->get(m_sock);
    if (setOption(SOL_SOCKET, SO_SNDTIMEO, tv)) {
        fdctx->setTimeout(SO_SNDTIMEO, v);
    }
}

int64_t Socket::getRecvTimeout() {
    FdCtx::ptr ctx = FdMgr::getInstance()->get(m_sock);
    if(ctx) {
        return ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::setRecvTimeout(int64_t v) {
    struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
    auto fdctx = FdMgr::getInstance()->get(m_sock);
    if (setOption(SOL_SOCKET, SO_RCVTIMEO, tv)) {
        fdctx->setTimeout(SO_RCVTIMEO, v);
    }
}

bool Socket::getOption(int level, int option, void* result, socklen_t* len) {
    int rt = getsockopt(m_sock, level, option, result, (socklen_t*)len);
    if(rt) {
        D_SLOG_ERROR(g_logger) << "getOption sock=" << m_sock
            << " level=" << level << " option=" << option
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOption(int level, int option, const void* result, socklen_t len) {
    if(setsockopt(m_sock, level, option, result, (socklen_t)len)) {
        D_SLOG_ERROR(g_logger) << "setOption sock=" << m_sock
            << " level=" << level << " option=" << option
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept() {
    Socket::ptr sock = std::make_shared<Socket>(m_family, m_type, m_protocol);
    int newsock = ::accept(m_sock, nullptr, nullptr);
    if(newsock == -1) {
        D_SLOG_ERROR(g_logger) << "accept(" << m_sock << ") errno="
            << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    if(sock->init(newsock)) {
        return sock;
    }
    return nullptr;
}

bool Socket::init(int sock) {
    FdCtx::ptr ctx = FdMgr::getInstance()->get(sock);
    if(ctx && ctx->isSocket() && !ctx->isClose()) {
        m_sock = sock;
        m_isConnected = true;
        initSock();
        getLocalAddress();
        getRemoteAddress();
        return true;
    }
    return false;
}

bool Socket::bind(const Address::ptr addr) {
    //m_localAddress = addr;
    if(!isValid()) {
        newSock();
        if(DREAMER_UNLIKELY(!isValid())) {
            return false;
        }
    }

    if(DREAMER_UNLIKELY(addr->getFamily() != m_family)) {
        D_SLOG_ERROR(g_logger) << "bind sock.family("
            << m_family << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    // UnixAddress::ptr uaddr = std::dynamic_pointer_cast<UnixAddress>(addr);
    // if(uaddr) {
    //     Socket::ptr sock = Socket::CreateUnixTCPSocket();
    //     if(sock->connect(uaddr)) {
    //         return false;
    //     } else {
    //         dreamer::FSUtil::Unlink(uaddr->getPath(), true);
    //     }
    // }

    if(::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
        D_SLOG_ERROR(g_logger) << "bind error errrno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }
    getLocalAddress();
    return true;
}

bool Socket::reconnect(uint64_t timeout_ms) {
    if(!m_remoteAddress) {
        D_SLOG_ERROR(g_logger) << "reconnect m_remoteAddress is null";
        return false;
    }
    m_localAddress.reset();
    return connect(m_remoteAddress, timeout_ms);
}

bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms) {
    m_remoteAddress = addr;
    if(!isValid()) {
        newSock();
        if(DREAMER_UNLIKELY(!isValid())) {
            return false;
        }
    }

    if(DREAMER_UNLIKELY(addr->getFamily() != m_family)) {
        D_SLOG_ERROR(g_logger) << "connect sock.family("
            << m_family << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    if(timeout_ms == (uint64_t)-1) {
        if(::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
            D_SLOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                << ") error errno=" << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    } else {
        if(::connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(), timeout_ms)) {
            D_SLOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                << ") timeout=" << timeout_ms << " error errno="
                << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    }
    m_isConnected = true;
    getRemoteAddress();
    getLocalAddress();
    return true;
}

bool Socket::listen(int backlog) {
    if(!isValid()) {
        D_SLOG_ERROR(g_logger) << "listen error sock=-1";
        return false;
    }
    if(::listen(m_sock, backlog)) {
        D_SLOG_ERROR(g_logger) << "listen error errno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::close() {
    if(!m_isConnected && m_sock == -1) {
        return true;
    }
    m_isConnected = false;
    if(m_sock != -1) {
        ::close(m_sock);
        m_sock = -1;
    }
    return true;
}

int Socket::send(const void* buffer, size_t length, int flags) {
    if(isConnected()) {
        return ::send(m_sock, buffer, length, flags);
    }
    return -1;
}

int Socket::send(const iovec* buffers, size_t length, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::sendTo(const void* buffer, size_t length, const Address::ptr to, int flags) {
    if(!isValid()) {
        newSock();
        if (DREAMER_UNLIKELY(!isValid())) {
            return -1;
        }
    }
    return ::sendto(m_sock, buffer, length, flags, to->getAddr(), to->getAddrLen());
}

int Socket::sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags) {
    if(!isValid()) {
        newSock();
        if (DREAMER_UNLIKELY(!isValid())) {
            return -1;
        }
    }
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    msg.msg_name = to->getAddr();
    msg.msg_namelen = to->getAddrLen();
    return ::sendmsg(m_sock, &msg, flags);
}

int Socket::recv(void* buffer, size_t length, int flags) {
    if(isConnected()) {
        return ::recv(m_sock, buffer, length, flags);
    }
    return -1;
}

int Socket::recv(iovec* buffers, size_t length, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recvFrom(void* buffer, size_t length, Address::ptr from, int flags) {
    if(!isValid()) {
        newSock();
        if (DREAMER_UNLIKELY(!isValid())) {
            return -1;
        }
    }
    socklen_t len = from->getAddrLen();
    return ::recvfrom(m_sock, buffer, length, flags, from->getAddr(), &len);
}

int Socket::recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags) {
    if(!isValid()) {
        newSock();
        if (DREAMER_UNLIKELY(!isValid())) {
            return -1;
        }
    }
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    msg.msg_name = from->getAddr();
    msg.msg_namelen = from->getAddrLen();
    return ::recvmsg(m_sock, &msg, flags);
}

Address::ptr Socket::getRemoteAddress() {
    if(m_remoteAddress) {
        return m_remoteAddress;
    }

    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result = std::make_shared<IPv4Address>();
            break;
        case AF_INET6:
            result = std::make_shared<IPv6Address>();
            break;
        // case AF_UNIX:
        //     result = std::make_shared<UnixAddress>();
        //     break;
        default:
            result = nullptr;
            break;
    }
    socklen_t addrlen = result->getAddrLen();
    if(getpeername(m_sock, result->getAddr(), &addrlen)) {
        D_SLOG_ERROR(g_logger) << "getpeername error sock=" << m_sock
           << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    // if(m_family == AF_UNIX) {
    //     UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
    //     addr->setAddrLen(addrlen);
    // }
    m_remoteAddress = result;
    return m_remoteAddress;
}

Address::ptr Socket::getLocalAddress() {
    if(m_localAddress) {
        return m_localAddress;
    }

    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result = std::make_shared<IPv4Address>();
            break;
        case AF_INET6:
            result = std::make_shared<IPv6Address>();
            break;
        // case AF_UNIX:
        //     result = std::make_shared<UnixAddress>();
        //     break;
        default:
            result = nullptr;
            break;
    }
    socklen_t addrlen = result->getAddrLen();
    if(getsockname(m_sock, result->getAddr(), &addrlen)) {
        D_SLOG_ERROR(g_logger) << "getsockname error sock=" << m_sock
            << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    // if(m_family == AF_UNIX) {
    //     UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
    //     addr->setAddrLen(addrlen);
    // }
    m_localAddress = result;
    return m_localAddress;
}

bool Socket::isValid() const {
    return m_sock != -1;
}

int Socket::getError() {
    int error = 0;
    socklen_t len = sizeof(error);
    if(!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
        error = errno;
    }
    return error;
}

std::ostream& Socket::dump(std::ostream& os) const {
    os << "[Socket sock=" << m_sock
       << " is_connected=" << m_isConnected
       << " family=" << m_family
       << " type=" << m_type
       << " protocol=" << m_protocol;
    if(m_localAddress) {
        os << " local_address=" << m_localAddress->toString();
    }
    if(m_remoteAddress) {
        os << " remote_address=" << m_remoteAddress->toString();
    }
    os << "]";
    return os;
}

std::string Socket::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

bool Socket::cancelRead() {
    return IOManager::GetThis()->cancelEvent(m_sock, dreamer::IOManager::READ);
}

bool Socket::cancelWrite() {
    return IOManager::GetThis()->cancelEvent(m_sock, dreamer::IOManager::WRITE);
}

bool Socket::cancelAccept() {
    return IOManager::GetThis()->cancelEvent(m_sock, dreamer::IOManager::READ);
}

bool Socket::cancelAll() {
    return IOManager::GetThis()->cancelAll(m_sock);
}

void Socket::initSock() {
    int val = 1;
    setOption(SOL_SOCKET, SO_REUSEADDR, val);
    if(m_type == SOCK_STREAM) {
        setOption(IPPROTO_TCP, TCP_NODELAY, val);
    }
}

void Socket::newSock() {
    m_sock = socket(m_family, m_type, m_protocol);
    if(DREAMER_LIKELY(m_sock != -1)) {
        initSock();
    } else {
        D_SLOG_ERROR(g_logger) << "socket(" << m_family
            << ", " << m_type << ", " << m_protocol << ") errno="
            << errno << " errstr=" << strerror(errno);
    }
}

std::ostream& operator<<(std::ostream& os, const Socket& sock) {
    return os << sock.toString();
}

}