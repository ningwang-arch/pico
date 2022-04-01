#include "pico/http/http.h"
#include "pico/logging.h"
#include <iostream>

void test_request() {
    pico::HttpRequest::Ptr req(new pico::HttpRequest);
    req->set_method(pico::HttpMethod::GET);
    req->set_path("/");
    req->set_version("HTTP/1.1");
    req->set_header("Connection", "keep-alive");
    req->set_header("Content-Type", "text/html");
    req->set_body("hello world");
    std::cout << req->to_string() << std::endl;
}

void test_response() {
    pico::HttpResponse::Ptr resp(new pico::HttpResponse);
    resp->set_status(pico::HttpStatus::OK);
    resp->set_version("HTTP/1.1");
    resp->set_header("Connection", "keep-alive");
    resp->set_header("Content-Type", "text/html");
    resp->set_body("hello world");
    std::cout << resp->to_string() << std::endl;
}

int main(int argc, char const* argv[]) {
    test_request();
    test_response();
    return 0;
}
