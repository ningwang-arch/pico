#ifndef __PICO_SERVER_MANAGER_H__
#define __PICO_SERVER_MANAGER_H__

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "singleton.h"
#include "tcp_server.h"

namespace pico {

class ServerManager : public Singleton<ServerManager>
{
public:
    typedef std::shared_ptr<ServerManager> Ptr;

    ServerManager();
    ~ServerManager();

    void addServer(const std::string& name, std::shared_ptr<TcpServer> server);
    void delServer(const std::string& name);

    std::vector<std::shared_ptr<TcpServer>> getServer(const std::string& name);

    void listAllServers(std::map<std::string, std::vector<std::shared_ptr<TcpServer>>>& servers);

    void start(const std::string& name);
    void startAll();

    void stop(const std::string& name);
    void stopAll();

    void reset();

private:
    std::map<std::string, std::vector<std::shared_ptr<TcpServer>>> _servers;
    std::mutex _mutex;
};

}   // namespace pico

#endif