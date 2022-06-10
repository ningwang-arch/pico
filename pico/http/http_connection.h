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
};


}   // namespace pico

#endif