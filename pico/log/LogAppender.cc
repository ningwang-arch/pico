#include "LogAppender.h"

#include <iostream>

namespace pico {
void LogAppender::setLayout(std::shared_ptr<Layout> layout) {
    MutexType::Lock lock(m_mutex);
    this->layout_ = layout;
    if (m_hasLayout) { m_hasLayout = true; }
    else {
        m_hasLayout = false;
    }
}

Layout::Ptr LogAppender::getLayout() {
    MutexType::Lock lock(m_mutex);
    return layout_;
}

void StdoutAppender::log(std::shared_ptr<Logger> logger, LogEvent::Ptr event) {
    MutexType::Lock lock(m_mutex);
    std::cout << layout_->format(event);
}

FileAppender::FileAppender(const std::string& fileName)
    : m_fileName(fileName) {
    reopen();
}


void FileAppender::log(std::shared_ptr<Logger> logger, LogEvent::Ptr event) {
    uint64_t now = event->getTimestamp();
    if (now - m_lastFlushTime > 1000) {
        reopen();
        m_lastFlushTime = now;
    }
    MutexType::Lock lock(m_mutex);
    m_ofs << layout_->format(event) << std::endl;
}

bool FileAppender::reopen() {
    MutexType::Lock lock(m_mutex);
    if (m_ofs) { m_ofs.close(); }
    m_ofs.open(m_fileName, std::ios::app);
    if (!m_ofs.is_open()) { return false; }
    return true;
}

}   // namespace pico