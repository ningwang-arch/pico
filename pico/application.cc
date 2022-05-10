#include "application.h"

#include <boost/program_options.hpp>

#include "class_factory.h"
#include "daemon.h"
#include "macro.h"

#include "http/servlets/404_servlet.h"

namespace po = boost::program_options;

namespace pico {


struct Route
{
    std::string path;
    Servlet::Ptr servlet;
};

template<>
class LexicalCast<std::string, std::unordered_map<std::string, Route>>
{
public:
    std::unordered_map<std::string, Route> operator()(const std::string& str) const {
        std::unordered_map<std::string, Route> routes;
        YAML::Node node = YAML::Load(str);
        for (auto route : node) {
            Route r;
            r.path = route["path"].as<std::string>();
            r.servlet = std::static_pointer_cast<Servlet>(
                ClassFactory::Instance().Create(route["class"].as<std::string>()));
            if (r.servlet == nullptr) { r.servlet = std::make_shared<Servlet>(); }
            routes.insert(std::make_pair(route["name"].as<std::string>(), r));
        }
        return routes;
    }
};

template<>
class LexicalCast<std::unordered_map<std::string, Route>, std::string>
{
public:
    std::string operator()(const std::unordered_map<std::string, Route>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            YAML::Node route;
            route["name"] = i.first;
            route["path"] = i.second.path;
            route["class"] = typeid(*i.second.servlet).name();
            node.push_back(route);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


static ConfigVar<std::unordered_map<std::string, Route>>::Ptr g_servlets =
    Config::Lookup<std::unordered_map<std::string, Route>>(
        CONF_ROOT "servlets", std::unordered_map<std::string, Route>(), "servlets");

static pico::ConfigVar<std::vector<TcpServerOptions>>::Ptr g_servers_conf = pico::Config::Lookup(
    CONF_ROOT "servers", std::vector<TcpServerOptions>(), "http server config");


Application* Application::s_instance = nullptr;

Application::Application() {
    // if (s_instance) { throw std::runtime_error("Application already exists"); }
    s_instance = this;
}

bool Application::init(int argc, char* argv[]) {
    m_argc = argc;
    m_argv = argv;
    pico::Config::LoadFromConfDir(CONF_DIR);

    bool is_daemon = false;
    po::options_description desc("options");
    desc.add_options()("help,h", "help message")("daemon,d", po::bool_switch(&is_daemon), "daemon");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return false;
    }
    m_is_daemon = is_daemon;

    return true;
}

int Application::run() {
    return start_daemon(
        m_argc,
        m_argv,
        std::bind(&Application::main, this, std::placeholders::_1, std::placeholders::_2),
        m_is_daemon);
}

int Application::main(int argc, char* argv[]) {
    m_main_manager.reset(new IOManager(1, true, "main"));
    m_main_manager->schedule(std::bind(&Application::run_in_fiber, this));
    m_main_manager->addTimer(
        2000, []() {}, true);

    m_main_manager->stop();

    return 0;
}

void Application::run_in_fiber() {
    auto server_confs = g_servers_conf->getValue();
    auto servlets = g_servlets->getValue();

    std::vector<TcpServer::Ptr> servers;
    for (auto& server_conf : server_confs) {
        std::vector<Address::Ptr> addresses;
        for (auto& addr : server_conf.addresses) {
            auto pos = addr.find(':');
            if (pos == std::string::npos) {
                LOG_ERROR("invalid address: %s", addr.c_str());
                continue;
            }
            std::string host = addr.substr(0, pos);
            uint16_t port = atoi(addr.substr(pos + 1).c_str());

            auto address = IPAddress::Create(host.c_str(), port);
            if (address) {
                addresses.push_back(address);
                continue;
            }
            std::vector<Address::Ptr> addrs;
            if (Address::getInterfaceAddresses(addrs, host.c_str())) {
                for (auto item : addrs) {
                    auto ipaddr = std::dynamic_pointer_cast<IPAddress>(item);
                    if (ipaddr) { ipaddr->setPort(port); }
                    addresses.push_back(ipaddr);
                }
                continue;
            }
            auto adr = Address::LookupAny(addr);
            if (adr) {
                addresses.push_back(adr);
                continue;
            }
        }

        IOManager* worker = IOManager::getThis();
        worker->setName("worker");
        IOManager* acceptor = IOManager::getThis();
        acceptor->setName("acceptor");

        HttpServer::Ptr server(new HttpServer(false, worker, acceptor));
        if (!server_conf.name.empty()) { server->setName(server_conf.name); }
        std::vector<Address::Ptr> fails;
        if (!server->bind(addresses, fails, server_conf.ssl)) {
            for (auto i : fails) { LOG_ERROR("bind to %s failed", i->to_string().c_str()); }
            exit(-1);
        }
        if (server_conf.ssl) {
            if (!server->loadCertificate(server_conf.cert_file, server_conf.key_file)) {
                LOG_ERROR("load certficate failed");
            }
        }

        auto handlers = server_conf.servlets;
        auto req_handler = server->getRequestHandler();
        for (auto handler_name : handlers) {
            if (handler_name.find("*") != std::string::npos) {
                req_handler->addGlobalRoute(servlets[handler_name].path,
                                            servlets[handler_name].servlet);
            }
            else {
                req_handler->addRoute(servlets[handler_name].path, servlets[handler_name].servlet);
            }
        }

        m_servers[server_conf.type].push_back(server);
        servers.push_back(server);
    };

    for (auto server : servers) { server->start(); }
};

}   // namespace pico