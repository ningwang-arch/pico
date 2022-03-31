/**
 * 对socket进行再次封装
 */

#ifndef __PICO_SOCKET_H__
#define __PICO_SOCKET_H__

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "address.h"
#include "noncopyable.h"

namespace pico {
class Socket : Noncopyable
{
public:
    typedef std::shared_ptr<Socket> Ptr;


    static Socket::Ptr CreateTcp(Address::Ptr addr);
    static Socket::Ptr CreateUdp(Address::Ptr addr);

    static Socket::Ptr CreateTcpSocket();
    static Socket::Ptr CreateUdpSocket();
    static Socket::Ptr CreateTcpScoketv6();
    static Socket::Ptr CreateUdpSocketv6();


    enum Type
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };
    enum Family
    {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX,
    };


    Socket(int family, int type, int protocol = 0);
    ~Socket();

    /**
     * 获取发送超时时间
     */
    uint64_t getSendTimeout() const;
    /**
     * 设置发送超时时间
     * @param timeout
     * @return
     */
    void setSendTimeout(uint64_t timeout);

    /**
     * 获取接收超时时间
     */
    uint64_t getRecvTimeout() const;
    /**
     * 设置接收超时时间
     * @param timeout
     * @return
     */
    void setRecvTimeout(uint64_t timeout);

    int getopt(int level, int optname, void* optval, socklen_t* optlen);

    int setopt(int level, int optname, const void* optval, socklen_t optlen);

    template<typename T>
    int setopt(int level, int optname, const T& optval) {
        return setopt(level, optname, &optval, sizeof(T));
    }

    template<typename T>
    int getopt(int level, int optname, T& optval) {
        return getopt(level, optname, &optval, sizeof(T));
    }

    bool connect(const Address::Ptr& addr, uint64_t timeout = -1);
    bool reconnect(uint64_t timeout = -1);

    bool bind(const Address::Ptr& addr);

    bool listen(int backlog = 5);

    Socket::Ptr accept();

    int send(const void* buf, size_t len, int flags = 0);
    int send(const iovec* iov, int iovcnt, int flags = 0);

    int recv(void* buf, size_t len, int flags = 0);
    int recv(iovec* iov, int iovcnt, int flags = 0);

    int sendto(const void* buf, size_t len, const Address::Ptr& addr, int flags = 0);
    int sendto(const iovec* iov, int iovcnt, const Address::Ptr& addr, int flags = 0);

    int recvfrom(void* buf, size_t len, Address::Ptr& addr, int flags = 0);
    int recvfrom(iovec* iov, int iovcnt, Address::Ptr& addr, int flags = 0);

    bool close();

    int shutdown(int how);

    std::string to_string() const;
    int getSocket() const { return m_sockfd; };
    bool cancelRead();
    bool cancelWrite();
    bool cancelAll();

    Address::Ptr getLocalAddress();
    Address::Ptr getPeerAddress();

    // protected:
    bool init(int sock);
    void newSock();
    void initSock();

private:
    int m_sockfd;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_is_connected;

    Address::Ptr m_local_addr;
    Address::Ptr m_peer_addr;
};
}   // namespace pico


#endif