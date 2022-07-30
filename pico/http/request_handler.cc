#include "request_handler.h"
#include "../logging.h"

#include "servlets/404_servlet.h"

#include <fnmatch.h>

namespace pico {
RequestHandler::RequestHandler() {}

void RequestHandler::addRoute(const std::string& path, Servlet::Ptr servlet) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_routes.push_back({path, servlet});
}

void RequestHandler::addGlobalRoute(const std::string& path, Servlet::Ptr servlet) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_glob_routes.push_back({path, servlet});
}

void RequestHandler::addRoute(const Route& route) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_routes.push_back(route);
}

void RequestHandler::addGlobalRoute(const Route& route) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_glob_routes.push_back(route);
}

void RequestHandler::delRoute(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it = m_routes.begin(); it != m_routes.end(); ++it) {
        if (it->path == path) {
            m_routes.erase(it);
            return;
        }
    }
}

void RequestHandler::delGlobalRoute(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it = m_glob_routes.begin(); it != m_glob_routes.end(); ++it) {
        if (it->path == path) {
            m_glob_routes.erase(it);
            return;
        }
    }
}

void RequestHandler::addExcludePath(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    exclude_paths.push_back(path);
}

void RequestHandler::addExcludePath(const std::vector<std::string>& paths) {
    std::mutex m;
    std::lock_guard<std::mutex> lock(m);
    for (auto& path : paths) {
        addExcludePath(path);
    }
}

void RequestHandler::delExcludePath(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it = exclude_paths.begin(); it != exclude_paths.end(); ++it) {
        if (*it == path) {
            exclude_paths.erase(it);
            return;
        }
    }
}

void RequestHandler::delExcludePath(const std::vector<std::string>& paths) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& path : paths) {
        delExcludePath(path);
    }
}

bool RequestHandler::isExcludePath(const std::string& path) {
    for (auto& exclude_path : exclude_paths) {
        if (fnmatch(exclude_path.c_str(), path.c_str(), 0) == 0) {
            return true;
        }
    }
    return false;
}

Servlet::Ptr RequestHandler::findHandler(const std::string& path) {
    for (auto& route : m_routes) {
        if (route.path == path || (fnmatch(route.path.c_str(), path.c_str(), 0) == 0) ||
            fuzzy_match(path, route.path)) {
            return route.servlet;
        }
    }
    return Servlet::Ptr(new NotFoundServlet());
}

void RequestHandler::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_routes.clear();
    m_glob_routes.clear();
    exclude_paths.clear();
}

void RequestHandler::listAllRoutes(std::map<std::string, Servlet::Ptr>& routes) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& route : m_routes) {
        routes[route.path] = route.servlet;
    }
}

void RequestHandler::listAllGlobalRoutes(std::map<std::string, Servlet::Ptr>& routes) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& route : m_glob_routes) {
        routes[route.path] = route.servlet;
    }
}

void RequestHandler::handle(const HttpRequest::Ptr& req, HttpResponse::Ptr& resp) {
    auto path = req->get_path();
    auto servlet = findHandler(path);
    if (servlet == nullptr) {
        servlet = Servlet::Ptr(new NotFoundServlet());
    }
    if (isExcludePath(path)) {
        servlet->service(req, resp);
    }
    else {
        FilterChain::Ptr filter_chain(new FilterChain());
        if (findFilterChain(path)) {
            filter_chain->setFilters(findFilterChain(path)->getFilters());
        }
        filter_chain->reuse();
        filter_chain->setServlet(servlet);
        filter_chain->doFilter(req, resp);
    }
}
}   // namespace pico