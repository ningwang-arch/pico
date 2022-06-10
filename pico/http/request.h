#ifndef __PICO_HTTP_REQUEST_H__
#define __PICO_HTTP_REQUEST_H__

#include "../socket_stream.h"
#include "../uri.h"
#include "http.h"
#include "http_parser.h"

#include <map>
#include <memory>
#include <string>

namespace pico {
class Request : public SocketStream
{
public:
    typedef std::shared_ptr<Request> Ptr;

    Request(Socket::Ptr sock, bool owner = true)
        : SocketStream(sock, owner) {}

    int sendRequest(HttpRequest::Ptr req);
    HttpResponse::Ptr recvResponse();

    static HttpResponse::Ptr doGet(const std::string& url,
                                   const std::map<std::string, std::string>& headers = {},
                                   const std::string& body = "", const std::string& proxy = "",
                                   uint64_t timeout = 0);
    static HttpResponse::Ptr doPost(const std::string& url,
                                    const std::map<std::string, std::string>& headers = {},
                                    const std::string& body = "", const std::string& proxy = "",
                                    uint64_t timeout = 0);

    static HttpResponse::Ptr doGet(const Uri::Ptr uri,
                                   const std::map<std::string, std::string>& headers = {},
                                   const std::string& body = "", const std::string& proxy = "",
                                   uint64_t timeout = 0);
    static HttpResponse::Ptr doPost(const Uri::Ptr uri,
                                    const std::map<std::string, std::string>& headers = {},
                                    const std::string& body = "", const std::string& proxy = "",
                                    uint64_t timeout = 0);
    static HttpResponse::Ptr doRequest(const HttpMethod& method, const std::string& url,
                                       const std::map<std::string, std::string>& headers = {},
                                       const std::string& body = "", const std::string& proxy = "",
                                       uint64_t timeout = 0);
    static HttpResponse::Ptr doRequest(const HttpMethod& method, const Uri::Ptr uri,
                                       const std::map<std::string, std::string>& headers = {},
                                       const std::string& body = "", const std::string& proxy = "",
                                       uint64_t timeout = 0);

    static HttpResponse::Ptr doRequest(const HttpRequest::Ptr req, const Uri::Ptr uri,
                                       uint64_t timeout = 0);
    static HttpResponse::Ptr doRequest(const HttpRequest::Ptr req, const Uri::Ptr uri,
                                       const std::string& proxy = "", uint64_t timeout = 0);
};
};   // namespace pico

#endif
