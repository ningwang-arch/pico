#ifndef __PICO_WS_DISPATCH_H__
#define __PICO_WS_DISPATCH_H__

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "ws_servlet.h"

namespace pico {
class WsServletDispatcher
{
public:
    typedef std::shared_ptr<WsServletDispatcher> Ptr;

    WsServletDispatcher();

    void addServlet(const std::string& path, WsServlet::Ptr servlet, bool global = false);


    void reset();


    void delServlet(const std::string& path, bool global = false);

    void listAllServlets(std::map<std::string, WsServlet::Ptr>& servlets, bool global = false);


    WsServlet::Ptr dispatch(const std::string& path);

private:
    std::map<std::string, WsServlet::Ptr> m_servlets;
    std::map<std::string, WsServlet::Ptr> m_globalServlets;

    std::mutex m_mutex;
};
}   // namespace pico

#endif