<h1 align="center">Pico</h1>

## Description

Pico is a C++ framework for creating web applications. It is designed to be easy to use and easy to learn.

### Features

- Easy Routing
- Uses Morden C++11
- Http/1.1 support
- Middleware support for extensions

### Still to do

- HTTPS support
- WebSocket support

### Dependencies

- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

### Note

This project is completed under Linux and does not support Windows

## How to build

```bash
$ git clone https://github.com/ningwang-arch/pico.git
$ cd pico
$ mkdir build
$ cd build
$ cmake ..
$ make
```

The binary is located in the bin/ directory.
Also, the source code is available in the pico/ directory.
Some examples are available in the tests/ directory.

## Examples

#### Hello World

```c++
#include "pico/pico.h"

void run(){
    pico::HttpServer<>::Ptr server(new pico::HttpServer<>(true));

    pico::Address::Ptr addr = pico::Address::LookupAnyIPAddress("0.0.0.0:8080");
    if (!addr) {
        LOG_INFO("Could not find any IP address");
        return;
    }
    server->bind(addr);

    auto handler = server->getRequestHandler();

    handler->addRoute("/",
                      pico::HttpMethod::GET,
                      [](const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res) {
                          res->set_status(pico::HttpStatus::OK);
                          res->set_header("Content-Type", "text/plain");
                          res->set_body("Hello World");
                      });

    server->start();
}

int main(int argc, const char* argv[]){
    pico::IOManager iom(2);
    iom.scheduler(run);
    return 0;
}

```

#### Middleware

```c++
#include "pico/pico.h"

void run(){
    pico::HttpServer<pico::Session>::Ptr server(new pico::HttpServer<pico::Session>(true));

    pico::Address::Ptr addr = pico::Address::LookupAnyIPAddress("0.0.0.0:8080");
    if (!addr) {
        LOG_INFO("Could not find any IP address");
        return;
    }

    server->bind(addr);
    auto handler = server->getRequestHandler();

    handler->addRoute("/save",
                      pico::HttpMethod::GET,
                      [](const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res){
                          auto session = server->get_context<pico::Session>(req).session;
                          session->set("name", "pico");
                      });
    handler->addRoute("/get",
                      pico::HttpMethod::GET,
                      [](const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res){
                          auto session = server->get_context<pico::Session>(req).session;
                          if(session->has("name")){
                              auto name = session->get("name");
                              res->set_status(pico::HttpStatus::OK);
                              res->set_body(session->get("name").asCString());
                          }
                      });
    server->start();
}

int main(int argc, char const* argv[]){
    pico::IOManager iom(2);
    iom.scheduler(run);

    return 0;
}
```

There are three middlewares in pico: UTF8, CORS, Session.<br>
If you want to use middleware which is created by yourself, your middleware must have structure like this:

```c++
struct MyMiddleware{
    struct context{
        // ...
    };

    void before_handle(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res, context& ctx){
        // ...
    }

    void after_handle(const pico::HttpRequest::Ptr& req, pico::HttpResponse::Ptr& res,context& ctx){
        // ...
    }
};
```

Then just add your middleware to the server template list.

#### Logging

The logging system is expired by Log4j.
Due to tight time, the configuration module is not implemented.
The default logging layout is PatternLayout with format pattern:

```
%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%T%m%n
```

and the result is like this:

```
2022-04-06 16:21:04     66932   _0      3       [INFO]  [root]  pico/tcp_server.cc:77           server is listening on [family: 2, type: 1, protocol: 0], addr: 0.0.0.0:8080
```

```c++
LOG_DEBUG("msg fmt",msg...);
LOG_INFO("msg fmt",msg...);
LOG_WARN("msg fmt",msg...);
LOG_ERROR("msg fmt",msg...);
LOG_FATAL("msg fmt",msg...);
```

If you want to create a new layout, please implement Layout interface and override the format() method.

Use the following code to create a new layout and set it to the logger.

```c++
Layout::Ptr layout(new MyLayout());
g_logger->setLayout(layout);
```

#### Other

Thanks to the [sylar](https://github.com/sylar-yin/sylar), [Crow](https://github.com/CrowCpp/Crow) and [mongrel2](https://github.com/mongrel2/mongrel2), pico is a powerful framework for creating web applications.
