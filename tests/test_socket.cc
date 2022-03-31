#include "pico/address.h"
#include "pico/iomanager.h"
#include "pico/logging.h"
#include "pico/socket.h"
#include <iostream>
#include <unistd.h>

void test() {
    pico::IPAddress::Ptr addr = pico::Address::LookupAnyIPAddress("www.baidu.com");

    if (addr) { LOG_INFO("%s", addr->to_string().c_str()); }
    else {
        LOG_INFO("not found");
        return;
    }

    pico::Socket::Ptr sock = pico::Socket::CreateTcp(addr);
    addr->setPort(80);
    LOG_INFO("%s", addr->to_string().c_str());
    if (!sock->connect(addr)) {
        LOG_INFO("connect failed");
        return;
    }

    const char buf[] = "GET / HTTP/1.1\r\nConnection: close\r\n\r\n";
    if (!sock->send(buf, sizeof(buf))) {
        LOG_INFO("send failed, errno=%d, %s", errno, strerror(errno));
        return;
    }

    char buf2[BUFSIZ];
    std::string resp;
    while (true) {
        int rt = sock->recv(buf2, sizeof(buf2));
        // std::cout << rt << std::endl;
        if (rt <= 0) {
            LOG_INFO("recv failed");
            break;
        }
        resp.append(buf2, rt);
        memset(buf2, 0, sizeof(buf2));
    }


    LOG_INFO("%s", resp.c_str());
}

void test_addr() {
    pico::IPAddress::Ptr addr = pico::Address::LookupAnyIPAddress("www.baidu.com");
    addr->setPort(80);
    LOG_INFO("%s", addr->to_string().c_str());


    int sockfd = socket(addr->getFamily(), SOCK_STREAM, 0);

    connect(sockfd, addr->getAddr(), addr->getAddrLen());

    const char buf[] = "GET / HTTP/1.1\r\n\r\n";
    send(sockfd, buf, sizeof(buf), 0);

    char buf2[BUFSIZ];
    std::string resp;
    while (true) {
        int rt = recv(sockfd, buf2, sizeof(buf2), 0);
        // std::cout << rt << std::endl;
        if (rt <= 0) {
            LOG_INFO("recv failed");
            break;
        }
        resp.append(buf2, rt);
        memset(buf2, 0, sizeof(buf2));
    }
}

int main(int argc, char const* argv[]) {
    pico::IOManager iom(1, true, "iom");
    iom.schedule(test);
    return 0;
}
