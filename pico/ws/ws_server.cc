#include "ws_server.h"

namespace pico {
WsServer::WsServer(bool keepalive, IOManager* worker, IOManager* acceptor)
    : TcpServer(worker, acceptor)
    , m_servlet_dispatcher(std::make_shared<WsServletDispatcher>()) {}

void WsServer::handleClient(Socket::Ptr& sock) {
    auto conn = std::make_shared<WsConnection>(sock);
    do {
        auto header = conn->handleShake();
        if (!header) { break; }

        auto servlet = m_servlet_dispatcher->dispatch(header->get_path());
        if (!servlet) { break; }
        servlet->onConnect(conn);

        while (true) {
            auto msg = conn->recvMessage();
            if (msg->get_opcode() == WsFrameHeader::CLOSE) { break; }
            servlet->onMessage(conn, msg);
        }

        servlet->onDisconnect(conn);

    } while (0);
    conn->close();
}

}   // namespace pico