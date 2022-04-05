#include "pico/http/middlewares/cors.h"
#include "pico/http/middlewares/session.h"
#include "pico/http/middlewares/utf-8.h"
#include "pico/pico.h"

void test() {
    // pico::HttpServer<pico::UTF8>::Ptr server(new pico::HttpServer<pico::UTF8>(true));
    // pico::HttpServer<>::Ptr server(new pico::HttpServer<>(true));
    pico::HttpServer<pico::CORSHandler, pico::UTF8, pico::Session>::Ptr server(
        new pico::HttpServer<pico::CORSHandler, pico::UTF8, pico::Session>(true));
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
                      [&](const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) {
                          LOG_INFO("add");
                          auto session = server->get_context<pico::Session>(req).session;
                          // std::cout << session->json() << std::endl;
                          session->set("name", "pico");
                          int a = std::atoi(req->get_param("a").c_str());
                          int b = std::atoi(req->get_param("b").c_str());

                          resp->set_status(pico::HttpStatus::OK);
                          resp->set_body(std::to_string(a + b));
                      });

    handler->addRoute("/get",
                      pico::HttpMethod::GET,
                      [&](const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& resp) {
                          LOG_INFO("get");
                          auto session = server->get_context<pico::Session>(req).session;
                          if (session == nullptr) {
                              std::cout << "session is null" << std::endl;
                              resp->set_status(pico::HttpStatus::UNAUTHORIZED);
                              return;
                          }
                          std::cout << session->json() << std::endl;
                          auto name = session->get("name");
                          resp->set_status(pico::HttpStatus::OK);
                          resp->set_body(session->get("name").asCString());
                      });

    server->start();
}

int main(int argc, char const* argv[]) {
    pico::IOManager iom(2);
    iom.schedule(test);
    return 0;
}
