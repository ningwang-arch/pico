#include "pico/pico.h"

void test() {
    pico::HttpServer::Ptr server(new pico::HttpServer(true));
    server->setName("pico");

    pico::Address::Ptr addr = pico::Address::LookupAnyIPAddress("0.0.0.0:8080");
    if (!addr) {
        LOG_INFO("not found");
        return;
    }
    server->bind(addr);

    auto handler = server->getRequestHandler();
    handler->addRoute("/add",
                      pico::HttpMethod::POST,
                      [](const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) {
                          LOG_INFO("add");
                          int a = std::atoi(req->get_param("a").c_str());
                          int b = std::atoi(req->get_param("b").c_str());

                          resp->set_status(pico::HttpStatus::OK);
                          resp->set_body(std::to_string(a + b));
                      });

    server->start();
}

int main(int argc, char const* argv[]) {
    pico::IOManager iom(2);
    iom.schedule(test);
    return 0;
}
