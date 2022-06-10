#include "http_connection.h"
#include "../logging.h"

#include "pico/config.h"
#include "pico/util.h"

namespace pico {



HttpConnection::HttpConnection(Socket::Ptr sock, bool owner)
    : SocketStream(sock, owner) {}

HttpRequest::Ptr HttpConnection::recvRequest() {
    HttpRequestParser::Ptr parser(new HttpRequestParser());
    uint64_t buff_size = HttpRequestParser::getHttpRequestBufferSize();
    std::shared_ptr<char> buff(new char[buff_size], std::default_delete<char[]>());
    char* data = buff.get();
    int offset = 0;
    do {
        int len = read(data + offset, buff_size - offset);
        if (len <= 0) {
            close();
            return nullptr;
        }
        len += offset;
        size_t nparsed = parser->parse(data, len);
        offset = len - nparsed;
        if (parser->hasError()) {
            LOG_ERROR("parse http request error");
            close();
            return nullptr;
        }
        if (parser->isFinished()) { break; }
    } while (true);
    uint64_t length = parser->getContentLength();
    if (length > 0) {
        std::string body;
        body.resize(length);
        int len = 0;
        if ((int)length < offset) {
            memcpy(&body[0], data, length);
            len = length;
        }
        else {
            memcpy(&body[0], data, offset);
            len = offset;
        }
        length -= offset;
        if (length > 0) {
            if (readFixSize(&body[len], length) < 0) {
                close();
                return nullptr;
            }
        }
        parser->getRequest()->set_body(body);
    }
    parser->getRequest()->init();
    return parser->getRequest();
}

int HttpConnection::sendResponse(HttpResponse::Ptr resp) {
    std::string content = resp->to_string();
    return write(content.data(), content.size());
}


}   // namespace pico