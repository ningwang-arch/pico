#include "socket.h"
#include "fdmanager.h"
#include "hook.h"
#include "iomanager.h"
#include "logging.h"
#include <iostream>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sstream>

namespace pico {
Socket::Ptr Socket::CreateTcp(Address::Ptr addr) {
    Socket::Ptr sock(new Socket(addr->getFamily(), Socket::TCP, 0));
    return sock;
}

Socket::Ptr Socket::CreateUdp(Address::Ptr addr) {
    Socket::Ptr sock(new Socket(addr->getFamily(), Socket::UDP, 0));
    sock->newSock();
    sock->m_is_connected = true;
    return sock;
}

Socket::Ptr Socket::CreateTcpSocket() {
    Socket::Ptr sock(new Socket(Socket::IPv4, Socket::TCP, 0));
    return sock;
}

Socket::Ptr Socket::CreateUdpSocket() {
    Socket::Ptr sock(new Socket(Socket::IPv4, Socket::UDP, 0));
    sock->newSock();
    sock->m_is_connected = true;
    return sock;
}

Socket::Ptr Socket::CreateTcpScoketv6() {
    Socket::Ptr sock(new Socket(Socket::IPv6, Socket::TCP, 0));
    return sock;
}

Socket::Ptr Socket::CreateUdpSocketv6() {
    Socket::Ptr sock(new Socket(Socket::IPv6, Socket::UDP, 0));
    sock->newSock();
    sock->m_is_connected = true;
    return sock;
}

Socket::Socket(int family, int type, int protocol)
    : m_family(family)
    , m_type(type)
    , m_protocol(protocol) {
    m_sockfd = -1;
    m_is_connected = false;
}

Socket::~Socket() {
    if (m_sockfd != -1) {
        close();
    }
}

bool Socket::init(int sock) {
    FdCtx::Ptr ctx = FdMgr::getInstance()->getFdCtx(sock);
    if (ctx && ctx->isSocket() && !ctx->isClosed()) {
        m_sockfd = sock;
        m_is_connected = true;
        initSock();
        getLocalAddress();
        getPeerAddress();
        return true;
    }
    return false;
}

void Socket::initSock() {
    if (m_sockfd == -1) {
        return;
    }
    int optval = 1;
    setopt(SOL_SOCKET, SO_REUSEADDR, optval);
    setopt(SOL_SOCKET, SO_REUSEPORT, optval);
    setopt(SOL_SOCKET, SO_KEEPALIVE, optval);
    setopt(SOL_SOCKET, SO_LINGER, 0);
    if (m_type == SOCK_STREAM) setopt(IPPROTO_TCP, TCP_NODELAY, optval);
}

void Socket::newSock() {
    if (m_sockfd != -1) {
        return;
    }
    m_sockfd = socket(m_family, m_type, m_protocol);
    if (m_sockfd == -1) {
        LOG_ERROR("socket error: %s", strerror(errno));
        return;
    }
    initSock();
}

uint64_t Socket::getSendTimeout() const {
    FdCtx::Ptr ctx = FdMgr::getInstance()->getFdCtx(m_sockfd);
    if (ctx) {
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::setSendTimeout(uint64_t timeout) {
    timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    setopt(SOL_SOCKET, SO_SNDTIMEO, tv);
}

uint64_t Socket::getRecvTimeout() const {
    FdCtx::Ptr ctx = FdMgr::getInstance()->getFdCtx(m_sockfd);
    if (ctx) {
        return ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::setRecvTimeout(uint64_t timeout) {
    timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    setopt(SOL_SOCKET, SO_RCVTIMEO, tv);
}

int Socket::getopt(int level, int optname, void* optval, socklen_t* optlen) {
    return ::getsockopt(m_sockfd, level, optname, optval, optlen);
}

int Socket::setopt(int level, int optname, const void* optval, socklen_t optlen) {
    return ::setsockopt(m_sockfd, level, optname, optval, optlen);
}

bool Socket::connect(const Address::Ptr& addr, uint64_t timeout) {
    if (m_is_connected) {
        return true;
    }
    if (m_sockfd == -1) {
        newSock();
        if (m_sockfd == -1) {
            return false;
        }
    }

    if (addr->getFamily() != m_family) {
        LOG_ERROR("socket family is not match");
        return false;
    }

    if (timeout == (uint64_t)-1) {
        if (::connect(m_sockfd, addr->getAddr(), addr->getAddrLen()) && errno != EINPROGRESS) {
            LOG_ERROR("connect failed, errno = %d, %s", errno, strerror(errno));
            close();
            return false;
        }
    }
    else {
        if (::connect_with_timeout(m_sockfd, addr->getAddr(), addr->getAddrLen(), timeout) &&
            errno != EINPROGRESS) {
            LOG_ERROR("connect failed");
            close();
            return false;
        }
    }
    m_is_connected = true;
    getPeerAddress();
    getLocalAddress();
    return true;
}

bool Socket::reconnect(uint64_t timeout) {
    if (!m_peer_addr) {
        LOG_ERROR("peer address is null");
        return false;
    }
    m_local_addr.reset();
    return connect(m_peer_addr, timeout);
}

bool Socket::bind(const Address::Ptr& addr) {
    if (m_sockfd == -1) {
        newSock();
        if (m_sockfd == -1) {
            return false;
        }
    }
    if (addr->getFamily() != m_family) {
        LOG_ERROR("socket family is not match");
        return false;
    }
    if (::bind(m_sockfd, addr->getAddr(), addr->getAddrLen())) {
        LOG_ERROR("bind failed");
        return false;
    }
    getLocalAddress();
    return true;
}

bool Socket::listen(int backlog) {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return false;
    }
    if (::listen(m_sockfd, backlog)) {
        LOG_ERROR("listen failed");
        close();
        return false;
    }
    return true;
}

Socket::Ptr Socket::accept() {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return nullptr;
    }

    int sock = ::accept(m_sockfd, nullptr, nullptr);
    if (sock == -1) {
        LOG_ERROR("accept failed, errno = %d, %s", errno, strerror(errno));
        return nullptr;
    }
    Socket::Ptr sock_ptr(new Socket(m_family, m_type, m_protocol));
    if (sock_ptr->init(sock)) {
        return sock_ptr;
    }
    return nullptr;
}

int Socket::send(const void* buf, size_t len, int flags) {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return -1;
    }
    if (!m_is_connected) {
        LOG_ERROR("socket is not connected");
        return -1;
    }
    return ::send(m_sockfd, buf, len, flags);
}

int Socket::send(const iovec* iov, int iovcnt, int flags) {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return -1;
    }
    if (!m_is_connected) {
        LOG_ERROR("socket is not connected");
        return -1;
    }
    msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = const_cast<iovec*>(iov);
    msg.msg_iovlen = iovcnt;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    return ::sendmsg(m_sockfd, &msg, flags);
}

int Socket::sendto(const void* buf, size_t len, const Address::Ptr& addr, int flags) {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return -1;
    }
    if (!m_is_connected) {
        LOG_ERROR("socket is not connected");
        return -1;
    }
    if (addr->getFamily() != m_family) {
        LOG_ERROR("socket family is not match");
        return -1;
    }
    return ::sendto(m_sockfd, buf, len, flags, addr->getAddr(), addr->getAddrLen());
}

