#include "LoggerManager.h"
#include "LogAppender.h"
#include "Logger.h"
#include "PatternLayout.h"
#include "SimpleLayout.h"

namespace pico {

LoggerManager::LoggerManager()
    : m_mutex()
    , m_root(new Logger("root")) {
    pico::StdoutAppender::Ptr stdoutAppender(new pico::StdoutAppender);

    stdoutAppender->setLayout(pico::Layout::Ptr(new pico::PatternLayout));

    m_root->addAppender(stdoutAppender);

    m_loggers[m_root->getName()] = m_root;
}

void LoggerManager::addLogger(std::string& name, Logger::Ptr logger) {
    MutexType::Lock lock(m_mutex);
    m_loggers[name] = logger;
}

Logger::Ptr LoggerManager::getLogger(const std::string& name) {
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) return it->second;

    Logger::Ptr logger(new Logger(name));
    logger = m_root;
    m_loggers[name] = logger;
    return logger;
}

Logger::Ptr LoggerManager::getRootLogger() {
    MutexType::Lock lock(m_mutex);
    return m_root;
}

}   // namespace pico