#include "pico/logging.h"
#include "pico/ws/ws_server.h"


class WsServlet : public pico::WsServlet
{
public:
    virtual bool onConnect(const pico::HttpRequest::Ptr& req) override {
        LOG_INFO("WsServlet::onConnect");
        return true;
    }

    virtual void onMessage(const pico::WsConnection::Ptr& conn,
                           pico::WsFrameMessage::Ptr& msg) override {
        conn->sendMessage(msg);
    }

    virtual void onDisconnect(const pico::WsConnection::Ptr& conn) override {
        LOG_INFO("WsServlet::onDisconnect");
    }
};

void test_sha1() {
    std::string str = "hello world";
    std::string sha1 = pico::sha1sum(str);
    sha1 = pico::base64_encode(sha1, false);
    LOG_INFO("sha1: %s", sha1.c_str());
}


void run() {
    pico::WsServer::Ptr server = std::make_shared<pico::WsServer>();
    pico::Address::Ptr addr = pico::Address::LookupAnyIPAddress("127.0.0.1:8080");

    server->bind(addr);

    server->getServletDispatcher()->addServlet("/", std::make_shared<WsServlet>());

    server->start();
}
int main(int argc, char const* argv[]) {
    pico::IOManager io;
    io.schedule(&run);
    // test_sha1();
    return 0;
}
