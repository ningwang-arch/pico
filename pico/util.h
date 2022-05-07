#ifndef __PICO_UTIL_H__
#define __PICO_UTIL_H__

#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <string>
#include <vector>

#include "fiber.h"

namespace pico {
pid_t getThreadId();

uint32_t getFiberId();

std::string getForamtedTime(const char* format);

uint64_t getCurrentTime();

std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");

class StringUtil
{
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);

    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

    static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");
};


std::string getAbsolutePath(const std::string& path);

void listDir(const std::string& path, std::vector<std::string>& files, std::string& suffix = "");

}   // namespace pico



#endif
