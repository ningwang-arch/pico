#include "pico/hook.h"
#include "pico/iomanager.h"
#include "pico/logging.h"
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void test_sleep() {
    pico::IOManager iom(1);
    iom.schedule([]() {
        sleep(2);
        LOG_INFO("sleep 2s");
    });

    iom.schedule([]() {
        sleep(3);
        LOG_INFO("sleep 3s");
    });

    LOG_INFO("test sleep");
}

void test_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG_ERROR("socket errno=%d, %s", errno, strerror(errno));
        return;
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "36.152.44.96", &addr.sin_addr.s_addr);

    LOG_INFO("connect begin");
    int rt = connect(sock, (const struct sockaddr*)&addr, sizeof(addr));
    // fcntl(sock, F_SETFL, O_NONBLOCK);
    if (rt && errno != EINPROGRESS) {
        LOG_INFO("connect errno=%d, %s, fd=%d", errno, strerror(errno), sock);
        return;
    }
    else {
        LOG_INFO("connect success");
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    if (rt < 0) {
        LOG_INFO("send errno=%d, %s", errno, strerror(errno));
        return;
    }
    else {
        LOG_INFO("send success");
    }

    char buf[1024];
    std::string resp;
    while (recv(sock, buf, sizeof(buf), 0) > 0) {
        resp += std::string(buf);
        memset(buf, 0, sizeof(buf));
    }
    close(sock);
    LOG_INFO("recv success, resp.length=%d", resp.length());
    LOG_INFO("%s", resp.c_str());
}

int main(int argc, char const* argv[]) {
    // test_sleep();
    pico::IOManager iom(1, true, "iom");
    iom.schedule(test_socket);
    return 0;
}
