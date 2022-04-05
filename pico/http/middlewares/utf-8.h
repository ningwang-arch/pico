#ifndef __PICO_HTTP_MIDDLEWARES_UTF8_H__
#define __PICO_HTTP_MIDDLEWARES_UTF8_H__

#include "../../singleton.h"
#include "../http.h"

namespace pico {

struct UTF8
{
    struct context
    {};

    // typedef Singleton<context> Context;

    void before_handle(HttpRequest::Ptr& req, HttpResponse::Ptr& resp, context& ctx) {}

    void after_handle(HttpRequest::Ptr& req, HttpResponse::Ptr& resp, context& ctx) {
        if (resp->get_header("Content-Type").empty()) {
            resp->set_header("Content-Type", "text/plain; charset=utf-8");
        }
    }
};

}   // namespace pico

#endif