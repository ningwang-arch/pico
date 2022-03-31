#include "pico/logging.h"
#include "pico/tcp_server.h"
#include <iostream>

void test() {
    auto addr = pico::Address::LookupAny("0.0.0.0:8080");

    std::cout << addr->to_string() << std::endl;
    pico::TcpServer::Ptr server(new pico::TcpServer);

    if (!server->bind(addr)) {
        LOG_ERROR("bind error, errno=%d, %s", errno, strerror(errno));
        return;
    }

    server->start();
}

int main(int argc, char const* argv[]) {
    pico::IOManager iom(2, true, "iom");
    iom.schedule(test);
    return 0;
}
