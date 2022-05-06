<h1 align="center">Pico</h1>

## Description

Pico is a C++ framework for creating web applications. It is designed to be easy to use and easy to learn.

### Features

- Easy Routing
- Uses Morden C++11
- Http/1.1 support

### Still to do

- HTTPS support
- WebSocket support

### Dependencies

- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)

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

using request = pico::HttpRequest::Ptr;
using response = pico::HttpResponse::Ptr;

class HelloWorld : public pico::Servlet {
public:
    void doGet(const request& req, const response& res)  override{
        res->set_status(pico::HttpStatus::OK);
        res->set_header("Content-Type", "text/plain");
        res->set_body("Hello World");
    }
};

REGISTER_CLASS(HelloWorld);

void run(){
    pico::Config::LoadFromFile("web.yml");

    pico::HttpServer::Ptr server(new pico::HttpServer(true));

    pico::Address::Ptr addr = pico::Address::LookupAnyIPAddress("0.0.0.0:8080");
    if (!addr) {
        LOG_INFO("Could not find any IP address");
        return;
    }
    server->bind(addr);

    server->start();
}

int main(int argc, const char* argv[]){
    pico::IOManager iom(2);
    iom.scheduler(run);
    return 0;
}

```

#### Request Handle

Same as servlet, you must inherit from pico::Servlet, and then override the handle method you want to handle the request.
Just like servlet, you can use the pico::HttpRequest::Ptr and pico::HttpResponse::Ptr objects to handle the request.
A simple example is shown below.
After implementing the handle method, you can use the `REGISTER_CLASS` to register the servlet.

```c++
using request = pico::HttpRequest::Ptr;
using response = pico::HttpResponse::Ptr;
class MyServlet : public pico::Servlet {
public:
    void doGet(const request& req, const response& res)  override{
        res->set_status(pico::HttpStatus::OK);
        res->set_header("Content-Type", "text/plain");
        res->set_body("Hello World");
    }
};
REGISTER_CLASS(MyServlet);
```

If you want to use the servlet, you can add configuration in the web.yml file under the servlet section.

```yml
servlet:
  - class: MyServlet
    path: /hello
    name: hello
```

#### Config File

The config file is located in the conf/ directory.

```yaml
root:
  server:
    address: 127.0.0.1
    port: 8080
  servlet:
    - { name: hello, path: /, class: HelloServlet }
    - { name: set, path: /set, class: SessionSetServlet }
    - { name: get, path: /get, class: SessionGetServlet }
  session:
    timeout: 1800
```

The default conf dir is `conf/`, if you want to change it, you can use `#define CONF_DIR "your conf location"` in your main.cpp.
If you conf root is not `root`, you can use `#define CONF_ROOT "your root"` in your main.cpp.

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
