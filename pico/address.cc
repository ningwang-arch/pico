#include "address.h"
#include "logging.h"
#include <ifaddrs.h>
#include <iostream>
#include <netdb.h>
#include <string.h>

namespace pico {
int Address::getFamily() const {
    return getAddr()->sa_family;
}

Address::Ptr Address::Create(const sockaddr* addr, socklen_t addrlen) {
    if (!addr) { return nullptr; }
    Address::Ptr ret;

    switch (addr->sa_family) {
    case AF_INET: ret.reset(new IPv4Address(*(const sockaddr_in*)addr)); break;
    case AF_INET6: ret.reset(new IPv6Address(*(const sockaddr_in6*)addr)); break;
    default: ret.reset(new UnknownAddress(*addr)); break;
    }

    return ret;
}

Address::Ptr Address::LookupAny(const std::string& host, int family, int socktype, int protocol) {
    if (host.empty()) { return nullptr; }
    std::vector<Address::Ptr> addrs;
    if (!Lookup(host, addrs, family, socktype, protocol)) { return nullptr; }
    return addrs[0];
}

bool Address::Lookup(const std::string& host, std::vector<Address::Ptr>& addrs, int family,
                     int socktype, int protocol) {
    struct addrinfo hints;
    struct addrinfo* res;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = socktype;
    hints.ai_protocol = protocol;
    hints.ai_flags = AI_ADDRCONFIG;

    std::string node;
    std::string service;
    if (!host.empty() && host[0] == '[') {
        size_t pos = host.find(']');
        if (pos == std::string::npos) { return false; }
        node = host.substr(1, pos - 1);
        service = host.substr(pos + 1);
    }
    else {
        size_t pos = host.find(':');
        if (pos == std::string::npos) { node = host; }
        else {
            node = host.substr(0, pos);
            service = host.substr(pos + 1);
        }
    }

    if (node.empty()) { node = host; }

    ret = getaddrinfo(node.c_str(), service.c_str(), &hints, &res);

    if (ret != 0) { return false; }

    for (struct addrinfo* rp = res; rp != NULL; rp = rp->ai_next) {
        addrs.push_back(Address::Create(rp->ai_addr, (socklen_t)rp->ai_addrlen));
    }

    freeaddrinfo(res);
    return addrs.size() > 0;
}

std::shared_ptr<IPAddress> Address::LookupAnyIPAddress(const std::string& host, int family,
                                                       int socktype, int protocol) {
    if (host.empty()) { return nullptr; }
    std::vector<Address::Ptr> addrs;
    if (!Lookup(host, addrs, family, socktype, protocol)) { return nullptr; }
    for (auto& addr : addrs) {
        std::shared_ptr<IPAddress> ipaddr = std::dynamic_pointer_cast<IPAddress>(addr);
        if (ipaddr) { return ipaddr; }
    }
    return nullptr;
}

bool Address::getInterfaceAddresses(std::multimap<std::string, Address::Ptr>& addrs, int family,
                                    int socktype, int protocol) {
    struct ifaddrs* ifap;
    struct ifaddrs* ifa;
    int ret;

    ret = getifaddrs(&ifap);
    if (ret != 0) { return false; }

    try {
        for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
            Address::Ptr addr;
            if (family != AF_UNSPEC && ifa->ifa_addr->sa_family != family) { continue; }

            switch (ifa->ifa_addr->sa_family) {
            case AF_INET: addr = Create(ifa->ifa_addr, sizeof(struct sockaddr_in)); break;
            case AF_INET6: addr = Create(ifa->ifa_addr, sizeof(struct sockaddr_in6)); break;
            default: break;   // ignore
            };

            if (addr) { addrs.insert(std::make_pair(ifa->ifa_name, addr)); }
        }
    }
    catch (...) {
        LOG_ERROR("getifaddrs failed");
        freeifaddrs(ifap);
        return false;
    }

    freeifaddrs(ifap);
    return addrs.size() > 0;
}

