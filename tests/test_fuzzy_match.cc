#include "pico/util.h"

#include "pico/common.h"
#include "pico/http/http.h"

#include <iostream>


void test() {
    pico::HttpRequest::Ptr req = std::make_shared<pico::HttpRequest>();
    pico::HttpResponse::Ptr res = std::make_shared<pico::HttpResponse>();

    req->set_path("/user/1/abc");

    std::string pattern = "/user/<int>/<string>";

    // req res int string
    // auto func =
    //     [](const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res, int i, std::string s)
    //     {
    //         res->set_body("abc");
    //     };

    // req int string
    // auto func = [](const pico::HttpRequest::Ptr& req, int i, std::string s) { return "abc"; };

    // res int string
    auto func = [](pico::HttpResponse::Ptr& res, int i, std::string s) { res->set_body("abc"); };

    // // int string
    // auto func = [&](int i, std::string s) {
    //     std::cout << i << " " << s << std::endl;
    //     return "abc";
    // };

    pico::handle(req, res, pattern, func);

    std::cout << res->to_string() << std::endl;
}



int main() {

    test();




    return 0;
}
