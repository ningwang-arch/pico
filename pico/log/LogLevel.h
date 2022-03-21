#ifndef __PICO_LOG_LOGLEVEL_H__
#define __PICO_LOG_LOGLEVEL_H__

namespace pico {
class LogLevel
{
public:
    enum Level
    {
        UNKNOWN = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    static const char* toString(LogLevel::Level level);
    static LogLevel::Level fromString(const char* level);
};
}   // namespace pico

#endif