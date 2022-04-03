#ifndef __PICO_HTTP_HTTP_SERVER_H__
#define __PICO_HTTP_HTTP_SERVER_H__

#include "../tcp_server.h"
#include "http.h"
#include "http_connection.h"
#include "request_handler.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace pico {
class HttpServer : public TcpServer
{
public:
    typedef std::shared_ptr<HttpServer> Ptr;

    HttpServer(bool keepalive = false, IOManager* worker = IOManager::getThis(),
               IOManager* acceptor = IOManager::getThis());

    RequestHandler::Ptr getRequestHandler() const { return m_request_handler; }

    void setName(const std::string& name) override;

    void setRequestHandler(RequestHandler::Ptr& handler) { m_request_handler = handler; }

protected:
    void handleClient(Socket::Ptr& sock) override;

private:
    RequestHandler::Ptr m_request_handler;
    bool m_is_KeepAlive;
};
}   // namespace pico

#endif