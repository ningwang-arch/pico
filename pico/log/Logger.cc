#include "Logger.h"
#include "LogAppender.h"
#include "SimpleLayout.h"

#include <iostream>

namespace pico {

Logger::Logger(const std::string& name)
    : m_name(name)
    , m_level(LogLevel::DEBUG)
    , m_layout(new SimpleLayout()) {}


void Logger::addAppender(std::shared_ptr<LogAppender> appender) {

    MutexType::Lock lock(m_mutex);
    if (!appender->getLayout()) { appender->setLayout(m_layout); }

    m_appenders.push_back(appender);
}

void Logger::removeAppender(std::shared_ptr<LogAppender> appender) {
    MutexType::Lock lock(m_mutex);
    m_appenders.remove(appender);
}

void Logger::clearAppenders() {
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
}

void Logger::setLayout(std::shared_ptr<Layout> layout) {
    MutexType::Lock lock(m_mutex);
    m_layout = layout;
    for (auto& i : m_appenders) {
        MutexType::Lock lock(i->m_mutex);
        if (!i->m_hasLayout) { i->layout_ = layout; }
    }
}

void Logger::log(LogLevel::Level level, LogEvent::Ptr event) {
    if (level >= m_level) {
        auto self = shared_from_this();
        MutexType::Lock lock(m_mutex);
        if (!m_appenders.empty())
            for (auto& i : m_appenders) { i->log(self, event); }
        else
            std::cout << event->toString() << std::endl;
    }
}

void Logger::log(LogLevel::Level level, std::string& message) {
    if (level >= m_level) {
        auto self = shared_from_this();
        MutexType::Lock lock(m_mutex);
        auto event = LogEvent::Ptr(
            new LogEvent(self, level, __FILE__, __LINE__, 0, 0, time(0), "t1", message));
        if (!m_appenders.empty())
            for (auto& i : m_appenders) i->log(self, event);
        else
            std::cout << event->toString() << std::endl;
    }
}

void Logger::debug(LogEvent::Ptr event) {
    log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::Ptr event) {
    log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::Ptr event) {
    log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::Ptr event) {
    log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::Ptr event) {
    log(LogLevel::FATAL, event);
}

void Logger::debug(const std::string& message) {
    debug(LogEvent::Ptr(new LogEvent(
        shared_from_this(), LogLevel::DEBUG, __FILE__, __LINE__, 0, 0, time(0), "t1", message)));
}

void Logger::info(const std::string& message) {
    info(LogEvent::Ptr(new LogEvent(
        shared_from_this(), LogLevel::INFO, __FILE__, __LINE__, 0, 0, time(0), "t1", message)));
}

void Logger::warn(const std::string& message) {
    warn(LogEvent::Ptr(new LogEvent(
        shared_from_this(), LogLevel::WARN, __FILE__, __LINE__, 0, 0, time(0), "t1", message)));
}

void Logger::error(const std::string& message) {
    error(LogEvent::Ptr(new LogEvent(
        shared_from_this(), LogLevel::ERROR, __FILE__, __LINE__, 0, 0, time(0), "t1", message)));
}

void Logger::fatal(const std::string& message) {
    fatal(LogEvent::Ptr(new LogEvent(
        shared_from_this(), LogLevel::FATAL, __FILE__, __LINE__, 0, 0, time(0), "t1", message)));
}

}   // namespace pico