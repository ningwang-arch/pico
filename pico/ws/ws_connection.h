#ifndef __PICO_WS_WS_CONNECTION_H__
#define __PICO_WS_WS_CONNECTION_H__

#include <memory>
#include <stdint.h>
#include <string>

#include "../http/http.h"
#include "../http/http_connection.h"
#include "../socket_stream.h"
#include "../uri.h"

namespace pico {

/**
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-------+-+-------------+-------------------------------+
 |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 | |1|2|3|       |K|             |                               |
 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 |     Extended payload length continued, if payload len == 127  |
 + - - - - - - - - - - - - - - - +-------------------------------+
 |                               |Masking-key, if MASK set to 1  |
 +-------------------------------+-------------------------------+
 | Masking-key (continued)       |          Payload Data         |
 +-------------------------------- - - - - - - - - - - - - - - - +
 :                     Payload Data continued ...                :
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 +---------------------------------------------------------------+

*/

#pragma pack(1)
struct WsFrameHeader
{
    enum Opcode
    {
        CONTINUATION = 0x0,
        TEXT = 0x1,
        BINARY = 0x2,
        CLOSE = 0x8,
        PING = 0x9,
        PONG = 0xA,
        ERROR = 0xF
    };

    uint32_t opcode : 4;
    bool rsv3 : 1;
    bool rsv2 : 1;
    bool rsv1 : 1;
    bool fin : 1;
    uint32_t payload_len : 7;
    bool mask : 1;




    std::string toString() const;
};

#pragma pack()

class WsFrameMessage
{
public:
    typedef std::shared_ptr<WsFrameMessage> Ptr;

    WsFrameMessage(const uint8_t opcode, std::string data = "")
        : _opcode(opcode)
        , _data(data) {}

    void set_opcode(const uint8_t opcode) { _opcode = opcode; }
    void set_data(const std::string& data) { _data = data; }

    uint8_t get_opcode() const { return _opcode; }
    std::string get_data() const { return _data; }
    std::string& get_data() { return _data; }

private:
    uint8_t _opcode;
    std::string _data;
};

class WsConnection : public HttpConnection
{
public:
    typedef std::shared_ptr<WsConnection> Ptr;
    WsConnection(Socket::Ptr sock, bool owner = true)
        : HttpConnection(sock, owner) {}

    HttpRequest::Ptr handleShake();

    WsFrameMessage::Ptr recvMessage();
    int32_t sendMessage(const WsFrameMessage::Ptr& msg, bool is_fin = true);
    int32_t sendMessage(const std::string& data, const uint8_t opcode = WsFrameHeader::TEXT,
                        bool is_fin = true);

    int32_t ping();
    int32_t pong();

private:
    bool handleServerShake();
    bool handleClientShake();
};

WsFrameMessage::Ptr WsRecvMessage(SocketStream* stream, bool is_client);
int32_t WsSendMessage(SocketStream* stream, const WsFrameMessage::Ptr& message, bool is_client,
                      bool is_fin);
int32_t WsPing(SocketStream* stream);
int32_t WsPong(SocketStream* stream);

}   // namespace pico

#endif