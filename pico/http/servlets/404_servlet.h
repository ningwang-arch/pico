#ifndef __PICO_HTTP_404_SERVLET_H__
#define __PICO_HTTP_404_SERVLET_H__

#include "../servlet.h"

#include "../../class_factory.h"

namespace pico {

class NotFoundServlet : public Servlet
{
public:
    NotFoundServlet() = default;
    ~NotFoundServlet() = default;

    void doGet(const request& req, response& res) override {
        res->set_status(HttpStatus::NOT_FOUND);
        res->set_header("Content-Type", "text/html");
        res->set_header("Server", "pico/1.0.0");
        res->set_body("<html>"
                      "<head>"
                      "<title>404 Not Found</title>"
                      "</head>"
                      "<body>"
                      "<center><h1>404 Not Found</h1></center>"
                      "<hr>"
                      "<center>Pico</center>"
                      "</body>"
                      "</html>");
    }
    void doPost(const request& req, response& res) override {
        res->set_status(HttpStatus::NOT_FOUND);
        res->set_header("Content-Type", "text/html");
        res->set_header("Server", "pico/1.0.0");
        res->set_body("<html>"
                      "<head>"
                      "<title>404 Not Found</title>"
                      "</head>"
                      "<body>"
                      "<center><h1>404 Not Found</h1></center>"
                      "<hr>"
                      "<center>Pico</center>"
                      "</body>"
                      "</html>");
    }
};

REGISTER_CLASS(NotFoundServlet);

}   // namespace pico
#endif