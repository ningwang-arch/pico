#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "pico/iomanager.h"
#include "pico/logging.h"
#include <fcntl.h>
#include <string.h>

int sock = 0;

void func() {
    LOG_INFO("func begin");
    sock = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt, sizeof(opt));

    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "220.181.38.251", &addr.sin_addr.s_addr);

    if (!connect(sock, (const struct sockaddr*)&addr, sizeof(addr))) {}
    else if (errno == EINPROGRESS) {
        LOG_INFO("add event errno=%d, %s", errno, strerror(errno));
        pico::IOManager::getThis()->addEvent(
            sock, pico::IOManager::READ, []() { LOG_INFO("read callback"); });

        pico::IOManager::getThis()->addEvent(sock, pico::IOManager::WRITE, []() {
            LOG_INFO("write callback");

            pico::IOManager::getThis()->cancelEvent(sock, pico::IOManager::READ);
            close(sock);
        });
    }
    else {
        LOG_INFO("connect errno=%d, %s", errno, strerror(errno));
    }
}


void test() {
    pico::IOManager iom(2, false, "iom");
    iom.schedule(&func);
}

int main(int argc, char const* argv[]) {
    test();
    return 0;
}
