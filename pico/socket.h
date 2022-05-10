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
#include <openssl/ssl.h>
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

    bool isConnected() const { return m_is_connected; }

    virtual bool connect(const Address::Ptr& addr, uint64_t timeout = -1);
    virtual bool reconnect(uint64_t timeout = -1);

    virtual bool bind(const Address::Ptr& addr);

    virtual bool listen(int backlog = 5);

    virtual Socket::Ptr accept();

    virtual int send(const void* buf, size_t len, int flags = 0);
    virtual int send(const iovec* iov, int iovcnt, int flags = 0);

    virtual int recv(void* buf, size_t len, int flags = 0);
    virtual int recv(iovec* iov, int iovcnt, int flags = 0);

    virtual int sendto(const void* buf, size_t len, const Address::Ptr& addr, int flags = 0);
    virtual int sendto(const iovec* iov, int iovcnt, const Address::Ptr& addr, int flags = 0);

    virtual int recvfrom(void* buf, size_t len, Address::Ptr& addr, int flags = 0);
    virtual int recvfrom(iovec* iov, int iovcnt, Address::Ptr& addr, int flags = 0);

    virtual bool close();

    virtual int shutdown(int how);

    virtual std::string to_string() const;
    int getSocket() const { return m_sockfd; };
    bool cancelRead();
    bool cancelWrite();
    bool cancelAll();

    Address::Ptr getLocalAddress();
    Address::Ptr getPeerAddress();

protected:
    virtual bool init(int sock);
    void newSock();
    void initSock();

protected:
    int m_sockfd;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_is_connected;

    Address::Ptr m_local_addr;
    Address::Ptr m_peer_addr;
};

class SSLSocket : public Socket
{
public:
    typedef std::shared_ptr<SSLSocket> Ptr;
    static SSLSocket::Ptr CreateTcp(Address::Ptr addr);
    static SSLSocket::Ptr CreateTcpSocket();
    static SSLSocket::Ptr CreateTcpScoketv6();

    SSLSocket(int family, int type, int protocol = 0);

    virtual bool connect(const Address::Ptr& addr, uint64_t timeout = -1) override;


    virtual bool bind(const Address::Ptr& addr) override;

    virtual bool listen(int backlog = 5) override;

    virtual Socket::Ptr accept() override;

    virtual int send(const void* buf, size_t len, int flags = 0) override;
    virtual int send(const iovec* iov, int iovcnt, int flags = 0) override;

    virtual int recv(void* buf, size_t len, int flags = 0) override;
    virtual int recv(iovec* iov, int iovcnt, int flags = 0) override;


    virtual bool close() override;

    virtual int shutdown(int how) override;

    virtual std::string to_string() const override;

    bool loadCertificate(const std::string& cert_file, const std::string& key_file);

protected:
    virtual bool init(int sock) override;

private:
    std::shared_ptr<SSL_CTX> m_ctx;
    std::shared_ptr<SSL> m_ssl;
};

}   // namespace pico


#endif