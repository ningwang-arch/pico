#include "util.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <sys/syscall.h>
#include <unistd.h>

namespace pico {
pid_t getThreadId() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

std::string getForamtedTime(const char* format) {
    std::stringstream ss;
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    ss << std::put_time(tm, format);
    return ss.str();
}

uint64_t getCurrentTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

}   // namespace pico