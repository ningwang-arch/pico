#include "pico/logging.h"

// static pico::Logger::Ptr g_logger = LoggerMgr::getInstance()->getRootLogger();

// static pico::Logger::Ptr g_logger;

void test_simple() {
    pico::Layout::Ptr layout(new pico::SimpleLayout);
    g_logger->setLayout(layout);
    LOG_DEBUG("debug");
    LOG_DEBUG("msg is %s", "debug");
    // g_logger->debug("debug");
    // g_logger->info("info");
    // g_logger->warn("warn");
    // g_logger->error("error");
    // g_logger->fatal("fatal");
}

void test() {
    // g_logger->debug("debug");
    // g_logger->info("info");
    // g_logger->warn("warn");
    // g_logger->error("error");
    // g_logger->fatal("fatal");
    LOG_DEBUG("debug");
    LOG_INFO("msg is %s", "info");
    // LOG_DEBUG(g_logger, "debug");
}

void test_custom_pattern() {
    pico::PatternLayout::Ptr pl(new pico::PatternLayout);
    // g_logger->setLayout(pl);
    // layout->getPattern();
    pl->setPattern("%d %p [%c] - %m%n");
    g_logger->setLayout(pl);
    LOG_DEBUG("debug");
    LOG_FATAL("msg is %s", "fatal");
}

void test_ttcc() {
    LOG_DEBUG("debug");
    LOG_DEBUG("msg is %s", "debug");
    // g_logger->debug("debug");
    // g_logger->info("info");
    // g_logger->warn("warn");
    // g_logger->error("error");
    // g_logger->fatal("fatal");
}

int main(int argc, char const* argv[]) {
    // test_simple();
    // test_ttcc();
    // test();
    test_custom_pattern();
    return 0;
}
