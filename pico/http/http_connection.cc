#include "http_connection.h"
#include "../logging.h"

#include "pico/config.h"
#include "pico/util.h"

namespace pico {

static pico::ConfigVar<uint64_t>::Ptr g_recvTimeout =
    pico::Config::Lookup<uint64_t>("other.recv.timeout", uint64_t(60 * 1000), "recv timeout");

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

int HttpConnection::sendRequest(HttpRequest::Ptr req) {
    std::string content = req->to_string();
    return write(content.data(), content.size());
}

HttpResponse::Ptr HttpConnection::recvResponse() {
    HttpResponseParser::Ptr parser(new HttpResponseParser);
    uint64_t buff_size = HttpRequestParser::getHttpRequestBufferSize();
    // uint64_t buff_size = 100;
    std::shared_ptr<char> buffer(new char[buff_size + 1], [](char* ptr) { delete[] ptr; });
    char* data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, buff_size - offset);
        if (len <= 0) {
            close();
            return nullptr;
        }
        len += offset;
        data[len] = '\0';
        size_t nparse = parser->parse(data, len, false);
        if (parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if (offset == (int)buff_size) {
            close();
            return nullptr;
        }
        if (parser->isFinished()) { break; }
    } while (true);
    auto& client_parser = parser->getParser();
    std::string body;
    if (client_parser.chunked) {
        int len = offset;
        do {
            bool begin = true;
            do {
                if (!begin || len == 0) {
                    int rt = read(data + len, buff_size - len);
                    if (rt <= 0) {
                        close();
                        return nullptr;
                    }
                    len += rt;
                }
                data[len] = '\0';
                size_t nparse = parser->parse(data, len, true);
                if (parser->hasError()) {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if (len == (int)buff_size) {
                    close();
                    return nullptr;
                }
                begin = false;
            } while (!parser->isFinished());
            // len -= 2;
            if (client_parser.content_len + 2 <= len) {
                body.append(data, client_parser.content_len);
                memmove(data,
                        data + client_parser.content_len + 2,
                        len - client_parser.content_len - 2);
                len -= client_parser.content_len + 2;
            }
            else {
                body.append(data, len);
                int left = client_parser.content_len - len + 2;
                while (left > 0) {
                    int rt = read(data, left > (int)buff_size ? (int)buff_size : left);
                    if (rt <= 0) {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                body.resize(body.size() - 2);
                len = 0;
            }
        } while (!client_parser.chunks_done);
    }
    else {
        int64_t length = parser->getContentLength();
        if (length > 0) {
            body.resize(length);

            int len = 0;
            if (length >= offset) {
                memcpy(&body[0], data, offset);
                len = offset;
            }
            else {
                memcpy(&body[0], data, length);
                len = length;
            }
            length -= offset;
            if (length > 0) {
                if (readFixSize(&body[len], length) < 0) {
                    close();
                    return nullptr;
                }
            }
        }
    }
    if (!body.empty()) { parser->getResponse()->set_body(body); }
    return parser->getResponse();
}

HttpResponse::Ptr HttpConnection::doGet(const std::string& url,
                                        const std::map<std::string, std::string>& headers,
                                        const std::string& body, const std::string& proxy,
                                        uint64_t timeout) {
    return doRequest(HttpMethod::GET, url, headers, body, proxy, timeout);
}

HttpResponse::Ptr HttpConnection::doPost(const std::string& url,
                                         const std::map<std::string, std::string>& headers,
                                         const std::string& body, const std::string& proxy,
                                         uint64_t timeout) {
    return doRequest(HttpMethod::POST, url, headers, body, proxy, timeout);
}

HttpResponse::Ptr HttpConnection::doGet(const Uri::Ptr uri,
                                        const std::map<std::string, std::string>& headers,
                                        const std::string& body, const std::string& proxy,
                                        uint64_t timeout) {
    return doRequest(HttpMethod::GET, uri, headers, body, proxy, timeout);
}

HttpResponse::Ptr HttpConnection::doPost(const Uri::Ptr uri,
                                         const std::map<std::string, std::string>& headers,
                                         const std::string& body, const std::string& proxy,
                                         uint64_t timeout) {
    return doRequest(HttpMethod::POST, uri, headers, body, proxy, timeout);
}

HttpResponse::Ptr HttpConnection::doRequest(const HttpMethod& method, const std::string& url,
                                            const std::map<std::string, std::string>& headers,
                                            const std::string& body, const std::string& proxy,
                                            uint64_t timeout) {
    Uri::Ptr uri = Uri::Create(url);
    if (uri == nullptr) {
        LOG_ERROR("parse url error");
        return nullptr;
    }
    return doRequest(method, uri, headers, body, proxy, timeout);
}

HttpResponse::Ptr HttpConnection::doRequest(const HttpMethod& method, const Uri::Ptr uri,
                                            const std::map<std::string, std::string>& headers,
                                            const std::string& body, const std::string& proxy,
                                            uint64_t timeout) {
    HttpRequest::Ptr req(new HttpRequest());
    req->set_method(method);
    req->set_path(uri->getPath());
    req->set_query(uri->getQuery());
    req->set_body(body);
    req->set_fragment(uri->getFragment());
    for (auto& header : headers) {
        if (strcasecmp(header.first.c_str(), "connection") == 0) {
            if (strcasecmp(header.second.c_str(), "keep-alive") == 0) { req->set_close(false); }
            else {
                req->set_close(true);
            }
        }
        req->set_header(header.first, header.second);
    }
    if (req->get_header("Host") == "") { req->set_header("Host", uri->getHost()); }

    if (proxy.empty()) { return doRequest(req, uri, timeout); }
    else {
        return doRequest(req, uri, proxy, timeout);
    }
}

HttpResponse::Ptr HttpConnection::doRequest(const HttpRequest::Ptr req, const Uri::Ptr uri,
                                            uint64_t timeout) {
    if (req == nullptr) {
        LOG_ERROR("request is null");
        return nullptr;
    }
    if (uri == nullptr) {
        LOG_ERROR("uri is null");
        return nullptr;
    }
    bool is_ssl = uri->getScheme() == "https";
    Address::Ptr addr = uri->getAddress();
    if (addr == nullptr) {
        LOG_ERROR("parse url error");
        return nullptr;
    }
    Socket::Ptr sock = is_ssl ? SSLSocket::CreateTcp(addr) : Socket::CreateTcp(addr);
    if (sock == nullptr) {
        LOG_ERROR("create socket error");
        return nullptr;
    }
    if (!sock->connect(addr)) {
        LOG_ERROR("connect to %s:%d error", uri->getHost().c_str(), uri->getPort());
        return nullptr;
    }
    if (timeout == 0) { timeout = g_recvTimeout->getValue(); }
    sock->setRecvTimeout(timeout);
    HttpConnection::Ptr conn = std::make_shared<HttpConnection>(sock);
    if (!conn->sendRequest(req)) {
        LOG_ERROR("send request error");
        return nullptr;
    }
    HttpResponse::Ptr resp = conn->recvResponse();
    if (resp == nullptr) {
        LOG_ERROR("recv response error");
        return nullptr;
    }
    return resp;
}

HttpResponse::Ptr HttpConnection::doRequest(const HttpRequest::Ptr req, const Uri::Ptr uri,
                                            const std::string& proxy, uint64_t timeout) {
    if (req == nullptr) {
        LOG_ERROR("request is null");
        return nullptr;
    }
    if (uri == nullptr) {
        LOG_ERROR("uri is null");
        return nullptr;
    }
    int pos = proxy.find(":");
    if (pos == -1) {
        LOG_ERROR("http_proxy format error");
        return nullptr;
    }
    std::string host = proxy.substr(0, pos);
    uint16_t port = std::stoi(proxy.substr(pos + 1));
    auto addr = IPAddress::Create(host.data(), port);
    if (addr == nullptr) {
        LOG_ERROR("parse http_proxy error");
        return nullptr;
    }
    Socket::Ptr sock = Socket::CreateTcp(addr);
    if (sock == nullptr) {
        LOG_ERROR("create socket error");
        return nullptr;
    }
    if (!sock->connect(addr)) {
        LOG_ERROR("connect to %s error", proxy.c_str());
        return nullptr;
    }
    HttpRequest::Ptr conn_req = std::make_shared<HttpRequest>();
    conn_req->set_method(HttpMethod::CONNECT);
    conn_req->set_path(proxy);

    if (timeout == 0) { timeout = g_recvTimeout->getValue(); }
    sock->setRecvTimeout(timeout);
    HttpConnection::Ptr conn = std::make_shared<HttpConnection>(sock);
    if (!conn->sendRequest(conn_req)) {
        LOG_ERROR("send request error");
        return nullptr;
    }
    HttpResponse::Ptr resp = conn->recvResponse();
    if (resp == nullptr) {
        LOG_ERROR("recv response error");
        return nullptr;
    }
    if (resp->get_status() != HttpStatus(200)) {
        LOG_ERROR("connect to %s error", proxy.c_str());
        return nullptr;
    }
    req->set_path(uri->toString());
    if (!conn->sendRequest(req)) {
        LOG_ERROR("send request error");
        return nullptr;
    }
    auto resp2 = conn->recvResponse();
    if (resp2 == nullptr) {
        LOG_ERROR("recv response error");
        return nullptr;
    }
    return resp2;
}

}   // namespace pico