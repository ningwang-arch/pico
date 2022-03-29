#include "pico/address.h"
#include "pico/logging.h"

void test() {
    std::vector<pico::Address::Ptr> addrs;
    bool v = pico::Address::Lookup("www.baidu.com", addrs);
    if (v) {
        for (auto& addr : addrs) { LOG_INFO("%s", addr->to_string().c_str()); }
    }
}

void test_iface() {
    std::multimap<std::string, pico::Address::Ptr> addrs;
    bool v = pico::Address::getInterfaceAddresses(addrs, AF_INET);
    if (v) {
        for (auto& addr : addrs) {
            LOG_INFO("%s - %s", addr.first.c_str(), addr.second->to_string().c_str());
        }
    }
}

void test_ipv4() {
    auto addr = pico::IPv4Address::Create("192.168.1.101");
    addr->setPort(80);
    LOG_INFO("%s", addr->to_string().c_str());
}

void test_ipv6() {
    auto addr = pico::IPv6Address::Create("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
    addr->setPort(80);
    LOG_INFO("%s", addr->to_string().c_str());
}

int main(int argc, char const* argv[]) {
    // test();
    // test_iface();
    // test_ipv4();
    test_ipv6();
    return 0;
}
