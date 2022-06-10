#ifndef __PICO_WS_WS_SERVER_H__
#define __PICO_WS_WS_SERVER_H__

#include <memory>

#include "../tcp_server.h"
#include "ws_servlet_dispatch.h"

namespace pico {

class WsServer : public TcpServer
{
public:
    typedef std::shared_ptr<WsServer> Ptr;

    WsServer(bool keepalive = false, IOManager* worker = IOManager::GetThis(),
             IOManager* acceptor = IOManager::GetThis());

    WsServletDispatcher::Ptr getServletDispatcher() { return m_servlet_dispatcher; }

private:
    void handleClient(Socket::Ptr& sock) override;

private:
    WsServletDispatcher::Ptr m_servlet_dispatcher;
};

}   // namespace pico

#endif   // __PICO_WS_WS_SERVER_H__