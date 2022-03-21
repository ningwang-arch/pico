#ifndef __PICO_UTIL_H__
#define __PICO_UTIL_H__

#include <string>

namespace pico {
std::string getForamtedTime(const char* format);

uint64_t getCurrentTime();

}   // namespace pico

#endif