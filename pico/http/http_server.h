#ifndef __PICO_HTTP_HTTP_SERVER_H__
#define __PICO_HTTP_HTTP_SERVER_H__

#include "../logging.h"
#include "../tcp_server.h"
#include "../util.h"
#include "http.h"
#include "http_connection.h"
#include "request_handler.h"
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace pico {

class HttpServer : public TcpServer
{
public:
    typedef std::shared_ptr<HttpServer> Ptr;

    HttpServer(bool keepalive = false, IOManager* worker = IOManager::getThis(),
               IOManager* acceptor = IOManager::getThis());

    ~HttpServer(){};



protected:
    void handleClient(Socket::Ptr& sock) override;

private:
    RequestHandler::Ptr m_request_handler;
    bool m_is_KeepAlive;
};
}   // namespace pico

#endif