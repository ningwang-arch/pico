#ifndef __PICO_LOGGING_H__
#define __PICO_LOGGING_H__


#include "log/LogLevel.h"
#include "log/LoggerManager.h"
#include "log/PatternLayout.h"
#include "log/SimpleLayout.h"
#include "log/TTCCLayout.h"
#include "singleton.h"
#include "thread.h"
#include "util.h"


typedef pico::Singleton<pico::LoggerManager> LoggerMgr;

struct LogInit
{
    LogInit();
};


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


#define DEBUG(logger, fmt, ...) LOG(logger, pico::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define INFO(logger, fmt, ...) LOG(logger, pico::LogLevel::INFO, fmt, ##__VA_ARGS__)
#define WARN(logger, fmt, ...) LOG(logger, pico::LogLevel::WARN, fmt, ##__VA_ARGS__)
#define ERROR(logger, fmt, ...) LOG(logger, pico::LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define FATAL(logger, fmt, ...) LOG(logger, pico::LogLevel::FATAL, fmt, ##__VA_ARGS__)

#define ROOT_LOGGER() LoggerMgr::getInstance()->getRootLogger()

#define LOG_DEBUG(fmt, ...) LOG(ROOT_LOGGER(), pico::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG(ROOT_LOGGER(), pico::LogLevel::INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(ROOT_LOGGER(), pico::LogLevel::WARN, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG(ROOT_LOGGER(), pico::LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) LOG(ROOT_LOGGER(), pico::LogLevel::FATAL, fmt, ##__VA_ARGS__)

#endif