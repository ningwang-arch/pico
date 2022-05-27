#include "server_manager.h"

namespace pico {
ServerManager::ServerManager() {}

void ServerManager::addServer(const std::string& name, std::shared_ptr<TcpServer> server) {
    std::lock_guard<std::mutex> lock(_mutex);
    _servers[name].push_back(server);
}

void ServerManager::delServer(const std::string& name) {
    std::lock_guard<std::mutex> lock(_mutex);
    stop(name);
    _servers.erase(name);
}

std::vector<std::shared_ptr<TcpServer>> ServerManager::getServer(const std::string& name) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _servers.find(name);
    if (it == _servers.end()) { return {}; }
    return it->second;
}

void ServerManager::listAllServers(
    std::map<std::string, std::vector<std::shared_ptr<TcpServer>>>& servers) {
    std::lock_guard<std::mutex> lock(_mutex);
    servers = _servers;
}

void ServerManager::start(const std::string& name) {
    auto server = getServer(name);
    if (server.empty()) { return; }
    for (auto& s : server) { s->start(); }
}

void ServerManager::startAll() {
    for (auto& server : _servers) {
        for (auto& s : server.second) { s->start(); }
    }
}

void ServerManager::stop(const std::string& name) {
    auto server = getServer(name);
    if (server.empty()) { return; }
    for (auto& s : server) { s->stop(); }
}

void ServerManager::stopAll() {
    for (auto& server : _servers) {
        for (auto& s : server.second) { s->stop(); }
    }
}


void ServerManager::reset() {
    stopAll();
    _servers.clear();
}

ServerManager::~ServerManager() {
    stopAll();
}

}   // namespace pico