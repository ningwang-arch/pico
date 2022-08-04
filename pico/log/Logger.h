#ifndef __PICO_LOG_LOGGER_H__
#define __PICO_LOG_LOGGER_H__

#include <list>
#include <memory>
#include <string>

#include "../mutex.h"
#include "Layout.h"
#include "LogLevel.h"

namespace pico {
class LoggerManager;
class LogEvent;
class LogAppender;

bool is_log_enabled();

void set_log_enabled(bool enabled);

class Logger : public std::enable_shared_from_this<Logger>
{
    friend class LoggerManager;

public:
    typedef Mutex MutexType;
    typedef std::shared_ptr<Logger> Ptr;

    Logger(const std::string& name = "root");

    void log(LogLevel::Level level, LogEvent::Ptr event);


    void addAppender(std::shared_ptr<LogAppender> appender);
    void removeAppender(std::shared_ptr<LogAppender> appender);

    void clearAppenders();

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level level) { m_level = level; }

    std::string getName() const { return m_name; }

    void setLayout(std::shared_ptr<Layout> layout);

    Layout::Ptr getLayout() {
        MutexType::Lock lock(m_mutex);
        return m_layout;
    }


private:
    void debug(LogEvent::Ptr event);
    void info(LogEvent::Ptr event);
    void warn(LogEvent::Ptr event);
    void error(LogEvent::Ptr event);
    void fatal(LogEvent::Ptr event);

    std::string m_name;
    LogLevel::Level m_level;
    std::list<std::shared_ptr<LogAppender>> m_appenders;
    Layout::Ptr m_layout;

    MutexType m_mutex;
};

}   // namespace pico

#endif