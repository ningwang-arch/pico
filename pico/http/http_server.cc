#include "http_server.h"

#include "../class_factory.h"
#include "pico/config.h"
#include "pico/macro.h"

namespace pico {

struct Route
{
    std::string path;
    Servlet::Ptr servlet;
};

template<>
class LexicalCast<std::string, Route>
{
public:
    Route operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Route route;
        route.path = node["path"].as<std::string>();
        std::string class_name = node["class"].as<std::string>();
        route.servlet =
            std::static_pointer_cast<Servlet>(ClassFactory::Instance().Create(class_name));
        if (!route.servlet) { route.servlet = std::make_shared<Servlet>(); }
        return route;
    }
};

template<>
class LexicalCast<Route, std::string>
{
public:
    std::string operator()(const Route& v) {
        YAML::Node node;
        node["path"] = v.path;
        node["class"] = typeid(*v.servlet).name();
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


static ConfigVar<std::vector<Route>>::Ptr g_servlets = Config::Lookup<std::vector<Route>>(
    CONF_ROOT + std::string("servlet"), std::vector<Route>(), "servlets");

HttpServer::HttpServer(bool keepalive, IOManager* worker, IOManager* acceptor)
    : TcpServer(worker, acceptor)
    , m_request_handler(new RequestHandler())
    , m_is_KeepAlive(keepalive) {
    if (!m_request_handler) { m_request_handler = std::make_shared<RequestHandler>(); }

    for (auto& route : g_servlets->getValue()) {
        if (route.path.empty()) { continue; }
        else if (route.path.find("*") != std::string::npos) {
            m_request_handler->addGlobalRoute(route.path, route.servlet);
        }
        else {
            m_request_handler->addRoute(route.path, route.servlet);
        }
    }
}

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
