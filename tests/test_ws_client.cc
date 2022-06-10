#include <iostream>

#include "pico/iomanager.h"
#include "pico/util.h"
#include "pico/ws/ws_request.h"

void run() {
    auto conn = pico::WsRequest::create("http://127.0.0.1:8080/", 1000);
    if (!conn) { return; }

    while (true) {
        for (int i = 0; i < 1; i++) {
            conn->sendMessage(pico::genRandomString(60), pico::WsFrameHeader::TEXT, false);
        }
        conn->sendMessage(pico::genRandomString(65), pico::WsFrameHeader::TEXT, true);
        auto msg = conn->recvMessage();
        if (!msg) { std::cout << errno << " " << strerror(errno) << std::endl; }
        std::cout << msg->get_data() << std::endl;
        sleep(5);
    }
}

int main(int argc, char const* argv[]) {
    pico::IOManager iom;
    iom.schedule(&run);
    return 0;
}
