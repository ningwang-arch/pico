#include "http_server.h"
#include "../logging.h"

namespace pico {
HttpServer::HttpServer(bool keepalive, IOManager* worker, IOManager* acceptor)
    : TcpServer(worker, acceptor)
    , m_request_handler(new RequestHandler())
    , m_is_KeepAlive(keepalive){};

void HttpServer::setName(const std::string& name) {
    TcpServer::setName(name);
}

void HttpServer::handleClient(Socket::Ptr& sock) {
    LOG_INFO("handleClient");
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