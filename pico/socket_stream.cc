#include "socket_stream.h"

namespace pico {
SocketStream::SocketStream(Socket::Ptr sock, bool owner)
    : m_sock(sock)
    , m_owner(owner) {}

SocketStream::~SocketStream() {
    if (m_owner) { m_sock->close(); }
}

int SocketStream::read(void* buf, size_t len) {
    if (!isConnected()) { return -1; }
    return m_sock->recv(buf, len);
}

int SocketStream::write(const void* buf, size_t len) {
    if (!isConnected()) { return -1; }
    return m_sock->send(buf, len);
}

int SocketStream::readFixSize(void* buf, size_t length) {
    if (!isConnected()) { return -1; }
    int64_t left = length;
    size_t offset = 0;
    while (left > 0) {
        uint64_t len = read(buf + offset, left);
        if (len <= 0) { return -1; }
        left -= len;
        offset += len;
    }
    return length;
}

int SocketStream::writeFixSize(const void* buf, size_t length) {
    if (!isConnected()) { return -1; }
    int64_t left = length;
    size_t offset = 0;
    while (left > 0) {
        uint64_t len = write(buf + offset, left);
        if (len <= 0) { return -1; }
        left -= len;
        offset += len;
    }
    return length;
}

void SocketStream::close() {
    if (m_sock) m_sock->close();
}

bool SocketStream::isConnected() const {
    return m_sock && m_sock->isConnected();
}

Address::Ptr SocketStream::getPeerAddress() {
    if (!isConnected()) { return nullptr; }
    return m_sock->getPeerAddress();
}

Address::Ptr SocketStream::getLocalAddress() {
    if (!isConnected()) { return nullptr; }
    return m_sock->getLocalAddress();
}

}   // namespace pico