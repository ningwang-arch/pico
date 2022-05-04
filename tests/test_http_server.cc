
#include "pico/class_factory.h"
#include "pico/config.h"
#include "pico/pico.h"
#include "pico/session.h"

#ifndef CONF_ROOT
#    define CONF_ROOT "root."
#endif
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


REGISTER_CLASS(HelloServlet);
REGISTER_CLASS(SessionSetServlet);
REGISTER_CLASS(SessionGetServlet);

}   // namespace pico

pico::ConfigVar<std::string>::Ptr server_address = pico::Config::Lookup<std::string>(
    CONF_ROOT + std::string("server.address"), "127.0.0.1", "Server address");
pico::ConfigVar<int>::Ptr server_port =
    pico::Config::Lookup<int>(CONF_ROOT + std::string("server.port"), 8080, "Server port");

void test() {
    pico::Config::LoadFromFile("web.yml");

    pico::HttpServer::Ptr server(new pico::HttpServer(true));

    pico::Address::Ptr addr = pico::Address::LookupAnyIPAddress(
        server_address->getValue() + ":" + std::to_string(server_port->getValue()));

    server->bind(addr);

    server->start();
}

int main(int argc, char const* argv[]) {
    pico::IOManager iom(2);
    iom.schedule(test);
    return 0;
}
