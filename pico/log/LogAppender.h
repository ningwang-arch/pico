#ifndef __PICO_LOG_LOGAPPENDER_H__
#define __PICO_LOG_LOGAPPENDER_H__

#include "../mutex.h"
#include "Layout.h"
#include "LogEvent.h"
#include <fstream>

#include <memory>

namespace pico {
class Logger;

class LogAppender
{
    friend class Logger;

public:
    typedef Mutex MutexType;
    typedef std::shared_ptr<LogAppender> Ptr;
    virtual void log(std::shared_ptr<Logger> logger, LogEvent::Ptr event) = 0;

    void setLayout(std::shared_ptr<Layout> layout);
    Layout::Ptr getLayout();

protected:
    Layout::Ptr layout_;
    bool m_hasLayout = false;
    MutexType m_mutex;
};

class StdoutAppender : public LogAppender
{
public:
    typedef std::shared_ptr<StdoutAppender> Ptr;
    virtual ~StdoutAppender() {}
    virtual void log(std::shared_ptr<Logger> logger, LogEvent::Ptr event) override;
};

class FileAppender : public LogAppender
{
public:
    typedef std::shared_ptr<FileAppender> Ptr;
    FileAppender(const std::string& fileName);
    virtual ~FileAppender() {}
    virtual void log(std::shared_ptr<Logger> logger, LogEvent::Ptr event) override;

    bool reopen();

private:
    std::string m_fileName;
    std::ofstream m_ofs;
    uint64_t m_lastFlushTime = 0;
};

}   // namespace pico


#endif