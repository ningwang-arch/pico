#include "pico/http/http.h"
#include "pico/http/http_connection.h"
#include "pico/logging.h"
#include "pico/uri.h"

void test() {
    pico::IPAddress::Ptr addr = pico::Address::LookupAnyIPAddress("www.baidu.com");
    if (addr) { LOG_INFO("%s", addr->to_string().c_str()); }
    else {
        LOG_INFO("not found");
        return;
    }

    pico::Socket::Ptr sock = pico::Socket::CreateTcp(addr);
    addr->setPort(80);
    if (!sock->connect(addr)) {
        LOG_INFO("connect failed");
        return;
    }

    pico::HttpRequest::Ptr req(new pico::HttpRequest());
    req->set_method(pico::HttpMethod::GET);
    req->set_header("Connection", "close");
    req->set_header("Host", "www.baidu.com");
    req->set_path("/");

    LOG_INFO("%s", req->to_string().c_str());
    pico::HttpConnection::Ptr conn(new pico::HttpConnection(sock, true));
    int ret = conn->sendRequest(req);
    if (ret < 0) {
        LOG_INFO("send failed, errno=%d, %s", errno, strerror(errno));
        return;
    }
    pico::HttpResponse::Ptr resp = conn->recvResponse();
    if (!resp) {
        LOG_INFO("recv failed");
        return;
    }
    LOG_INFO("%s", resp->to_string().c_str());
}

void test_do_io() {
    std::map<std::string, std::string> headers;
    headers["Connection"] = "close";
    headers["Host"] = "www.baidu.com";
    pico::HttpResponse::Ptr resp = pico::HttpConnection::doGet("http://www.baidu.com", headers);
    if (!resp) {
        LOG_INFO("recv failed");
        return;
    }
    LOG_INFO("%s", resp->to_string().c_str());
}

void test_uri() {
    using pico::Uri;
    Uri::Ptr uri = Uri::Create("http://www.baidu.com/");
    if (!uri) {
        LOG_INFO("uri create failed");
        return;
    }
    LOG_INFO("%s", uri->toString().c_str());
}

int main(int argc, char const* argv[]) {
    // test();
    test_do_io();
    // test_uri();
    return 0;
}
