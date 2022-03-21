#include "util.h"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace pico {

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