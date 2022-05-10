#ifndef __PICO_APPPLICATION_H__
#define __PICO_APPPLICATION_H__

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "http/http_server.h"
#include "iomanager.h"
#include "tcp_server.h"

namespace pico {

class Application
{
public:
    Application();

    static Application* getInstance() { return s_instance; }

    bool init(int argc, char* argv[]);

    int run();

private:
    int main(int argc, char* argv[]);

    void run_in_fiber();

private:
    static Application* s_instance;
    std::unordered_map<std::string, std::vector<TcpServer::Ptr>> m_servers;

    IOManager::Ptr m_main_manager;

    int m_argc = 0;
    char** m_argv = nullptr;

    bool m_is_daemon = false;
};

}   // namespace pico


#endif