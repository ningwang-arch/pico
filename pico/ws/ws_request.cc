#include "ws_request.h"

#include <iostream>

#include "../logging.h"

namespace pico {
WsRequest::Ptr WsRequest::create(const std::string& url, uint64_t timeout,
                                 std::map<std::string, std::string> headers) {
    auto uri = Uri::Create(url);
    if (uri == nullptr) {
        LOG_ERROR("create uri failed");
        return nullptr;
    }
    return create(uri, timeout, headers);
}

WsRequest::Ptr WsRequest::create(const Uri::Ptr& uri, uint64_t timeout,
                                 std::map<std::string, std::string> headers) {
    auto addr = uri->getAddress();
    if (addr == nullptr) {
        LOG_ERROR("create addr failed");
        return nullptr;
    }
    Socket::Ptr sock = Socket::CreateTcp(addr);
    if (sock == nullptr) {
        LOG_ERROR("create socket failed");
        return nullptr;
    }
    if (!sock->connect(addr)) {
        LOG_ERROR("connect failed");
        return nullptr;
    }
    sock->setRecvTimeout(timeout);
    auto conn = std::make_shared<WsRequest>(sock);
    auto req = std::make_shared<HttpRequest>();

    req->set_path(uri->getPath());
    req->set_query(uri->getQuery());
    req->set_fragment(uri->getFragment());
    req->set_method(HttpMethod::GET);

    bool has_host = false;
    bool has_connection = false;

    for (auto& i : headers) {
        if (strcasecmp(i.first.c_str(), "connection") == 0) { has_connection = true; }
        else if (!has_host && strcasecmp(i.first.c_str(), "host") == 0) {
            has_host = !i.second.empty();
        }

        req->set_header(i.first, i.second);
    }
    req->set_websocket(true);
    if (!has_connection) { req->set_header("Connection", "Upgrade"); }
    req->set_header("Upgrade", "websocket");
    req->set_header("Sec-WebSocket-Version", "13");
    req->set_header("Sec-WebSocket-Key", base64_encode(genRandomString(16), false));
    if (!has_host) { req->set_header("Host", uri->getHost()); }

    int ret = conn->sendRequest(req);
    if (ret <= 0) {
        LOG_ERROR("send request failed");
        return nullptr;
    }

    auto resp = conn->recvResponse();
    if (resp == nullptr) {
        LOG_ERROR("recv response failed");
        return nullptr;
    }
    if (resp->get_status() != HttpStatus::SWITCHING_PROTOCOLS) {
        LOG_ERROR("upgrade to websocket failed");
        return nullptr;
    }
    return conn;
}

WsFrameMessage::Ptr WsRequest::recvMessage() {
    return WsRecvMessage(this, true);
}

int32_t WsRequest::sendMessage(const WsFrameMessage::Ptr& msg, bool is_fin) {
    return WsSendMessage(this, msg, true, is_fin);
}

int32_t WsRequest::sendMessage(const std::string& data, const uint8_t opcode, bool is_fin) {
    return WsSendMessage(this, std::make_shared<WsFrameMessage>(opcode, data), true, is_fin);
}

}   // namespace pico