
#include "pico/application.h"
#include "pico/class_factory.h"
#include "pico/config.h"
#include "pico/pico.h"
#include "pico/session.h"
#include <fstream>

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

// /home/eclipse/code/vscode/pico/conf
// /home/eclipse/code/vscode/pico/conf/tmp.yml

int main(int argc, char* argv[]) {
    pico::Application app;
    if (app.init(argc, argv)) { app.run(); }


    return 0;
}
