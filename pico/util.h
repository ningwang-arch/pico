#ifndef __PICO_UTIL_H__
#define __PICO_UTIL_H__

#include <pthread.h>
#include <string>

namespace pico {
pid_t getThreadId();

std::string getForamtedTime(const char* format);

uint64_t getCurrentTime();

}   // namespace pico

#endif