#ifndef __PICO_LOGGING_H__
#define __PICO_LOGGING_H__

#include "log/LoggerManager.h"
#include "log/PatternLayout.h"
#include "log/SimpleLayout.h"
#include "log/TTCCLayout.h"
#include "singleton.h"
#include "thread.h"
#include "util.h"

typedef pico::Singleton<pico::LoggerManager> LoggerMgr;

const static pico::Logger::Ptr g_logger = LoggerMgr::getInstance()->getRootLogger();


#define LOG(logger, level, fmt, ...)                                    \
    logger->log(                                                        \
        level,                                                          \
        pico::LogEvent::Ptr(new pico::LogEvent(logger,                  \
                                               level,                   \
                                               __FILE__,                \
                                               __LINE__,                \
                                               pico::getThreadId(),     \
                                               pico::getFiberId(),      \
                                               time(0),                 \
                                               pico::Thread::GetName(), \
                                               pico::LogEvent::format(fmt, ##__VA_ARGS__))))

#define LOG_DEBUG(fmt, ...) LOG(g_logger, pico::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG(g_logger, pico::LogLevel::INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(g_logger, pico::LogLevel::WARN, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG(g_logger, pico::LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) LOG(g_logger, pico::LogLevel::FATAL, fmt, ##__VA_ARGS__)

#endif