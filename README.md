<h1 align="center">Pico</h1>

## Description

Pico is a C++ framework for creating web applications. It is designed to be easy to use and easy to learn.

### Features

- Easy Routing
- Uses Morden C++11
- Http/1.1 support
- Filter and servlet similar to Java
- JWT authentication

### Still to do

- WebSocket support

### Dependencies

- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [openssl](https://www.openssl.org/)

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
    void doGet(const request& req, response& res)  override{
        res->set_status(pico::HttpStatus::OK);
        res->set_header("Content-Type", "text/plain");
        res->set_body("Hello World");
    }
};
REGISTER_CLASS(MyServlet);
```

If you want to use the servlet, you can add configuration in the servlets.yml file under the servlet section.

```yml
servlets:
  - class: MyServlet
    path: /hello
    name: hello
```

#### Filter
Similar to servlet, you must inherit from pico::Filter, and then override the doFilter method.

```c++
using request = pico::HttpRequest::Ptr;
using response = pico::HttpResponse::Ptr;
class MyFilter : public pico::Filter {
public:
    void doFilter(const request& req, response& res, pico::FilterChain& chain) override{
      // do something
      chain.doFilter(req, res);
    }
};
REGISTER_CLASS(MyFilter);
```
If you want to use the filter, you can add configuration in the filters.yml file under the filter section.

```yml
filters:
  - name: "filter1"
    class: MyFilter
    description: "Filter1 description"
    path:
      - /
      - /set
    init_params:
      param1: "value1"
      param2: "value2"

```

#### Config File

The config file is located in the conf/ directory.

```yaml
# server
servers:
  - addresses: ["127.0.0.1:8080", "127.0.0.1:11223"]
    type: http
    ssl: false
    name: server
    worker: worker
    keep_alive: false
    acceptor: acceptor
    servlets:
      - hello
  - addresses: ["127.0.0.1:12233"]
    type: http
    ssl: true
    name: server_1
    servlets:
      - hello
    certificates:
      file: conf/cert.pem
      key: conf/key.pem

# servlet
servlets:
  - class: MyServlet
    path: /hello
    name: hello
  
# filter
filters:
  - name: "filter1"
    class: HelloFilter
    description: "Filter1 description"
    path:
      - /
      - /set
    init_params:
      param1: "value1"
      param2: "value2"

```
All configuration files are in yaml format, and can be loaded by pico::Config::LoadFromFile.
All conf files should be in the conf/ directory.

Default conf dir is `${PWD/conf`, but you can change it by add a `-c conf_dir` option to the command line.

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

Use the following code to create / add a new layout to the logger.
``` yml
logs:
  - name: root
    level: info
    appenders:
      - type: console
        layout: my_layout
      - type: file
        filename: logs/app.log
        layout: pattern
        pattern: "[%d{%Y-%m-%d %H:%M:%S}] %p %m"
```
Don't forget to modify pico/logging.cc to add the new layout.

#### JWT
Same to java, you can use the JWT to create a token.

```c++
// create a token
pico::Algorithm::Ptr alg = pico::Algorithm::HMAC256(TOKEN_SECRET);
pico::JWTCreator::Builder::Ptr builder = pico::JWTCreator::init();

builder->withHeader(Json::Value(Json::objectValue))
        ->withKeyId("keyId")
        ->withIssuer("issuer")
        ->withSubject("subject")
        ->withAudience(std::vector<std::string>{"audience1", "audience2"})
        ->withExpiresAt(pico::Date() + 3)
        ->withNotBefore(pico::Date() /* + 30*/)
        ->withIssuedAt(pico::Date())
        ->withJWTId("jwtId")
        ->withClaim("name", "value")

std::string token = builder->sign(alg);

// verify the token
try{
  pico::Algorithm::Ptr alg = pico::Algorithm::HMAC256("privateKey");
  auto verifier = pico::JWT::require(alg)->build();
  auto decoder = verifier->verify(token);
}catch(...){
  // do something
}

```
when some error occurs, you can get the error message by the exception.
More usage about jwt module just same as java.

### Mustache
An example of mustache template engine.
```html
<!--test.tpl-->
<!DOCTYPE html>
<html>
<head>
    <title>Test</title>
</head>
<body>
    <h1>Test</h1>
    <p>This is a test of the <strong>{{ name }}</strong> template.</p>
    <div>
        {{ a}} + {{ b }} = {{ ret }}
    </div>
</body>
</html>

```
Then you can use the following code to render the template.
```c++
Json::Value ctx;
ctx["name"] = "mustache";
ctx["a"] = 1;
ctx["b"] = 2;
ctx["ret"] = ctx["a"].asInt() + ctx["b"].asInt();
pico::mustache::RenderedTemplate tpl= pico::mustache::load("test.tpl").render(ctx);
// you can write the template to response directly, like this:
res->write(tpl);

// or if you just want to get the template string, you can use the following code:
std::string tpl_str = tpl.dump();
// or
std::string tpl_str2= pico::mustache::load("test.tpl").render_string(ctx);
```
The default template directory is `${PWD}/templates`, but you can change it by add a conf in the config file `other.yml` like this:
```yaml
other:
  templates:
    dir: "templates"
```
More usage about mustache module you can see the files in the `templates/` directory.

###### Generate certificate

```
$ openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes
```

#### Other

Thanks to the [sylar](https://github.com/sylar-yin/sylar), [Crow](https://github.com/CrowCpp/Crow) and [mongrel2](https://github.com/mongrel2/mongrel2), pico is a powerful framework for creating web applications.
