#include "ws_connection.h"

#include <iostream>
#include <sstream>

#include "../logging.h"

namespace pico {
const uint32_t g_websocket_message_max_size = 32 * 1024 * 1024;

std::string WsFrameHeader::toString() const {
    std::stringstream ss;
    ss << "fin: " << (int)fin << " rsv1: " << (int)rsv1 << " rsv2: " << (int)rsv2
       << " rsv3: " << (int)rsv3 << " opcode: " << (int)opcode << " mask: " << (int)mask
       << " payload_len: " << (int)payload_len;
    return ss.str();
}

HttpRequest::Ptr WsConnection::handleShake() {
    HttpRequest::Ptr req;
    do {
        req = recvRequest();
        if (!req) {
            LOG_ERROR("recv request failed");
            break;
        }
        if (strcasecmp(req->get_header("Upgrade").c_str(), "websocket")) {
            LOG_ERROR("upgrade to websocket failed");
            break;
        }

        if (find(req->get_header("Connection"), "Upgrade", true) == std::string::npos) {
            LOG_ERROR("upgrade to websocket failed");
            break;
        }

        if (req->get_header("Sec-webSocket-Version") != "13") {
            LOG_ERROR("http header Sec-webSocket-Version != 13");
            break;
        }

        std::string sec_key = req->get_header("Sec-WebSocket-Key");
        if (sec_key.empty()) {
            LOG_ERROR("http header Sec-WebSocket-Key is empty");
            break;
        }



        std::string sec_key_accept = sec_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        // https://zh.wikipedia.org/zh-cn/Base64#URL
        // f**k, sec_key_accept should be base64 encoded, but it doesn't need to url encoded.
        // unfortunately, base64_encode/base64_decode should be url encoded in jwt.
        // so I have to add a parameter to base64_encode/base64_decode to enable/disable url
        // encode/decode.
        sec_key_accept = base64_encode(sha1sum(sec_key_accept), false);
        req->set_websocket(true);

        auto resp = std::make_shared<HttpResponse>(req->get_version(), req->is_close());
        resp->set_status(HttpStatus::SWITCHING_PROTOCOLS);
        resp->set_header("Upgrade", "websocket");
        resp->set_header("Connection", "Upgrade");
        resp->set_header("Sec-WebSocket-Accept", sec_key_accept);
        resp->set_header("Sec-WebSocket-Version", "13");
        resp->set_websocket(true);
        resp->set_reason("Web Socket Protocol Handshake");

        sendResponse(resp);

        return req;

    } while (false);

    return nullptr;
}

WsFrameMessage::Ptr WsConnection::recvMessage() {
    return WsRecvMessage(this, false);
}

int32_t WsConnection::sendMessage(const WsFrameMessage::Ptr& msg, bool is_fin) {
    return WsSendMessage(this, msg, false, is_fin);
}

int32_t WsConnection::sendMessage(const std::string& data, const uint8_t opcode, bool is_fin) {
    return WsSendMessage(this, std::make_shared<WsFrameMessage>(opcode, data), false, is_fin);
}

int32_t WsConnection::ping() {
    return WsPing(this);
}

int32_t WsConnection::pong() {
    return WsPong(this);
}

WsFrameMessage::Ptr WsRecvMessage(SocketStream* stream, bool is_client) {
    uint8_t opcode = 0;
    std::string data;
    int cur_len = 0;

    do {
        WsFrameHeader header;

        if (stream->readFixSize(&header, sizeof(header)) <= 0) {
            break;
        }
        if (header.opcode == WsFrameHeader::PING) {
            if (WsPong(stream) <= 0) {
                break;
            }
        }
        else if (header.opcode == WsFrameHeader::PONG) {}
        else if (header.opcode == WsFrameHeader::BINARY || header.opcode == WsFrameHeader::TEXT ||
                 header.opcode == WsFrameHeader::CONTINUATION) {
            if (!is_client && !header.mask) {
                break;
            }

            uint64_t length;
            if (header.payload_len == 126) {
                uint16_t len = 0;
                if (stream->readFixSize(&len, sizeof(len)) <= 0) {
                    break;
                }
                length = byteswapOnLittleEndian(len);
            }
            else if (header.payload_len == 127) {
                uint64_t len = 0;
                if (stream->readFixSize(&len, sizeof(len)) <= 0) {
                    break;
                }
                length = byteswapOnLittleEndian(len);
            }
            else {
                length = header.payload_len;
            }

            if (cur_len + length >= g_websocket_message_max_size) {
                break;
            }
            char mask[4];
            if (header.mask) {
                if (stream->readFixSize(mask, sizeof(mask)) <= 0) {
                    break;
                }
            }

            data.resize(cur_len + length);
            if (stream->readFixSize(&data[cur_len], length) <= 0) {
                break;
            }
            if (header.mask) {
                for (size_t i = 0; i < length; i++) {
                    data[cur_len + i] = data[cur_len + i] ^ mask[i % 4];
                }
            }
            cur_len += length;
            if (!opcode && header.opcode != WsFrameHeader::CONTINUATION) {
                opcode = header.opcode;
            }

            if (header.fin) {
                return std::make_shared<WsFrameMessage>(opcode, std::move(data));
            }
        }
        else if (header.opcode == WsFrameHeader::CLOSE) {
            return std::make_shared<WsFrameMessage>(WsFrameHeader::CLOSE, std::move(data));
        }
        else {
            break;
        }
    } while (true);

    stream->close();
    return std::make_shared<WsFrameMessage>(WsFrameHeader::ERROR, std::move(data));
}

int32_t WsSendMessage(SocketStream* stream, const WsFrameMessage::Ptr& message, bool is_client,
                      bool is_fin) {
    do {
        WsFrameHeader header;
        memset(&header, 0, sizeof(header));
        header.fin = is_fin;
        header.opcode = message->get_opcode();
        header.mask = is_client;
        header.payload_len = message->get_data().size();
        if (header.payload_len < 126) {}
        else if (header.payload_len < 0xFFFF) {
            header.payload_len = 126;
        }
        else {
            header.payload_len = 127;
        }
        if (stream->writeFixSize(&header, sizeof(header)) <= 0) {
            LOG_ERROR("write header failed");
            break;
        }

        if (header.payload_len == 126) {
            uint16_t len = byteswapOnLittleEndian(message->get_data().size());
            if (stream->writeFixSize(&len, sizeof(len)) <= 0) {
                LOG_ERROR("write payload_len failed");
                break;
            }
        }
        else if (header.payload_len == 127) {
            uint64_t len = byteswapOnLittleEndian(message->get_data().size());
            if (stream->writeFixSize(&len, sizeof(len)) <= 0) {
                LOG_ERROR("write payload_len failed");
                break;
            }
        }

        if (is_client) {
            char mask[4];
            uint32_t rand_value = rand();
            memcpy(mask, &rand_value, sizeof(mask));
            std::string& data = message->get_data();
            for (size_t i = 0; i < data.size(); ++i) {
                data[i] ^= mask[i % 4];
            }

            if (stream->writeFixSize(mask, sizeof(mask)) <= 0) {
                break;
            }
        }

        if (message->get_data().size() > 0) {
            if (stream->writeFixSize(message->get_data().data(), message->get_data().size()) <= 0) {
                LOG_ERROR("write payload failed");
                break;
            }
        }

        return message->get_data().size() + sizeof(header);
    } while (0);
    stream->close();
    return -1;
}

int32_t WsPing(SocketStream* stream) {
    WsFrameHeader header;
    header.fin = true;
    header.rsv1 = false;
    header.rsv2 = false;
    header.rsv3 = false;
    header.opcode = WsFrameHeader::PING;
    header.mask = false;
    header.payload_len = 0;
    int32_t ret = stream->writeFixSize(&header, sizeof(header));
    if (ret <= 0) {
        stream->close();
    }
    return ret;
}
int32_t WsPong(SocketStream* stream) {
    WsFrameHeader header;
    header.fin = true;
    header.rsv1 = false;
    header.rsv2 = false;
    header.rsv3 = false;
    header.opcode = WsFrameHeader::PONG;
    header.mask = false;
    header.payload_len = 0;
    int32_t ret = stream->writeFixSize(&header, sizeof(header));
    if (ret <= 0) {
        stream->close();
    }
    return ret;
}



}   // namespace pico