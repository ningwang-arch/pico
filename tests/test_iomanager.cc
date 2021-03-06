#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "pico/iomanager.h"
#include "pico/logging.h"
#include <chrono>
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
        pico::IOManager::GetThis()->addEvent(
            sock, pico::IOManager::READ, []() { LOG_INFO("read callback"); });

        pico::IOManager::GetThis()->addEvent(sock, pico::IOManager::WRITE, []() {
            LOG_INFO("write callback");

            pico::IOManager::GetThis()->cancelEvent(sock, pico::IOManager::READ);
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

pico::Timer::Ptr s_timer;
void test_timer() {
    pico::IOManager iom(2, false, "iom");
    s_timer = iom.addTimer(
        1000,
        []() {
            static int i = 0;
            LOG_INFO("timer callback %d", i);
            if (++i == 3) { s_timer->cancel(); }
        },
        true);
}

int main(int argc, char const* argv[]) {
    // test();
    test_timer();
    return 0;
}
