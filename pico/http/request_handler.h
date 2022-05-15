#ifndef __PICO_HTTP_REQUEST_HANDLER_H__
#define __PICO_HTTP_REQUEST_HANDLER_H__

#include "http.h"
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "pico/filter.h"
#include "servlet.h"

namespace pico {
class RequestHandler
{
public:
    typedef std::shared_ptr<RequestHandler> Ptr;
    typedef std::function<void(const HttpRequest::Ptr&, HttpResponse::Ptr&)> Handler;

    struct Route
    {
        std::string path;
        Servlet::Ptr servlet;
    };

    RequestHandler();
    ~RequestHandler() = default;

    void addRoute(const Route& route);
    void addGlobalRoute(const Route& route);
    void addRoute(const std::string& path, Servlet::Ptr servlet);
    void addGlobalRoute(const std::string& path, Servlet::Ptr servlet);

    void addFilterChain(const std::string& url_pattern, FilterChain::Ptr filter_chain);


    void delRoute(const std::string& path);
    void delGlobalRoute(const std::string& path);


    void handle(const HttpRequest::Ptr& req, HttpResponse::Ptr& resp);

private:
    /**
     * 优先精确匹配
     * 如果没有找到，则模糊匹配
     */
    Servlet::Ptr findHandler(const std::string& path);

    FilterChain::Ptr findFilterChain(const std::string& path);

private:
    std::vector<Route> m_glob_routes;
    std::vector<Route> m_routes;

    std::map<std::string, FilterChain::Ptr> m_filter_chains;
};
}   // namespace pico


#endif