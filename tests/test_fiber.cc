#include "pico/fiber.h"
#include "pico/logging.h"
#include "pico/thread.h"

int count = 0;
pico::Mutex mutex;

void run_in_fiber() {
    LOG_INFO("run_in_fiber begin");
    pico::Fiber::yieldToSuspend();
    LOG_INFO("run_in_fiber end");
    pico::Fiber::yieldToSuspend();
}

void test_fiber() {
    LOG_INFO("test_fiber begin");
    {
        pico::Fiber::GetThis();
        pico::Fiber::Ptr fiber(new pico::Fiber(run_in_fiber));
        fiber->call();
        LOG_INFO("test_fiber after swapIn");
        fiber->call();
        LOG_INFO("test_fiber after end");
        fiber->call();
    }
    LOG_INFO("test_fiber end");
}

int main(int argc, char const* argv[]) {
    LOG_INFO("main begin");
    pico::Thread::SetName("main");

    std::vector<pico::Thread::Ptr> thrs;
    for (int i = 0; i < 3; i++) {
        pico::Thread::Ptr thr(new pico::Thread(test_fiber, "thr_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    for (auto& thr : thrs) { thr->join(); }

    LOG_INFO("main end");

    LOG_FMT_INFO("count: %d", count);

    return 0;
}
