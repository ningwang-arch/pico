#include "ws_servlet_dispatch.h"

namespace pico {
WsServletDispatcher::WsServletDispatcher() {}

void WsServletDispatcher::addServlet(const std::string& path, WsServlet::Ptr servlet, bool global) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (global) { m_globalServlets[path] = servlet; }
    else {
        m_servlets[path] = servlet;
    }
}

void WsServletDispatcher::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_servlets.clear();
    m_globalServlets.clear();
}

void WsServletDispatcher::delServlet(const std::string& path, bool global) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (global) { m_globalServlets.erase(path); }
    else {
        m_servlets.erase(path);
    }
}

void WsServletDispatcher::listAllServlets(std::map<std::string, WsServlet::Ptr>& servlets,
                                          bool global) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (global) { servlets = m_globalServlets; }
    else {
        servlets = m_servlets;
    }
}

WsServlet::Ptr WsServletDispatcher::dispatch(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_servlets.find(path);
    if (it != m_servlets.end()) { return it->second; }
    it = m_globalServlets.find(path);
    if (it != m_globalServlets.end()) { return it->second; }
    return nullptr;
}


}   // namespace pico