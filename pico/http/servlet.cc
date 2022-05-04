#include "servlet.h"

namespace pico {

void Servlet::service(const request& req, response& res) {
    auto method = req->get_method();
    if (method == HttpMethod::GET) { doGet(req, res); }
    else if (method == HttpMethod::POST) {
        doPost(req, res);
    }
    else if (method == HttpMethod::OPTIONS) {
        doOptions(req, res);
    }
    else if (method == HttpMethod::PUT) {
        doPut(req, res);
    }
    else if (method == HttpMethod::DELETE) {
        doDelete(req, res);
    }
    else if (method == HttpMethod::HEAD) {
        doHead(req, res);
    }
    else {
        res->set_status(HttpStatus::NOT_IMPLEMENTED);
        res->set_header("Content-Type", "text/html");
        res->set_header("Server", "pico/1.0.0");
        res->set_body("<html>"
                      "<head>"
                      "<title>501 Not Implemented</title>"
                      "</head>"
                      "<body>"
                      "<center><h1>501 Not Implemented</h1></center>"
                      "<hr>"
                      "<center>Pico</center>"
                      "</body>"
                      "</html>");
    }
}

void Servlet::doGet(const request& req, response& res) {
    sendMethodNotAllowed(req, res);
}

void Servlet::doPost(const request& req, response& res) {
    sendMethodNotAllowed(req, res);
}

void Servlet::doPut(const request& req, response& res) {
    sendMethodNotAllowed(req, res);
}

void Servlet::doDelete(const request& req, response& res) {
    sendMethodNotAllowed(req, res);
}

void Servlet::doHead(const request& req, response& res) {
    doGet(req, res);
    res->set_body("");
}

void Servlet::doOptions(const request& req, response& res) {
    res->set_header("Access-Control-Allow-Origin",
                    req->get_header("Origin").empty() ? "*" : req->get_header("Origin"));
    res->set_header("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
    res->set_header("Access-Control-Allow-Headers",
                    "Content-Type,Authorization,Accept,X-Requested-With,X-File-Name,X-"
                    "File-Size,X-File-Type,X-File-Ext");
    res->set_header("Access-Control-Max-Age", "1728000");
    res->set_header("Access-Control-Allow-Credentials", "true");
    res->set_header("Access-Control-Expose-Headers", "Content-Disposition");
    res->set_header("Content-Type", "text/plain");
    res->set_status((HttpStatus)200);
    res->set_body("");
}

void Servlet::sendMethodNotAllowed(const request& req, response& res) {
    res->set_status(HttpStatus::METHOD_NOT_ALLOWED);
    res->set_header("Content-Type", "text/html");
    res->set_header("Server", "pico/1.0.0");
    res->set_body("<html>"
                  "<head>"
                  "<title>405 Method Not Allowed</title>"
                  "</head>"
                  "<body>"
                  "<center><h1>405 Method Not Allowed</h1></center>"
                  "<hr>"
                  "<center>Pico</center>"
                  "</body>"
                  "</html>");
}

}   // namespace pico