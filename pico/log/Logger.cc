#include "Logger.h"
#include "LogAppender.h"
#include "SimpleLayout.h"

#include <iostream>

namespace pico {
static bool s_logEnabled = true;

bool is_log_enabled() {
    return s_logEnabled;
}

void set_log_enabled(bool enabled) {
    s_logEnabled = enabled;
}


Logger::Logger(const std::string& name)
    : m_name(name)
    , m_level(LogLevel::DEBUG)
    , m_layout(new SimpleLayout()) {}


void Logger::addAppender(std::shared_ptr<LogAppender> appender) {

    MutexType::Lock lock(m_mutex);
    if (!appender->getLayout()) {
        appender->setLayout(m_layout);
    }

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
        MutexType::Lock lock_(i->m_mutex);
        if (!i->m_hasLayout) {
            i->layout_ = layout;
        }
    }
}

void Logger::log(LogLevel::Level level, LogEvent::Ptr event) {
    if (!is_log_enabled()) {
        return;
    }

    if (level >= m_level) {
        auto self = shared_from_this();
        MutexType::Lock lock(m_mutex);
        if (!m_appenders.empty())
            for (auto& i : m_appenders) {
                i->log(self, event);
            }
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


}   // namespace pico