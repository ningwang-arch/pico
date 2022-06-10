#ifndef __PICO_HTTP_REQUEST_HANDLER_H__
#define __PICO_HTTP_REQUEST_HANDLER_H__

#include "http.h"
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
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

    void reset();

    void delRoute(const std::string& path);
    void delGlobalRoute(const std::string& path);


    void handle(const HttpRequest::Ptr& req, HttpResponse::Ptr& resp);

    void addExcludePath(const std::string& path);
    void addExcludePath(const std::vector<std::string>& paths);

    void delExcludePath(const std::string& path);
    void delExcludePath(const std::vector<std::string>& paths);

    void listAllRoutes(std::map<std::string, Servlet::Ptr>& routes);
    void listAllGlobalRoutes(std::map<std::string, Servlet::Ptr>& routes);

private:
    /**
     * 优先精确匹配
     * 如果没有找到，则模糊匹配
     */
    Servlet::Ptr findHandler(const std::string& path);

    bool isExcludePath(const std::string& path);


private:
    std::vector<Route> m_glob_routes;
    std::vector<Route> m_routes;

    std::vector<std::string> exclude_paths;

    std::mutex m_mutex;
};
}   // namespace pico


#endif