bool Address::getInterfaceAddresses(std::vector<Address::Ptr>& addrs, const std::string& ifname,
                                    int family, int socktype, int protocol) {
    if (ifname.empty() || ifname == "*") {
        if (family == AF_INET || family == AF_UNSPEC) {
            addrs.push_back(std::make_shared<IPv4Address>());
        }
        if (family == AF_INET6 || family == AF_UNSPEC) {
            addrs.push_back(std::make_shared<IPv6Address>());
        }
        return true;
    }
    std::multimap<std::string, Address::Ptr> result;
    if (!getInterfaceAddresses(result, family, socktype, protocol)) { return false; }

    for (auto& it : result) {
        if (it.first == ifname) { addrs.push_back(it.second); }
    }
    return addrs.size() > 0;
}

bool Address::operator==(const Address& other) {
    return getAddrLen() == other.getAddrLen() &&
           memcmp(getAddr(), other.getAddr(), getAddrLen()) == 0;
}

IPAddress::Ptr IPAddress::Create(const char* addr, uint16_t port) {
    addrinfo hints;
    addrinfo* res;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;

    ret = getaddrinfo(addr, NULL, &hints, &res);
    if (ret != 0) {
        LOG_ERROR("getaddrinfo failed: %s", gai_strerror(ret));
        return nullptr;
    }

    try {
        IPAddress::Ptr ptr =
            std::dynamic_pointer_cast<IPAddress>(Address::Create(res->ai_addr, res->ai_addrlen));
        ptr->setPort(port);
        return ptr;
    }
    catch (...) {
        LOG_ERROR("getaddrinfo failed: %s", gai_strerror(ret));
        return nullptr;
    }
}

IPv4Address::IPv4Address(const sockaddr_in& addr)
    : m_addr(addr) {}

IPv4Address::IPv4Address(const uint32_t addr, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = htonl(addr);
    m_addr.sin_port = htons(port);
}

IPv4Address::Ptr IPv4Address::Create(const char* addr, uint16_t port) {
    IPv4Address::Ptr ptr(new IPv4Address);

    ptr->m_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, addr, &ptr->m_addr.sin_addr) != 1) {
        LOG_ERROR("inet_pton failed: %s", addr);
        return nullptr;
    }
    ptr->m_addr.sin_port = htons(port);

    return ptr;
}

std::string IPv4Address::to_string() const {
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf));
    return std::string(buf) + ":" + std::to_string(ntohs(m_addr.sin_port));
}

IPAddress::Ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
    uint32_t mask = 0xffffffff << (32 - prefix_len);
    uint32_t addr = ntohl(m_addr.sin_addr.s_addr);
    uint32_t bcast = addr | ~mask;
    return std::make_shared<IPv4Address>(bcast, 0);
}

IPAddress::Ptr IPv4Address::networkAddress(uint32_t prefix_len) {
    sockaddr_in addr(m_addr);

    addr.sin_addr.s_addr = htonl(ntohl(m_addr.sin_addr.s_addr) & prefix_len);
    return std::make_shared<IPv4Address>(addr);
}

IPAddress::Ptr IPv4Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in addr(m_addr);

    addr.sin_addr.s_addr = htonl(prefix_len == 32 ? 0xffffffff : (0xffffffff << (32 - prefix_len)));
    return std::make_shared<IPv4Address>(addr);
}

void IPv4Address::setPort(uint16_t port) {
    m_addr.sin_port = htons(port);
}

uint32_t IPv4Address::getPort() const {
    return ntohs(m_addr.sin_port);
}

int IPv4Address::getAddrLen() const {
    return sizeof(m_addr);
}

const sockaddr* IPv4Address::getAddr() const {
    return reinterpret_cast<const sockaddr*>(&m_addr);
}

IPv6Address::IPv6Address() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& addr)
    : m_addr(addr) {}

IPv6Address::IPv6Address(const uint8_t* addr, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    memcpy(m_addr.sin6_addr.s6_addr, addr, 16);
    m_addr.sin6_port = htons(port);
}

IPv6Address::Ptr IPv6Address::Create(const char* addr, uint16_t port) {
    IPv6Address::Ptr ptr(new IPv6Address);

    if (inet_pton(AF_INET6, addr, &ptr->m_addr.sin6_addr) != 1) {
        LOG_ERROR("inet_pton failed: %s", addr);
        return nullptr;
    }
    ptr->m_addr.sin6_port = htons(port);

    return ptr;
}

