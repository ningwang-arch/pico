#ifndef __PICO_ADDRESS_H__
#define __PICO_ADDRESS_H__

#include <arpa/inet.h>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <vector>

namespace pico {

class IPAddress;
// base class for all addresses
class Address
{
public:
    typedef std::shared_ptr<Address> Ptr;

    static Address::Ptr Create(const sockaddr* addr, socklen_t addrlen);

    virtual ~Address(){};
    virtual std::string to_string() const = 0;
    bool operator==(const Address& other);

    // returns the address family
    virtual int getFamily() const;

    // returns the address length
    virtual int getAddrLen() const = 0;

    // returns the address in network byte order
    virtual const sockaddr* getAddr() const = 0;
    virtual sockaddr* getAddr() = 0;

    static Address::Ptr LookupAny(const std::string& host, int family = AF_INET,
                                  int socktype = SOCK_STREAM, int protocol = 0);
    static bool Lookup(const std::string& host, std::vector<Address::Ptr>& addrs,
                       int family = AF_UNSPEC, int socktype = SOCK_STREAM, int protocol = 0);
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
                                                         int family = AF_INET,
                                                         int socktype = SOCK_STREAM,
                                                         int protocol = 0);
    static bool getInterfaceAddresses(std::multimap<std::string, Address::Ptr>& addrs,
                                      int family = AF_INET, int socktype = SOCK_STREAM,
                                      int protocol = 0);

    static bool getInterfaceAddresses(std::vector<Address::Ptr>& addrs, const std::string& ifname,
                                      int family = AF_INET, int socktype = SOCK_STREAM,
                                      int protocol = 0);
};

// base class for IPv4/IPv6 addresses
class IPAddress : public Address
{
public:
    typedef std::shared_ptr<IPAddress> Ptr;

    static IPAddress::Ptr Create(const char* addr, uint16_t port = 0);

    virtual ~IPAddress() = default;

    virtual IPAddress::Ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::Ptr networkAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::Ptr subnetMask(uint32_t prefix_len) = 0;

    virtual void setPort(uint16_t port) = 0;
    virtual uint32_t getPort() const = 0;
};

class IPv4Address : public IPAddress
{
public:
    typedef std::shared_ptr<IPv4Address> Ptr;

    explicit IPv4Address(const sockaddr_in& addr);
    IPv4Address(const uint32_t addr = INADDR_ANY, uint16_t port = 0);
    static IPv4Address::Ptr Create(const char* addr, uint16_t port = 0);

    virtual ~IPv4Address() = default;

    virtual std::string to_string() const override;
    virtual IPAddress::Ptr broadcastAddress(uint32_t prefix_len) override;
    virtual IPAddress::Ptr networkAddress(uint32_t prefix_len) override;
    virtual IPAddress::Ptr subnetMask(uint32_t prefix_len) override;

    virtual void setPort(uint16_t port) override;
    virtual uint32_t getPort() const override;

    virtual int getAddrLen() const override;
    virtual const sockaddr* getAddr() const override;
    virtual sockaddr* getAddr() override;

private:
    struct sockaddr_in m_addr;
};

class IPv6Address : public IPAddress
{
public:
    typedef std::shared_ptr<IPv6Address> Ptr;

    IPv6Address();
    explicit IPv6Address(const sockaddr_in6& addr);
    explicit IPv6Address(const uint8_t* addr, uint16_t port = 0);
    static IPv6Address::Ptr Create(const char* addr, uint16_t port = 0);

    virtual ~IPv6Address() = default;

    virtual std::string to_string() const override;
    virtual IPAddress::Ptr broadcastAddress(uint32_t prefix_len) override;
    virtual IPAddress::Ptr networkAddress(uint32_t prefix_len) override;
    virtual IPAddress::Ptr subnetMask(uint32_t prefix_len) override;

    virtual void setPort(uint16_t port) override;
    virtual uint32_t getPort() const override;

    virtual int getAddrLen() const override;
    virtual const sockaddr* getAddr() const override;
    virtual sockaddr* getAddr() override;

private:
    struct sockaddr_in6 m_addr;
};

class UnixAddress : public Address
{
public:
    typedef std::shared_ptr<UnixAddress> Ptr;

    UnixAddress();
    explicit UnixAddress(const std::string& path);

    virtual ~UnixAddress() = default;

    virtual std::string to_string() const override;

    virtual int getAddrLen() const override;
    virtual const sockaddr* getAddr() const override;

    virtual void setPath(const std::string& path);
    virtual std::string getPath() const;

private:
    struct sockaddr_un m_addr;
    socklen_t m_length;
};

class UnknownAddress : public Address
{
public:
    typedef std::shared_ptr<UnknownAddress> Ptr;

    explicit UnknownAddress(int family);

    explicit UnknownAddress(const sockaddr& addr);

    virtual ~UnknownAddress() = default;

    virtual std::string to_string() const override;

    virtual int getAddrLen() const override;
    virtual const sockaddr* getAddr() const override;
    virtual sockaddr* getAddr() override;

private:
    struct sockaddr m_addr;
};
}   // namespace pico

#endif
