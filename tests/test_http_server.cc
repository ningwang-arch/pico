#include "pico/http/middlewares/cors.h"
#include "pico/http/middlewares/utf-8.h"
#include "pico/pico.h"

void test() {
    // pico::HttpServer<pico::UTF8>::Ptr server(new pico::HttpServer<pico::UTF8>(true));
    // pico::HttpServer<>::Ptr server(new pico::HttpServer<>(true));
    pico::HttpServer<pico::CORSHandler>::Ptr server(new pico::HttpServer<pico::CORSHandler>(true));
    server->setName("pico");

    // server->getContext<pico::CORSHandler>();
    // server->getContext<pico::UTF8>();

    auto cors = server->get_middleware<pico::CORSHandler>();
    cors.global()
        .headers("X-Custom-Header", "Upgrade-Insecure-Requests")
        .prefix("/")
        .origin("example.com")
        .prefix("/add")
        .ignore();

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
