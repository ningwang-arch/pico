#include "application.h"

#include "class_factory.h"
#include "daemon.h"
#include "env.h"
#include "filter.h"
#include "logging.h"
#include "worker.h"

#include "http/servlets/404_servlet.h"
#include "server_manager.h"

#include "ws/ws_server.h"
#include "ws/ws_servlet.h"

namespace pico {


auto server_manager = ServerManager::getInstance();

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
            if (!r.servlet) {
                continue;
            }
            r.servlet->name = route["class"].as<std::string>();
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
            if (!i.second.servlet) {
                continue;
            }
            YAML::Node route;
            route["name"] = i.first;
            route["path"] = i.second.path;
            route["class"] = i.second.servlet->name;
            node.push_back(route);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


struct WsRoute
{
    std::string path;
    WsServlet::Ptr servlet;
};
template<>
class LexicalCast<std::string, std::map<std::string, WsRoute>>
{
public:
    std::map<std::string, WsRoute> operator()(const std::string& str) const {
        std::map<std::string, WsRoute> servlets;
        YAML::Node node = YAML::Load(str);
        for (auto servlet : node) {
            WsRoute r;
            r.path = servlet["path"].as<std::string>();
            r.servlet = std::static_pointer_cast<WsServlet>(
                ClassFactory::Instance().Create(servlet["class"].as<std::string>()));
            if (!r.servlet) {
                continue;
            }
            r.servlet->name = servlet["class"].as<std::string>();
            servlets.insert(std::make_pair(servlet["name"].as<std::string>(), r));
        }
        return servlets;
    }
};

template<>
class LexicalCast<std::map<std::string, WsRoute>, std::string>
{
public:
    std::string operator()(const std::map<std::string, WsRoute>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            if (!i.second.servlet) {
                continue;
            }
            YAML::Node servlet;
            servlet["name"] = i.first;
            servlet["path"] = i.second.path;
            servlet["class"] = i.second.servlet->name;
            node.push_back(servlet);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

static ConfigVar<std::unordered_map<std::string, Route>>::Ptr g_servlets =
    Config::Lookup<std::unordered_map<std::string, Route>>(
        "servlets", std::unordered_map<std::string, Route>(), "servlets");

static ConfigVar<std::map<std::string, WsRoute>>::Ptr g_ws_servlets =
    Config::Lookup<std::map<std::string, WsRoute>>("ws_servlets", std::map<std::string, WsRoute>(),
                                                   "ws_servlets");

static pico::ConfigVar<std::vector<TcpServerOptions>>::Ptr g_servers_conf =
    pico::Config::Lookup("servers", std::vector<TcpServerOptions>(), "http server config");

static ConfigVar<std::vector<FilterConfs>>::Ptr g_filters_conf =
    Config::Lookup<std::vector<FilterConfs>>("filters", std::vector<FilterConfs>(),
                                             "http filters config");

Application* Application::s_instance = nullptr;

Application::Application() {
    if (s_instance) {
        throw std::runtime_error("Application already exists");
    }
    s_instance = this;
}

bool Application::init(int argc, char* argv[]) {
    m_argc = argc;
    m_argv = argv;

    pico::EnvManager::getInstance()->addHelp("d,daemon", "run as daemon");
    pico::EnvManager::getInstance()->addHelp("c,conf", "conf path [default: ./conf]");
    pico::EnvManager::getInstance()->addHelp("h,help", "print help");

    if (!pico::EnvManager::getInstance()->init(argc, argv)) {
        return false;
    }

    if (pico::EnvManager::getInstance()->has("h")) {
        pico::EnvManager::getInstance()->printHelp();
        return false;
    }

    std::string conf_path = pico::EnvManager::getInstance()->getConfigPath();
    if (conf_path.empty()) {
        conf_path = "./conf";
    }

    pico::Config::LoadFromConfDir(conf_path);

    LogInit log_init;

    return true;
}

int Application::run() {
    bool is_daemon = pico::EnvManager::getInstance()->has("d");
    return start_daemon(
        m_argc,
        m_argv,
        std::bind(&Application::main, this, std::placeholders::_1, std::placeholders::_2),
        is_daemon);
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
    auto ws_servlets = g_ws_servlets->getValue();
    auto filters = g_filters_conf->getValue();

    std::unordered_map<std::string, FilterChain::Ptr> filter_chains;
    for (auto& fc : filters) {
        std::string name = fc.name;
        std::vector<std::string> url_patterns = fc.url_patterns;
        std::string filter_class = fc.cls_name;
        Filter::Ptr filter =
            std::static_pointer_cast<Filter>(ClassFactory::Instance().Create(filter_class));
        if (!filter) {
            continue;
        }
        std::string desc = fc.description;
        auto init_params = fc.init_params;

        FilterConfig::Ptr filter_conf(new FilterConfig());
        filter_conf->setName(name);
        filter_conf->setClass(filter_class);
        filter_conf->setDescription(desc);
        filter_conf->setInitParams(init_params);
        filter_conf->setFilter(filter);

        filter->init(filter_conf);

        for (auto url_pattern : url_patterns) {
            if (filter_chains.find(url_pattern) == filter_chains.end()) {
                filter_chains[url_pattern] = std::make_shared<FilterChain>();
            }

            filter_chains[url_pattern]->addFilter(filter_conf);
        }
    }

    pico::WorkerMgr::getInstance()->init();

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
            if (Address::getInterfaceAddresses(addrs, host)) {
                for (auto item : addrs) {
                    auto ipaddr = std::dynamic_pointer_cast<IPAddress>(item);
                    if (ipaddr) {
                        ipaddr->setPort(port);
                    }
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

        IOManager* worker = IOManager::GetThis();
        IOManager* acceptor = IOManager::GetThis();

        if (!server_conf.worker.empty()) {
            worker = pico::WorkerMgr::getInstance()->get(server_conf.worker).get();
            if (!worker) {
                LOG_ERROR("invalid worker: %s", server_conf.worker.c_str());
                exit(1);
            }
        }
        if (!server_conf.acceptor.empty()) {
            acceptor = pico::WorkerMgr::getInstance()->get(server_conf.acceptor).get();
            if (!acceptor) {
                LOG_ERROR("invalid acceptor: %s", server_conf.acceptor.c_str());
                exit(1);
            }
        }

        if (server_conf.type == "http") {
            HttpServer::Ptr server(new HttpServer(server_conf.keep_alive, worker, acceptor));
            if (!server_conf.name.empty()) {
                server->setName(server_conf.name);
            }
            std::vector<Address::Ptr> fails;
            if (!server->bind(addresses, fails, server_conf.ssl)) {
                for (auto i : fails) {
                    LOG_ERROR("bind to %s failed", i->to_string().c_str());
                }
                exit(-1);
            }
            if (server_conf.ssl) {
                if (!server->loadCertificate(server_conf.cert_file, server_conf.key_file)) {
                    LOG_ERROR("load certficate failed");
                }
            }

            auto handlers = server_conf.servlets;
            auto req_handler = server->getRequestHandler();
            req_handler->reset();
            for (auto handler_name : handlers) {
                if (servlets.find(handler_name) == servlets.end()) {
                    LOG_ERROR("invalid servlet: %s", handler_name.c_str());
                    continue;
                }
                if (servlets[handler_name].path.find("*") != std::string::npos) {
                    req_handler->addGlobalRoute(servlets[handler_name].path,
                                                servlets[handler_name].servlet);
                }
                else {
                    req_handler->addRoute(servlets[handler_name].path,
                                          servlets[handler_name].servlet);
                }
            }
            req_handler->addExcludePath(server_conf.exclude_paths);
            for (auto filter_chain : filter_chains) {
                addFilterChain(filter_chain.first, filter_chain.second);
            }
            m_servers[server_conf.type].push_back(server);
            server_manager->addServer(server->getName(), server);
        }
        else if (server_conf.type == "ws") {
            WsServer::Ptr server(new WsServer(worker, acceptor));
            server->setIsHttp(false);
            if (!server_conf.name.empty()) {
                server->setName(server_conf.name);
            }
            std::vector<Address::Ptr> fails;
            if (!server->bind(addresses, fails, server_conf.ssl)) {
                for (auto i : fails) {
                    LOG_ERROR("bind to %s failed", i->to_string().c_str());
                }
                exit(-1);
            }

            if (server_conf.ssl) {
                if (!server->loadCertificate(server_conf.cert_file, server_conf.key_file)) {
                    LOG_ERROR("load certficate failed");
                }
            }

            auto handlers = server_conf.servlets;
            auto req_handler = server->getServletDispatcher();
            req_handler->reset();
            for (auto handler_name : handlers) {
                if (ws_servlets.find(handler_name) == ws_servlets.end()) {
                    LOG_ERROR("invalid servlet: %s", handler_name.c_str());
                    continue;
                }
                if (ws_servlets[handler_name].path.find("*") != std::string::npos) {
                    req_handler->addServlet(
                        ws_servlets[handler_name].path, ws_servlets[handler_name].servlet, true);
                }
                else {
                    req_handler->addServlet(ws_servlets[handler_name].path,
                                            ws_servlets[handler_name].servlet);
                }
            }

            m_servers[server_conf.type].push_back(server);
            server_manager->addServer(server->getName(), server);
        }
    };
    server_manager->startAll();
};

}   // namespace pico
