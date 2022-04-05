#include "tcp_server.h"
#include "logging.h"
#include <sstream>

namespace pico {
const static uint64_t recvTimeout = 60 * 1000;
TcpServer::TcpServer(IOManager* worker, IOManager* acceptor)
    : m_name("pico/1.0.0")
    , m_worker(worker)
    , m_acceptor(acceptor)
    , m_recvTimeout(recvTimeout)
    , m_is_stop(true) {}

TcpServer::~TcpServer() {
    for (auto& sock : m_sockets) { sock->close(); }

    m_sockets.clear();
}

bool TcpServer::bind(Address::Ptr& addr) {
    std::vector<Address::Ptr> addrs;
    std::vector<Address::Ptr> fails;
    addrs.push_back(addr);
    bind(addrs, fails);
    return fails.empty();
}


bool TcpServer::bind(std::vector<Address::Ptr>& addrs, std::vector<Address::Ptr>& fails) {
    for (auto&& addr : addrs) {
        Socket::Ptr sock = Socket::CreateTcp(addr);
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
    if (!m_is_stop) { return true; }
    m_is_stop = false;
    for (auto& sock : m_sockets) {
        LOG_INFO("server is listening on %s", sock->to_string().c_str());
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

}   // namespace pico