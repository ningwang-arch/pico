#include "ws_server.h"

namespace pico {
WsServer::WsServer(IOManager* worker, IOManager* acceptor)
    : TcpServer(worker, acceptor)
    , m_servlet_dispatcher(std::make_shared<WsServletDispatcher>()) {}

void WsServer::handleClient(Socket::Ptr& sock) {
    auto conn = std::make_shared<WsConnection>(sock);
    do {
        auto header = conn->handleShake();
        if (!header) { break; }

        LOG_INFO("server [%s] recv request from %s, %s %s",
                 this->getName().c_str(),
                 sock->getPeerAddress()->to_string().c_str(),
                 http_method_to_string(header->get_method()),
                 header->get_path().c_str());

        auto servlet = m_servlet_dispatcher->dispatch(header->get_path());
        if (!servlet) { break; }

        if (!servlet->onConnect(header)) { break; }

        while (true) {
            auto msg = conn->recvMessage();
            if (msg->get_opcode() == WsFrameHeader::CLOSE) { break; }
            if (msg->get_opcode() == WsFrameHeader::ERROR) {
                servlet->onError(conn);
                break;
            }
            else {
                servlet->onMessage(conn, msg);
            }
        }

        servlet->onDisconnect(conn);

    } while (0);
    conn->close();
}

}   // namespace pico