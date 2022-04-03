#include "pico/http/http.h"
#include "pico/http/http_parser.h"
#include "pico/logging.h"
#include <iostream>

void test_req() {
    pico::HttpRequestParser::Ptr parser =
        pico::HttpRequestParser::Ptr(new pico::HttpRequestParser());
    std::string req =
        "POST / HTTP/1.1\r\n"
        "Host: www.baidu.com\r\n"
        "Connection: keep-alive\r\n"
        "Cache-Control: max-age=0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/31.0.1650.63 Safari/537.36\r\n"
        "Accept-Encoding: gzip,deflate,sdch\r\n"
        "Accept-Language: zh-CN,zh;q=0.8\r\n"
        "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3\r\n"
        "Coookie: BAIDUID=C0D8E8E9E9E9E9E9E9E9E9E9E9E9E9E9:FG=1; "
        "BIDUPSID=C0D8E8E9E9E9E9E9E9E9E9E9E9E9E9E9E9; PSTM=1401290961; "
        "Content-Length: 4\r\n"
        "\r\n"
        "test";
    std::string req_str = req;
    LOG_INFO("parse begin");
    int ret = parser->parse(&req[0], req.size());
    if (ret < 0) { printf("parse error\n"); }
    pico::HttpRequest::Ptr request = parser->getRequest();
    LOG_INFO("%s", request->to_string().c_str());
    req.resize(req.size() - ret);
    LOG_INFO("body: %s", req.c_str());
}

void test_resp() {
    pico::HttpResponseParser::Ptr parser =
        pico::HttpResponseParser::Ptr(new pico::HttpResponseParser());
    std::string resp = "HTTP/1.1 200 OK\r\n"
                       "Server: BWS/1.1\r\n"
                       "Date: Mon, 27 Jul 2011 06:50:50 GMT\r\n"
                       "Content-Type: text/html; charset=utf-8\r\n"
                       "Content-Length: 4\r\n"
                       "Connection: keep-alive\r\n"
                       "\r\n"
                       "test";
    LOG_INFO("parse begin");
    int ret = parser->parse(&resp[0], resp.size(), false);
    if (ret < 0) { printf("parse error\n"); }
    pico::HttpResponse::Ptr response = parser->getResponse();
    LOG_INFO("%s", response->to_string().c_str());
    resp.resize(resp.size() - ret);
    LOG_INFO("body: %s", resp.c_str());
}

int main(int argc, char const* argv[]) {
    // test_req();
    test_resp();
    return 0;
}
