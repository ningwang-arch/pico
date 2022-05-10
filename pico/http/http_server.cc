#include "http_server.h"

#include "../class_factory.h"
#include "pico/config.h"
#include "pico/macro.h"

namespace pico {


HttpServer::HttpServer(bool keepalive, IOManager* worker, IOManager* acceptor)
    : TcpServer(worker, acceptor)
    , m_request_handler(new RequestHandler())
    , m_is_KeepAlive(keepalive) {}

void HttpServer::handleClient(Socket::Ptr& sock) {
    HttpConnection::Ptr conn(new HttpConnection(sock));
    do {
        auto req = conn->recvRequest();
        if (!req) {
            LOG_ERROR("recvRequest failed, close connection, sock: %s, errno=%d, %s",
                      sock->to_string().c_str(),
                      errno,
                      strerror(errno));
            break;
        }

        LOG_INFO("server [%s] recv request from %s, %s %s",
                 this->getName().c_str(),
                 sock->getPeerAddress()->to_string().c_str(),
                 http_method_to_string(req->get_method()),
                 req->get_path().c_str());

        HttpResponse::Ptr resp(
            new HttpResponse(req->get_version(), req->is_close() || !m_is_KeepAlive));
        resp->set_header("Server", getName());
        m_request_handler->handle(req, resp);

        conn->sendResponse(resp);
        if (!m_is_KeepAlive || req->is_close()) { break; }
    } while (true);
    conn->close();
}

}   // namespace pico
