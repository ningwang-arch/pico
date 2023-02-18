#ifndef __PICO_HTTP_MIDDLEWARE_H__
#define __PICO_HTTP_MIDDLEWARE_H__

#include <memory>


#include "http.h"


namespace pico {

using request = pico::HttpRequest::Ptr;
using response = pico::HttpResponse::Ptr;

class Middleware
{
public:
    typedef std::shared_ptr<Middleware> Ptr;

    virtual void before_request(request& /*req*/, response& /*res*/){};

    virtual void after_response(request& /*req*/, response& /*res*/){};

    virtual ~Middleware() {}
};


}   // namespace pico
#endif