int Socket::sendto(const iovec* iov, int iovcnt, const Address::Ptr& addr, int flags) {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return -1;
    }
    if (!m_is_connected) {
        LOG_ERROR("socket is not connected");
        return -1;
    }
    if (addr->getFamily() != m_family) {
        LOG_ERROR("socket family is not match");
        return -1;
    }
    msghdr msg;
    msg.msg_name = addr->getAddr();
    msg.msg_namelen = addr->getAddrLen();
    msg.msg_iov = const_cast<iovec*>(iov);
    msg.msg_iovlen = iovcnt;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    return ::sendmsg(m_sockfd, &msg, flags);
}

int Socket::recv(void* buf, size_t len, int flags) {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return -1;
    }
    if (!m_is_connected) {
        LOG_ERROR("socket is not connected");
        return -1;
    }
    return ::recv(m_sockfd, buf, len, flags);
}

int Socket::recv(iovec* iov, int iovcnt, int flags) {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return -1;
    }
    if (!m_is_connected) {
        LOG_ERROR("socket is not connected");
        return -1;
    }
    msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = iovcnt;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    return ::recvmsg(m_sockfd, &msg, flags);
}

int Socket::recvfrom(void* buf, size_t len, Address::Ptr& addr, int flags) {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return -1;
    }
    if (!m_is_connected) {
        LOG_ERROR("socket is not connected");
        return -1;
    }
    socklen_t socklen = addr->getAddrLen();
    return ::recvfrom(m_sockfd, buf, len, flags, addr->getAddr(), &socklen);
}

