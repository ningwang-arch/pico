#include "pico/logging.h"
#include "pico/thread.h"

int count = 0;
pico::Mutex mutex;

void func_1() {
    LOG_INFO("name: %s, this.name=%s, id: %d, this.id=%d",
             pico::Thread::GetName().c_str(),
             pico::Thread::GetThis()->getName().c_str(),
             pico::getThreadId(),
             pico::Thread::GetThis()->getId());
    for (int i = 0; i < 100000; i++) {
        pico::Mutex::Lock lock(mutex);
        count++;
    }
}

void func_2() {
    while (true) { LOG_INFO("xxxxxxxxxxxxxxxxxxxxx"); }
}
int main(int argc, char const* argv[]) {
    LOG_INFO("test_thread begin");
    std::vector<pico::Thread::Ptr> thrs;
    for (int i = 0; i < 2; i++) {
        pico::Thread::Ptr thr(new pico::Thread(func_1, "thr_" + std::to_string(i)));
        thrs.push_back(thr);
        // pico::Thread::Ptr thr2(new pico::Thread(func_2, "thr2_" + std::to_string(i)));
        // thrs.push_back(thr2);
    }

    for (auto& thr : thrs) { thr->join(); }

    LOG_INFO("test_thread end");

    LOG_INFO("count: %d", count);

    return 0;
}
