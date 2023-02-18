
#include "../pico/pico.h"

#include <chrono>
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

        auto session = req->get_session();

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

        auto session = req->get_session();

        auto value = session->get<int>("abc");

        res->set_body(std::to_string(value));
    }
};



class FuzzyMatchServlet : public Servlet
{
public:
    void doGet(const request& req, response& res) override {
        res->set_status(HttpStatus::OK);
        res->set_header("Content-Type", "text/plain");

        std::string pattern = "/fuzzy/<int>";

        auto func = [](int i) { return std::to_string(i); };

        pico::handle(req, res, pattern, func);

        // function_call(req,res,pattern,lambda_function_handler);
        /**
         * example:
         * req:
         * GET /fuzzy/123 HTTP/1.1
         * Host: localhost:8080
         *
         * pattern: /fuzzy/<int>
         *
         * lambda_function_handler:
         * [&](int id) {
         *   res->set_body(std::to_string(id));
         * }
         *
         *
         * function_call(req,res,"fuzzy/<int>",[&](int id) {
         *  res->set_body(std::to_string(id));
         * });
         *
         */

        // auto variant = split_variant(uri, pattern);

        // int id = boost::get<int>(variant[0]);

        // res->set_body(std::to_string(id));
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

class HelloMiddleware : public Middleware
{
public:
    void before_request(request& req, response& res) override {
        m_start_time = std::chrono::system_clock::now();
    }

    void after_response(request& req, response& res) override {
        auto end_time = std::chrono::system_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end_time - m_start_time);
        LOG_INFO("request url: %s, duration: %dus", req->get_path().c_str(), duration.count());
    }


private:
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
};

REGISTER_CLASS(HelloServlet);
REGISTER_CLASS(SessionSetServlet);
REGISTER_CLASS(SessionGetServlet);
REGISTER_CLASS(MustacheServlet);
REGISTER_CLASS(HelloFilter);
REGISTER_CLASS(TestFilter);
REGISTER_CLASS(WsHelloServlet);
REGISTER_CLASS(FuzzyMatchServlet);
REGISTER_CLASS(HelloMiddleware);

}   // namespace pico


int main(int argc, char* argv[]) {
    pico::Application app;

    pico::compression::set_compression_enabled(true);
    // pico::set_log_enabled(false);
    if (app.init(argc, argv)) {
        app.run();
    }

    return 0;
}