std::string IPv6Address::to_string() const {
    char buf[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &m_addr.sin6_addr, buf, sizeof(buf));
    return "[" + std::string(buf) + "]:" + std::to_string(ntohs(m_addr.sin6_port));
}

IPAddress::Ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
    sockaddr_in6 addr(m_addr);

    if (prefix_len > 128) {
        LOG_ERROR("invalid prefix length: %d", prefix_len);
        return nullptr;
    }

    uint32_t mask = 0xffffffff;
    mask = mask << (32 - prefix_len);
    mask = htonl(mask);

    for (int i = 0; i < 4; i++) {
        addr.sin6_addr.s6_addr32[i] = htonl(ntohl(addr.sin6_addr.s6_addr32[i]) | mask);
    }

    return std::make_shared<IPv6Address>(addr);
}

IPAddress::Ptr IPv6Address::networkAddress(uint32_t prefix_len) {
    sockaddr_in6 addr(m_addr);

    for (int i = 0; i < 16; i++) {
        if (prefix_len > 8) {
            addr.sin6_addr.s6_addr[i] = 0;
            prefix_len -= 8;
        }
        else {
            addr.sin6_addr.s6_addr[i] = addr.sin6_addr.s6_addr[i] & (0xff << (8 - prefix_len));
            prefix_len = 0;
        }
    }
    return std::make_shared<IPv6Address>(addr);
}

IPAddress::Ptr IPv6Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in6 addr(m_addr);

    if (prefix_len > 128) {
        LOG_ERROR("invalid prefix_len: %d", prefix_len);
        return nullptr;
    }

    for (int i = 0; i < 16; i++) {
        if (prefix_len > 8) {
            addr.sin6_addr.s6_addr[i] = 0xff;
            prefix_len -= 8;
        }
        else {
            addr.sin6_addr.s6_addr[i] = 0xff << (8 - prefix_len);
            break;
        }
    }

    return std::make_shared<IPv6Address>(addr);
}

void IPv6Address::setPort(uint16_t port) {
    m_addr.sin6_port = htons(port);
}

uint32_t IPv6Address::getPort() const {
    return ntohs(m_addr.sin6_port);
}

int IPv6Address::getAddrLen() const {
    return sizeof(m_addr);
}

const sockaddr* IPv6Address::getAddr() const {
    return reinterpret_cast<const sockaddr*>(&m_addr);
}
static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;

UnixAddress::UnixAddress() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;

    m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string& path) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;

    if (path.length() > MAX_PATH_LEN) {
        LOG_ERROR("path too long: %s", path.c_str());
        return;
    }

    strncpy(m_addr.sun_path, path.c_str(), MAX_PATH_LEN);
    m_length = offsetof(sockaddr_un, sun_path) + path.length();
}

std::string UnixAddress::to_string() const {
    return std::string(m_addr.sun_path);
}

int UnixAddress::getAddrLen() const {
    return m_length;
}

const sockaddr* UnixAddress::getAddr() const {
    return reinterpret_cast<const sockaddr*>(&m_addr);
}

void UnixAddress::setPath(const std::string& path) {
    if (path.length() > MAX_PATH_LEN) {
        LOG_ERROR("path too long: %s", path.c_str());
        return;
    }

    strncpy(m_addr.sun_path, path.c_str(), MAX_PATH_LEN);
    m_length = offsetof(sockaddr_un, sun_path) + path.length();
}

std::string UnixAddress::getPath() const {
    return std::string(m_addr.sun_path);
}

UnknownAddress::UnknownAddress(int family) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr& addr) {
    memcpy(&m_addr, &addr, sizeof(m_addr));
}

std::string UnknownAddress::to_string() const {
    return std::string("unknown address");
}

int UnknownAddress::getAddrLen() const {
    return sizeof(m_addr);
}

const sockaddr* UnknownAddress::getAddr() const {
    return reinterpret_cast<const sockaddr*>(&m_addr);
}

}   // namespace pico