#ifndef __PICO_HTTP_REQUEST_HANDLER_H__
#define __PICO_HTTP_REQUEST_HANDLER_H__

#include "http.h"
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace pico {
class RequestHandler
{
public:
    typedef std::shared_ptr<RequestHandler> Ptr;
    typedef std::function<void(const HttpRequest::Ptr&, HttpResponse::Ptr&)> Handler;

    struct Route
    {
        std::string path;
        HttpMethod method;
        Handler handler;
    };

    RequestHandler();
    ~RequestHandler() = default;

    void addRoute(const std::string& path, const HttpMethod& method, Handler handler);
    void addGlobalRoute(const std::string& path, const HttpMethod& method, Handler handler);

    void delRoute(const std::string& path, const HttpMethod& method);
    void delGlobalRoute(const std::string& path, const HttpMethod& method);
    void delRoute(const std::string& path);
    void delGlobalRoute(const std::string& path);

    Route getDefaultRoute() const;
    void setDefaultRoute(const Route& route);

    void handle(const HttpRequest::Ptr& req, HttpResponse::Ptr& resp);

private:
    /**
     * 优先精确匹配
     * 如果没有找到，则模糊匹配
     */
    Handler findHandler(const std::string& path, const HttpMethod& method);

private:
    std::vector<Route> m_glob_routes;
    std::vector<Route> m_routes;
    Route m_default = {
        "/", HttpMethod::GET, [](const HttpRequest::Ptr& req, HttpResponse::Ptr& resp) {
            resp->set_status(HttpStatus::NOT_FOUND);
            resp->set_header("Content-Type", "text/html");
            resp->set_header("Server", "pico/1.0.0");
            resp->set_body("<html>"
                           "<head>"
                           "<title>404 Not Found</title>"
                           "</head>"
                           "<body>"
                           "<center><h1>404 Not Found</h1></center>"
                           "<hr>"
                           "<center>Pico</center>"
                           "</body>"
                           "</html>");
        }};
};
}   // namespace pico


#endif