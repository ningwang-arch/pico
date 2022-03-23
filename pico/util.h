#ifndef __PICO_UTIL_H__
#define __PICO_UTIL_H__

#include <pthread.h>
#include <string>

#include "fiber.h"

namespace pico {
pid_t getThreadId();

uint32_t getFiberId();

std::string getForamtedTime(const char* format);

uint64_t getCurrentTime();

}   // namespace pico

#endif