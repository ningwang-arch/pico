#include "LoggerManager.h"
#include "LogAppender.h"
#include "Logger.h"
#include "SimpleLayout.h"

namespace pico {

LoggerManager::LoggerManager()
    : m_mutex()
    , m_root(new Logger("root")) {
    pico::StdoutAppender::Ptr stdoutAppender(new pico::StdoutAppender);

    stdoutAppender->setLayout(pico::Layout::Ptr(new pico::SimpleLayout));

    m_root->addAppender(stdoutAppender);
}

Logger::Ptr LoggerManager::getLogger() {
    MutexType::Lock lock(m_mutex);
    return m_root;
}

}   // namespace pico