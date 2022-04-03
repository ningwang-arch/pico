#include "request_handler.h"
#include "../logging.h"

#include <fnmatch.h>

namespace pico {



RequestHandler::RequestHandler() {}

void RequestHandler::addRoute(const std::string& path, const HttpMethod& method, Handler handler) {
    for (auto& route : m_routes) {
        if (route.path == path && route.method == method) {
            route.handler = handler;
            return;
        }
    }
    Route route = {path, method, handler};
    m_routes.push_back(route);
}
void RequestHandler::addGlobalRoute(const std::string& path, const HttpMethod& method,
                                    Handler handler) {
    for (auto& route : m_glob_routes) {
        if (route.path == path && route.method == method) {
            route.handler = handler;
            return;
        }
    }
    Route route = {path, method, handler};
}

void RequestHandler::delRoute(const std::string& path, const HttpMethod& method) {
    for (auto it = m_routes.begin(); it != m_routes.end(); ++it) {
        if (it->path == path && it->method == method) {
            m_routes.erase(it);
            return;
        }
    }
}

void RequestHandler::delGlobalRoute(const std::string& path, const HttpMethod& method) {
    for (auto it = m_glob_routes.begin(); it != m_glob_routes.end(); ++it) {
        if (it->path == path && it->method == method) {
            m_glob_routes.erase(it);
            return;
        }
    }
}

void RequestHandler::delRoute(const std::string& path) {
    for (auto it = m_routes.begin(); it != m_routes.end(); ++it) {
        if (it->path == path) { m_routes.erase(it); }
    }
}

void RequestHandler::delGlobalRoute(const std::string& path) {
    for (auto it = m_glob_routes.begin(); it != m_glob_routes.end(); ++it) {
        if (it->path == path) { m_glob_routes.erase(it); }
    }
}

void RequestHandler::handle(const HttpRequest::Ptr& req, HttpResponse::Ptr& resp) {
    Handler handler = findHandler(req->get_path(), req->get_method());
    if (handler) { handler(req, resp); }
    else {
        m_default.handler(req, resp);
    }
}

RequestHandler::Handler RequestHandler::findHandler(const std::string& path,
                                                    const HttpMethod& method) {
    for (auto it = m_routes.begin(); it != m_routes.end(); ++it) {
        if (it->path == path && it->method == method) { return it->handler; }
    }
    for (auto it = m_glob_routes.begin(); it != m_glob_routes.end(); ++it) {
        if (fnmatch(it->path.c_str(), path.c_str(), 0) == 0 && it->method == method) {
            return it->handler;
        }
    }
    return nullptr;
}


}   // namespace pico