#ifndef __PICO_WS_WS_REQUEST_H__
#define __PICO_WS_WS_REQUEST_H__

#include "../http/request.h"
#include "ws_connection.h"

namespace pico {
class WsRequest : public Request
{
public:
    typedef std::shared_ptr<WsRequest> Ptr;

    WsRequest(Socket::Ptr sock, bool owner = true)
        : Request(sock, owner) {}

    static WsRequest::Ptr create(const std::string& url, uint64_t timeout = 0,
                                 std::map<std::string, std::string> headers = {});
    static WsRequest::Ptr create(const Uri::Ptr& uri, uint64_t timeout = 0,
                                 std::map<std::string, std::string> headers = {});

    WsFrameMessage::Ptr recvMessage();
    int32_t sendMessage(const WsFrameMessage::Ptr& msg, bool is_fin = true);
    int32_t sendMessage(const std::string& data, const uint8_t opcode = WsFrameHeader::TEXT,
                        bool is_fin = true);
};

}   // namespace pico

#endif   // __PICO_WS_WS_REQUEST_H__