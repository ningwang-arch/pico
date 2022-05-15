#ifndef __PICO_TCP_SERVER_H__
#define __PICO_TCP_SERVER_H__

#include "address.h"
#include "config.h"
#include "iomanager.h"
#include "socket.h"
#include <memory>
#include <string>
#include <vector>

namespace pico {

struct TcpServerOptions
{
    typedef std::shared_ptr<TcpServerOptions> Ptr;

    std::vector<std::string> addresses;
    std::string type = "http";
    std::string name = "";

    bool ssl = false;

    std::string worker = "";
    std::string acceptor = "";

    std::string cert_file = "";
    std::string key_file = "";
    bool keep_alive = false;
    std::vector<std::string> servlets;

    bool isValid() const { return !addresses.empty(); }

    bool operator==(const TcpServerOptions& other) const {
        return addresses == other.addresses && type == other.type && name == other.name &&
               cert_file == other.cert_file && key_file == other.key_file &&
               servlets == other.servlets && ssl == other.ssl;
    }
};

template<>
class LexicalCast<std::string, TcpServerOptions>
{
public:
    TcpServerOptions operator()(const std::string& str) const {
        TcpServerOptions options;
        YAML::Node node = YAML::Load(str);
        options.type = node["type"].as<std::string>(options.type);
        options.name = node["name"].as<std::string>(options.name);
        options.ssl = node["ssl"].as<bool>(options.ssl);
        options.keep_alive = node["keep_alive"].as<bool>(options.keep_alive);
        options.worker = node["worker"].as<std::string>(options.worker);
        options.acceptor = node["acceptor"].as<std::string>(options.acceptor);
        if (options.ssl) {
            // std::cout << node["certificates"] << std::endl;
            options.cert_file = node["certificates"]["file"].as<std::string>(options.cert_file);
            options.key_file = node["certificates"]["key"].as<std::string>(options.key_file);
        }

        if (node["addresses"].IsDefined()) {
            for (auto addr : node["addresses"]) {
                options.addresses.push_back(addr.as<std::string>());
            }
        }
        if (node["servlets"].IsDefined()) {
            for (auto servlet : node["servlets"]) {
                options.servlets.push_back(servlet.as<std::string>());
            }
        }
        return options;
    }
};

template<>
class LexicalCast<TcpServerOptions, std::string>
{
public:
    std::string operator()(const TcpServerOptions& options) const {
        YAML::Node node;
        node["type"] = options.type;
        node["name"] = options.name;
        node["ssl"] = options.ssl;
        node["keep_alive"] = options.keep_alive;
        node["worker"] = options.worker;
        node["acceptor"] = options.acceptor;
        node["certicates"]["file"] = options.cert_file;
        node["certicates"]["key"] = options.key_file;
        for (auto& addr : options.addresses) { node["addresses"].push_back(addr); }
        for (auto& servlet : options.servlets) { node["servlets"].push_back(servlet); }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};



class TcpServer : Noncopyable, public std::enable_shared_from_this<TcpServer>
{
public:
    typedef std::shared_ptr<TcpServer> Ptr;

    TcpServer(IOManager* worker = IOManager::GetThis(), IOManager* acceptor = IOManager::GetThis());


    virtual ~TcpServer();

    virtual bool bind(Address::Ptr& addr, bool ssl = false);
    virtual bool bind(std::vector<Address::Ptr>& addrs, std::vector<Address::Ptr>& fails,
                      bool ssl = false);

    virtual bool start();

    virtual void stop();

    bool isStop() const { return m_is_stop; }

    virtual uint64_t getRecvTimeout() const { return m_recvTimeout; }
    virtual void setRecvTimeout(uint64_t& timeout) { m_recvTimeout = timeout; }

    virtual std::string getName() const { return m_name; }
    virtual void setName(const std::string& name) { m_name = name; }

    virtual std::string to_string();

    virtual bool loadCertificate(const std::string& cert_file, const std::string& key_file);

protected:
    virtual void startAccept(Socket::Ptr& sock);

    virtual void handleClient(Socket::Ptr& sock);


private:
    std::vector<Socket::Ptr> m_sockets;

    std::string m_name;
    IOManager* m_worker;
    IOManager* m_acceptor;

    uint64_t m_recvTimeout;

    bool m_is_stop;
};

}   // namespace pico

#endif