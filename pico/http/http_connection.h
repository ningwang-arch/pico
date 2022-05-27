#ifndef __PICO_HTTP_HTTP_CONNECTION_H__
#define __PICO_HTTP_HTTP_CONNECTION_H__

#include "../socket_stream.h"
#include "../uri.h"
#include "http.h"
#include "http_parser.h"

#include <map>
#include <memory>
#include <string>

namespace pico {
class HttpConnection : public SocketStream
{
public:
    typedef std::shared_ptr<HttpConnection> Ptr;
    HttpConnection(Socket::Ptr sock, bool owner = true);

    HttpRequest::Ptr recvRequest();
    int sendResponse(HttpResponse::Ptr resp);

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
}   // namespace pico

#endif