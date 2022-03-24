#include "pico/logging.h"
#include "pico/scheduler.h"
#include <stdio.h>
#include <unistd.h>

void test_fiber() {
    static int count = 5;
    LOG_INFO("test_in_fiber, count = %d", count);
    sleep(1);
    if (--count >= 0) { pico::Scheduler::GetThis()->schedule(&test_fiber, pico::getThreadId()); }
}

int main(int argc, char const* argv[]) {
    LOG_INFO("main begin");
    pico::Scheduler sc(3, false, "test");
    sleep(2);
    sc.start();
    sc.schedule(&test_fiber);
    sc.stop();
    LOG_INFO("main end");
    return 0;
}