int Socket::recvfrom(iovec* iov, int iovcnt, Address::Ptr& addr, int flags) {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return -1;
    }
    if (!m_is_connected) {
        LOG_ERROR("socket is not connected");
        return -1;
    }
    if (addr->getFamily() != m_family) {
        LOG_ERROR("socket family is not match");
        return -1;
    }
    msghdr msg;
    msg.msg_name = addr->getAddr();
    msg.msg_namelen = addr->getAddrLen();
    msg.msg_iov = iov;
    msg.msg_iovlen = iovcnt;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    int ret = ::recvmsg(m_sockfd, &msg, flags);
    return ret;
}

bool Socket::close() {
    if (!m_is_connected && m_sockfd == -1) {
        return true;
    }
    if (::close(m_sockfd)) {
        LOG_ERROR("close failed");
        return false;
    }
    m_sockfd = -1;
    m_is_connected = false;
    return true;
}

int Socket::shutdown(int how) {
    if (m_sockfd == -1) {
        LOG_ERROR("socket is not created");
        return -1;
    }
    return ::shutdown(m_sockfd, how);
}

std::string Socket::to_string() const {
    std::stringstream ss;
    ss << "[family: " << m_family << ", type: " << m_type << ", protocol: " << m_protocol
       << "], addr: " << m_local_addr->to_string();
    return ss.str();
}

bool Socket::cancelRead() {
    return IOManager::GetThis()->cancelEvent(m_sockfd, IOManager::READ);
}

bool Socket::cancelWrite() {
    return IOManager::GetThis()->cancelEvent(m_sockfd, IOManager::WRITE);
}

bool Socket::cancelAll() {
    return IOManager::GetThis()->cancelAll(m_sockfd);
}

Address::Ptr Socket::getLocalAddress() {
    if (m_local_addr) {
        return m_local_addr;
    }
    Address::Ptr addr;
    switch (m_family) {
    case AF_INET: addr.reset(new IPv4Address()); break;
    case AF_INET6: addr.reset(new IPv6Address()); break;
    default: addr.reset(new UnknownAddress(m_family)); break;
    }
    socklen_t len = addr->getAddrLen();
    if (::getsockname(m_sockfd, addr->getAddr(), &len) == -1) {
        LOG_ERROR("getsockname failed");
        return Address::Ptr(new UnknownAddress(m_family));
    }
    m_local_addr = addr;
    return m_local_addr;
}

Address::Ptr Socket::getPeerAddress() {
    if (m_peer_addr) {
        return m_peer_addr;
    }
    Address::Ptr addr;
    switch (m_family) {
    case AF_INET: addr.reset(new IPv4Address()); break;
    case AF_INET6: addr.reset(new IPv6Address()); break;
    default: addr.reset(new UnknownAddress(m_family)); break;
    }
    socklen_t len = addr->getAddrLen();
    if (::getpeername(m_sockfd, addr->getAddr(), &len) == -1) {
        LOG_ERROR("getpeername failed");
        return Address::Ptr(new UnknownAddress(m_family));
    }
    m_peer_addr = addr;
    return m_peer_addr;
}

namespace {

struct _SSLInit
{
    _SSLInit() {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
    }
};

static _SSLInit s_init;

}   // namespace
SSLSocket::Ptr SSLSocket::CreateTcp(Address::Ptr addr) {
    SSLSocket::Ptr sock(new SSLSocket(addr->getFamily(), Socket::TCP));
    return sock;
}

SSLSocket::Ptr SSLSocket::CreateTcpSocket() {
    SSLSocket::Ptr sock(new SSLSocket(Socket::IPv4, Socket::TCP));
    return sock;
}

SSLSocket::Ptr SSLSocket::CreateTcpScoketv6() {
    SSLSocket::Ptr sock(new SSLSocket(Socket::IPv6, Socket::TCP));
    return sock;
}

SSLSocket::SSLSocket(int family, int type, int protocol)
    : Socket(family, type, protocol)
    , m_ctx(NULL)
    , m_ssl(NULL) {}


