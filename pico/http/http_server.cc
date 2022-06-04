#include "http_server.h"

#include "../class_factory.h"
#include "../compression.h"
#include "pico/config.h"

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

        if (compression::is_compression_enabled()) {
            std::string compress_type;
            if (req->has_header("Content-Encoding", &compress_type)) {
                if (compress_type == "gzip") {
                    req->set_body(compression::decompress(req->get_body(), compression::GZIP));
                    req->del_header("Content-Encoding");
                }
                else if (compress_type == "deflate") {
                    req->set_body(compression::decompress(req->get_body(), compression::DEFLATE));
                    req->del_header("Content-Encoding");
                }
                req->set_header("Content-Length", std::to_string(req->get_body().size()));
            }
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


        if (compression::is_compression_enabled()) {
            std::string accept_encoding = req->get_header("Accept-Encoding");
            if (accept_encoding.find("gzip") != std::string::npos) {
                resp->set_header("Content-Encoding", "gzip");
                resp->set_body(compression::compress(resp->get_body(), compression::GZIP));
            }
            else if (accept_encoding.find("deflate") != std::string::npos) {
                resp->set_header("Content-Encoding", "deflate");
                resp->set_body(compression::compress(resp->get_body(), compression::DEFLATE));
            }
        }

        conn->sendResponse(resp);
        if (!m_is_KeepAlive || req->is_close()) { break; }
    } while (true);
    conn->close();
}

}   // namespace pico
