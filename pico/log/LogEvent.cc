#include "LogEvent.h"

#include "Logger.h"
#include <sstream>

namespace pico {
LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file,
                   int line, uint32_t threadId, uint32_t fiberId, uint64_t timestamp,
                   const std::string& threadName, const std::string& message)
    : m_logger(logger)
    , m_level(level)
    , m_file(file)
    , m_line(line)
    , m_threadId(threadId)
    , m_fiberId(fiberId)
    , m_timestamp(timestamp)
    , m_threadName(threadName)
    , m_message(message) {}

std::string LogEvent::toString() const {
    std::stringstream ss;
    ss << "[" << m_timestamp << "] " << m_threadName << "(" << m_threadId << ":" << m_fiberId
       << ") " << m_logger->getName() << " " << LogLevel::toString(m_level) << " " << m_file << ":"
       << m_line << " " << m_message;
    return ss.str();
}

std::shared_ptr<Logger> LogEvent::getLogger() const {
    return m_logger;
}

}   // namespace pico