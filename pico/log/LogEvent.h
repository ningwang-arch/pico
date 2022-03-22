#ifndef __PICO_LOG_LOGEVENT_H__
#define __PICO_LOG_LOGEVENT_H__

#include <memory>
#include <string>

#include "LogLevel.h"

namespace pico {
class Logger;
class LogEvent
{
public:
    typedef std::shared_ptr<LogEvent> Ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int line,
             uint32_t threadId, uint32_t fiberId, uint64_t timestamp, const std::string& threadName,
             const std::string& message);

    const char* getFile() const { return m_file; }
    int getLine() const { return m_line; }
    LogLevel::Level getLevel() const { return m_level; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTimestamp() const { return m_timestamp; }
    const std::string& getThreadName() const { return m_threadName; }
    const std::string& getMessage() const { return m_message; }

    std::shared_ptr<Logger> getLogger() const;

    std::string toString() const;

    static std::string format(const char* fmt, ...);
    static std::string format(const char* fmt, va_list al);

private:
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
    const char* m_file = nullptr;
    int m_line = 0;
    uint32_t m_threadId = 0;
    uint32_t m_fiberId = 0;
    uint64_t m_timestamp = 0;
    std::string m_threadName;
    std::string m_message;
};
}   // namespace pico

#endif