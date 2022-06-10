#ifndef __PICO_WS_WS_SERVLET_H__
#define __PICO_WS_WS_SERVLET_H__

#include <memory>
#include <string>

#include "../http/http.h"
#include "ws_connection.h"

namespace pico {



class WsServlet
{
public:
    typedef std::shared_ptr<WsServlet> Ptr;

    virtual void onConnect(const pico::WsConnection::Ptr& req){};

    virtual void onMessage(const pico::WsConnection::Ptr& conn, pico::WsFrameMessage::Ptr& msg) = 0;

    virtual void onDisconnect(const pico::WsConnection::Ptr& conn){};


public:
    std::string m_name;
};

}   // namespace pico

#endif