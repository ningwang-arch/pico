#include "tcp_server.h"
#include "config.h"
#include "logging.h"
#include <sstream>

namespace pico {
static pico::ConfigVar<uint64_t>::Ptr g_recvTimeout =
    pico::Config::Lookup<uint64_t>("other.recv.timeout", uint64_t(60 * 1000), "recv timeout");
TcpServer::TcpServer(IOManager* worker, IOManager* acceptor)
    : m_name("pico/1.0.0")
    , m_worker(worker)
    , m_acceptor(acceptor)
    , m_recvTimeout(g_recvTimeout->getValue())
    , m_is_stop(true) {}

TcpServer::~TcpServer() {
    for (auto& sock : m_sockets) {
        sock->close();
    }

    m_sockets.clear();
}

bool TcpServer::bind(Address::Ptr& addr, bool ssl) {
    std::vector<Address::Ptr> addrs;
    std::vector<Address::Ptr> fails;
    addrs.push_back(addr);
    bind(addrs, fails, ssl);
    return fails.empty();
}


bool TcpServer::bind(std::vector<Address::Ptr>& addrs, std::vector<Address::Ptr>& fails, bool ssl) {
    m_is_ssl = ssl;
    for (auto&& addr : addrs) {
        Socket::Ptr sock = ssl ? SSLSocket::CreateTcp(addr) : Socket::CreateTcp(addr);
        if (!sock->bind(addr)) {
            LOG_ERROR("addr[%s] bind error, errno=%d, %s",
                      addr->to_string().c_str(),
                      errno,
                      strerror(errno));
            fails.push_back(addr);
            continue;
        }
        if (!sock->listen()) {
            LOG_ERROR("listen on addr[%s] failed, errno=%d, %s",
                      addr->to_string().c_str(),
                      errno,
                      strerror(errno));
            fails.push_back(addr);
            continue;
        }
        m_sockets.push_back(sock);
    }
    if (!fails.empty()) {
        m_sockets.clear();
        return false;
    }
    return true;
}

void TcpServer::startAccept(Socket::Ptr& sock) {
    while (!m_is_stop) {
        Socket::Ptr client = sock->accept();
        if (!client) {
            LOG_ERROR("accept failed, errno=%d, %s", errno, strerror(errno));
            continue;
        }
        client->setRecvTimeout(m_recvTimeout);
        m_worker->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
    }
}

void TcpServer::handleClient(Socket::Ptr& sock) {
    LOG_INFO("handleClient");
}

bool TcpServer::start() {
    if (!m_is_stop) {
        return true;
    }
    m_is_stop = false;
    for (auto& sock : m_sockets) {
        LOG_INFO("server [%s] is listening on %s",
                 m_name.c_str(),
                 sock->getLocalAddress()->to_string().c_str());
    }
    for (auto&& sock : m_sockets) {
        m_acceptor->schedule(std::bind(&TcpServer::startAccept, shared_from_this(), sock));
    }

    return true;
}

void TcpServer::stop() {
    m_is_stop = true;

    auto self = shared_from_this();

    m_acceptor->schedule([self, this]() {
        for (auto&& sock : m_sockets) {
            sock->cancelAll();
            sock->close();
        }
        m_sockets.clear();
    });
}

std::string TcpServer::to_string() {
    std::stringstream ss;
    ss << "TcpServer[" << m_name << "]";
    return ss.str();
}

bool TcpServer::loadCertificate(const std::string& cert_file, const std::string& key_file) {
    for (auto&& sock : m_sockets) {
        auto ssl_sock = std::dynamic_pointer_cast<SSLSocket>(sock);
        if (!ssl_sock) {
            continue;
        }
        if (!ssl_sock->loadCertificate(cert_file, key_file)) {
            LOG_ERROR("loadCertificate failed");
            return false;
        }
    }
    return true;
}

}   // namespace pico