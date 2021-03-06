
#include "pico/application.h"
#include "pico/class_factory.h"
#include "pico/compression.h"
#include "pico/config.h"
#include "pico/filter.h"
#include "pico/mustache.h"
#include "pico/pico.h"
#include "pico/session.h"
#include "pico/ws/ws_servlet.h"
#include <fstream>

namespace pico {
class HelloServlet : public Servlet
{
public:
    void doGet(const request& req, response& res) override {
        res->set_status(HttpStatus::OK);
        res->set_header("Content-Type", "text/plain");
        res->set_body("Hello, World!");
    }
};

class SessionSetServlet : public Servlet
{
public:
    void doGet(const request& req, response& res) override {
        res->set_status(HttpStatus::OK);
        res->set_header("Content-Type", "text/plain");

        auto session = SessionManager::getInstance()->getRequestSession(req, res);

        session->set("abc", 1);

        res->set_body("OK");
    }
};


class SessionGetServlet : public Servlet
{
public:
    void doGet(const request& req, response& res) override {
        res->set_status(HttpStatus::OK);
        res->set_header("Content-Type", "text/plain");

        auto session = SessionManager::getInstance()->getRequestSession(req, res);

        auto value = session->get<int>("abc");

        res->set_body(std::to_string(value));
    }
};

class MustacheServlet : public Servlet
{
public:
    void doGet(const request& req, response& res) override {
        int a = std::stoi(req->get_param("a"));
        int b = std::stoi(req->get_param("b"));

        Json::Value json;
        json["name"] = "pico";
        json["a"] = a;
        json["b"] = b;
        json["ret"] = a + b;
        auto tpl = mustache::load("test.tpl").render(json);
        res->write(tpl);
    }
};

class HelloFilter : public Filter
{
public:
    void doFilter(const HttpRequest::Ptr& request, HttpResponse::Ptr& response,
                  FilterChain::Ptr chain) override {
        std::cout << "HelloFilter::doFilter" << std::endl;
        chain->doFilter(request, response);
    }
};

class TestFilter : public Filter
{
public:
    void init(const FilterConfig::Ptr& config) override {
        std::cout << "TestFilter::init" << std::endl;
        m_init_params = config->getInitParams();
    }

    void doFilter(const HttpRequest::Ptr& request, HttpResponse::Ptr& response,
                  FilterChain::Ptr chain) override {
        std::cout << "TestFilter::doFilter" << std::endl;
        for (auto& param : m_init_params) {
            std::cout << param.first << "=" << param.second << std::endl;
        }
        return;
        // chain->doFilter(request, response);
    }

private:
    std::map<std::string, std::string> m_init_params;
};

class WsHelloServlet : public WsServlet
{
public:
    bool onConnect(const HttpRequest::Ptr& req) override {
        std::cout << req->to_string() << std::endl;
        std::cout << "WsHelloServlet::onConnect" << std::endl;
        return true;
    }
    void onMessage(const WsConnection::Ptr& conn, WsFrameMessage::Ptr& msg) override {
        conn->sendMessage(msg);
    }
    void onDisconnect(const WsConnection::Ptr& conn) override {
        std::cout << "WsHelloServlet::onDisconnect" << std::endl;
    }
};

REGISTER_CLASS(HelloServlet);
REGISTER_CLASS(SessionSetServlet);
REGISTER_CLASS(SessionGetServlet);
REGISTER_CLASS(MustacheServlet);
REGISTER_CLASS(HelloFilter);
REGISTER_CLASS(TestFilter);
REGISTER_CLASS(WsHelloServlet);

}   // namespace pico

int main(int argc, char* argv[]) {
    pico::Application app;
    pico::compression::set_compression_enabled(true);
    if (app.init(argc, argv)) { app.run(); }


    return 0;
}
