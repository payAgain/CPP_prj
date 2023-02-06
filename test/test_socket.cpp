#include "d_address.h"
#include "d_socket.h"
#include "log.h"
#include "io_manager.h"
#include "fd_manager.h"
#include "utils.h"
#include "string.h"

static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

void test_socket() {
    auto addr = dreamer::IPv4Address::Create("127.0.0.1");
    D_SLOG_INFO(g_logger) << addr->toString();
    auto sock = dreamer::Socket::CreateTCP(addr);
    auto fd = dreamer::FdMgr::getInstance()->get(sock->getSocket());
    D_SLOG_INFO(g_logger) << sock->toString();
}
void test_socket1() {
    //std::vector<dreamer::Address::ptr> addrs;
    //dreamer::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //dreamer::IPAddress::ptr addr;
    //for(auto& i : addrs) {
    //    D_SLOG_INFO(g_logger) << i->toString();
    //    addr = std::dynamic_pointer_cast<dreamer::IPAddress>(i);
    //    if(addr) {
    //        break;
    //    }
    //}
    dreamer::IPAddress::ptr addr = dreamer::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr) {
        D_SLOG_INFO(g_logger) << "get address: " << addr->toString();
    } else {
        D_SLOG_ERROR(g_logger) << "get address fail";
        return;
    }

    dreamer::Socket::ptr sock = dreamer::Socket::CreateTCP(addr);
    addr->setPort(80);
    D_SLOG_INFO(g_logger) << "addr=" << addr->toString();
    if(!sock->connect(addr)) {
        D_SLOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        D_SLOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
    }

    auto fdctx = dreamer::FdMgr::getInstance()->get(sock->getSocket());
    if (!fdctx) {
        D_SLOG_ERROR(g_logger) << "fdctx not created";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        D_SLOG_INFO(g_logger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        D_SLOG_INFO(g_logger) << "recv fail rt=" << rt << " errno" << strerror(errno);
        return;
    }

    buffs.resize(rt);
    D_SLOG_INFO(g_logger) << buffs;
}

void test2() {
    dreamer::IPAddress::ptr addr = dreamer::Address::LookupAnyIPAddress("www.baidu.com:80");
    if(addr) {
        D_SLOG_INFO(g_logger) << "get address: " << addr->toString();
    } else {
        D_SLOG_ERROR(g_logger) << "get address fail";
        return;
    }

    dreamer::Socket::ptr sock = dreamer::Socket::CreateTCP(addr);
    if(!sock->connect(addr)) {
        D_SLOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        D_SLOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
    }

    uint64_t ts = dreamer::GetCurrentUS();
    for(size_t i = 0; i < 10000000000ul; ++i) {
        if(int err = sock->getError()) {
            D_SLOG_INFO(g_logger) << "err=" << err << " errstr=" << strerror(err);
            break;
        }

        //struct tcp_info tcp_info;
        //if(!sock->getOption(IPPROTO_TCP, TCP_INFO, tcp_info)) {
        //    D_SLOG_INFO(g_logger) << "err";
        //    break;
        //}
        //if(tcp_info.tcpi_state != TCP_ESTABLISHED) {
        //    D_SLOG_INFO(g_logger)
        //            << " state=" << (int)tcp_info.tcpi_state;
        //    break;
        //}
        static int batch = 10000000;
        if(i && (i % batch) == 0) {
            uint64_t ts2 = dreamer::GetCurrentUS();
            D_SLOG_INFO(g_logger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " us";
            ts = ts2;
        }
    }
}

int main() {
    dreamer::IOManager iom(1);
    iom.schedule(test_socket1);
}