#include "d_address.h"
#include "log.h"

static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();
void test_iface() {
    std::multimap<std::string, std::pair<dreamer::Address::ptr, uint32_t> > results;

    bool v = dreamer::Address::GetInterfaceAddresses(results);
    if(!v) {
        D_SLOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i: results) {
        D_SLOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}
void test() {
    std::vector<dreamer::Address::ptr> addrs;

    D_SLOG_INFO(g_logger) << "begin";
    bool v = dreamer::Address::Lookup(addrs, "localhost:3080");
    //bool v = dreamer::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    D_SLOG_INFO(g_logger) << "end";
    if(!v) {
        D_SLOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        D_SLOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }

    auto addr = dreamer::Address::LookupAny("localhost:4080");
    if(addr) {
        D_SLOG_INFO(g_logger) << *addr;
    } else {
        D_SLOG_ERROR(g_logger) << "error";
    }
}
void test_ipv4() {
    //auto addr = dreamer::IPAddress::Create("www.dreamer.top");
    auto addr = dreamer::IPAddress::Create("127.0.0.255");
    if(addr) {
        D_SLOG_INFO(g_logger) << addr->toString();
        D_SLOG_INFO(g_logger) << addr->subnetMask(24)->toString();
        D_SLOG_INFO(g_logger) << addr->networdAddress(24)->toString();
    }
}
int main() {
    // test_ipv4();
    // test_iface();
    test();
}