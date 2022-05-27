#ifndef __PICO_HTTP_SERVLET_H__
#define __PICO_HTTP_SERVLET_H__

#include <memory>

#include "http.h"

namespace pico {

using request = pico::HttpRequest::Ptr;
using response = pico::HttpResponse::Ptr;

class Servlet
{
public:
    typedef std::shared_ptr<Servlet> Ptr;

    virtual void service(const request& req, response& res);

    virtual void doGet(const request& req, response& res);

    virtual void doPost(const request& req, response& res);

    virtual void doPut(const request& req, response& res);

    virtual void doDelete(const request& req, response& res);

    virtual void doHead(const request& req, response& res);

    virtual void doOptions(const request& req, response& res);

public:
    std::string name = "";

private:
    void sendMethodNotAllowed(const request& req, response& res);
};
}   // namespace pico

#endif
