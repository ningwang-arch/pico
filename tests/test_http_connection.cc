#include "pico/http/http.h"
#include "pico/http/request.h"
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
    pico::Request::Ptr conn(new pico::Request(sock, true));
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
    pico::HttpResponse::Ptr resp = pico::Request::doGet("http://www.baidu.com", headers);
    if (!resp) {
        LOG_INFO("recv failed");
        return;
    }
    LOG_INFO("%s", resp->to_string().c_str());
}

// only support http proxy
void test_proxy() {
    std::map<std::string, std::string> proxies;
    std::string proxy = "127.0.0.1:7890";
    std::map<std::string, std::string> headers;
    headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
                            "like Gecko) Chrome/80.0.3987.132 Safari/537.36";
    auto resp = pico::Request::doGet("https://www.google.com", headers, "", proxy);
    if (!resp) {
        LOG_INFO("recv failed");
        return;
    }
    LOG_INFO("%s", resp->to_string().c_str());
}

void test_uri() {
    using pico::Uri;
    Uri::Ptr uri = Uri::Create("http://www.baidu.com/abc?a=1&b=2#c=3");
    if (!uri) {
        LOG_INFO("uri create failed");
        return;
    }
    LOG_INFO("%s", uri->toString().c_str());
}

int main(int argc, char const* argv[]) {
    // test();
    // test_do_io();
    // test_uri();
    test_proxy();
    return 0;
}
