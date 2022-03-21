#include "pico/logging.h"

static pico::Logger::Ptr g_logger = LoggerMgr::getInstance()->getLogger();

void test_simple() {
    pico::Layout::Ptr layout(new pico::SimpleLayout);
    g_logger->setLayout(layout);
    g_logger->debug("debug");
    g_logger->info("info");
    g_logger->warn("warn");
    g_logger->error("error");
    g_logger->fatal("fatal");
}

void test() {
    g_logger->debug("debug");
    g_logger->info("info");
    g_logger->warn("warn");
    g_logger->error("error");
    g_logger->fatal("fatal");
}

void test_ttcc() {
    pico::Layout::Ptr layout(new pico::TTCCLayout);
    g_logger->setLayout(layout);
    g_logger->debug("debug");
    g_logger->info("info");
    g_logger->warn("warn");
    g_logger->error("error");
    g_logger->fatal("fatal");
}

int main(int argc, char const* argv[]) {
    // test_simple();
    // test_ttcc();
    test();
    return 0;
}