bool SSLSocket::connect(const Address::Ptr& addr, uint64_t timeout) {
    bool ret = Socket::connect(addr, timeout);
    if (ret) {
        m_ctx.reset(SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free);
        m_ssl.reset(SSL_new(m_ctx.get()), SSL_free);
        SSL_set_fd(m_ssl.get(), m_sockfd);
        ret = (SSL_connect(m_ssl.get()) == 1);
    }
    return ret;
}

bool SSLSocket::bind(const Address::Ptr& addr) {
    return Socket::bind(addr);
}

bool SSLSocket::listen(int backlog) {
    return Socket::listen(backlog);
}

Socket::Ptr SSLSocket::accept() {
    SSLSocket::Ptr sock(new SSLSocket(m_family, m_type, m_protocol));
    int newsock = ::accept(m_sockfd, nullptr, nullptr);
    if (newsock == -1) {
        LOG_ERROR("accept failed");
        return nullptr;
    }
    sock->m_ctx = m_ctx;
    if (sock->init(newsock)) {
        return sock;
    }
    return nullptr;
}
int SSLSocket::send(const void* buf, size_t len, int flags) {
    if (!m_ssl) {
        LOG_ERROR("ssl is not init");
        return -1;
    }
    return SSL_write(m_ssl.get(), buf, len);
}

int SSLSocket::send(const iovec* iov, int iovcnt, int flags) {
    if (!m_ssl) {
        return -1;
    }
    int total = 0;
    for (int i = 0; i < iovcnt; ++i) {
        int tmp = SSL_write(m_ssl.get(), iov[i].iov_base, iov[i].iov_len);
        if (tmp <= 0) {
            return tmp;
        }
        total += tmp;
        if (tmp != (int)iov[i].iov_len) {
            break;
        }
    }
    return total;
}

int SSLSocket::recv(void* buf, size_t len, int flags) {
    if (!m_ssl) {
        LOG_ERROR("ssl is not init");
        return -1;
    }
    return SSL_read(m_ssl.get(), buf, len);
}

int SSLSocket::recv(iovec* iov, int iovcnt, int flags) {
    if (!m_ssl) {
        return -1;
    }
    int total = 0;
    for (int i = 0; i < iovcnt; ++i) {
        int tmp = SSL_read(m_ssl.get(), iov[i].iov_base, iov[i].iov_len);
        if (tmp <= 0) {
            return tmp;
        }
        total += tmp;
        if (tmp != (int)iov[i].iov_len) {
            break;
        }
    }
    return total;
}

int SSLSocket::shutdown(int how) {
    if (!m_ssl) {
        return false;
    }
    return SSL_shutdown(m_ssl.get());
}

bool SSLSocket::close() {
    return Socket::close();
}

std::string SSLSocket::to_string() const {
    std::stringstream ss;
    ss << "SSLSocket: " << Socket::to_string();
    return ss.str();
}

bool SSLSocket::loadCertificate(const std::string& cert_file, const std::string& key_file) {
    m_ctx.reset(SSL_CTX_new(SSLv23_server_method()), SSL_CTX_free);
    if (SSL_CTX_use_certificate_file(m_ctx.get(), cert_file.c_str(), SSL_FILETYPE_PEM) != 1) {
        ERR_print_errors_fp(stderr);
        LOG_ERROR("load certificate %s failed", cert_file.c_str());
        return false;
    }
    if (SSL_CTX_use_PrivateKey_file(m_ctx.get(), key_file.c_str(), SSL_FILETYPE_PEM) != 1) {
        ERR_print_errors_fp(stderr);
        LOG_ERROR("load key %s failed", key_file.c_str());
        return false;
    }
    if (SSL_CTX_check_private_key(m_ctx.get()) != 1) {
        ERR_print_errors_fp(stderr);
        LOG_ERROR("check key failed");
        return false;
    }
    return true;
}

bool SSLSocket::init(int sock) {
    bool v = Socket::init(sock);
    if (v) {
        m_ssl.reset(SSL_new(m_ctx.get()), SSL_free);
        SSL_set_fd(m_ssl.get(), m_sockfd);
        v = (SSL_accept(m_ssl.get()) == 1);
        if (!v) {
            ERR_print_errors_fp(stderr);
        }
    }
    return v;
}

}   // namespace pico