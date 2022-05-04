#include "request_handler.h"
#include "../logging.h"

#include "servlets/404_servlet.h"

#include <fnmatch.h>

namespace pico {
RequestHandler::RequestHandler() {}

void RequestHandler::addRoute(const std::string& path, Servlet::Ptr servlet) {
    m_routes.push_back({path, servlet});
}

void RequestHandler::addGlobalRoute(const std::string& path, Servlet::Ptr servlet) {
    m_glob_routes.push_back({path, servlet});
}

void RequestHandler::addRoute(const Route& route) {
    m_routes.push_back(route);
}

void RequestHandler::addGlobalRoute(const Route& route) {
    m_glob_routes.push_back(route);
}

void RequestHandler::delRoute(const std::string& path) {
    for (auto it = m_routes.begin(); it != m_routes.end(); ++it) {
        if (it->path == path) {
            m_routes.erase(it);
            return;
        }
    }
}

void RequestHandler::delGlobalRoute(const std::string& path) {
    for (auto it = m_glob_routes.begin(); it != m_glob_routes.end(); ++it) {
        if (it->path == path) {
            m_glob_routes.erase(it);
            return;
        }
    }
}

Servlet::Ptr RequestHandler::findHandler(const std::string& path) {
    for (auto& route : m_routes) {
        if (fnmatch(route.path.c_str(), path.c_str(), 0) == 0) { return route.servlet; }
    }
    for (auto& route : m_glob_routes) {
        if (fnmatch(route.path.c_str(), path.c_str(), 0) == 0) { return route.servlet; }
    }
    return Servlet::Ptr(new NotFoundServlet());
}

void RequestHandler::handle(const HttpRequest::Ptr& req, HttpResponse::Ptr& resp) {
    auto path = req->get_path();
    auto servlet = findHandler(path);
    servlet->service(req, resp);
}

}   // namespace pico