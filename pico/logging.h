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


#define LOG(logger, level, message)                                             \
    logger->log(level,                                                          \
                pico::LogEvent::Ptr(new pico::LogEvent(logger,                  \
                                                       level,                   \
                                                       __FILE__,                \
                                                       __LINE__,                \
                                                       pico::getThreadId(),     \
                                                       0,                       \
                                                       time(0),                 \
                                                       pico::Thread::GetName(), \
                                                       message)))

#define LOG_DEBUG(message) LOG(g_logger, pico::LogLevel::DEBUG, message)
#define LOG_INFO(message) LOG(g_logger, pico::LogLevel::INFO, message)
#define LOG_WARN(message) LOG(g_logger, pico::LogLevel::WARN, message)
#define LOG_ERROR(message) LOG(g_logger, pico::LogLevel::ERROR, message)
#define LOG_FATAL(message) LOG(g_logger, pico::LogLevel::FATAL, message)

// #define LOG_DEBUG_DEF(message) LOG_DEBUG(g_logger, message)
// #define LOG_INFO_DEF(message) LOG_INFO(g_logger, message)
// #define LOG_WARN_DEF(message) LOG_WARN(g_logger, message)
// #define LOG_ERROR_DEF(message) LOG_ERROR(g_logger, message)
// #define LOG_FATAL_DEF(message) LOG_FATAL(g_logger, message)

#define LOG_FMT(logger, level, fmt, ...)                                \
    logger->log(                                                        \
        level,                                                          \
        pico::LogEvent::Ptr(new pico::LogEvent(logger,                  \
                                               level,                   \
                                               __FILE__,                \
                                               __LINE__,                \
                                               pico::getThreadId(),     \
                                               0,                       \
                                               time(0),                 \
                                               pico::Thread::GetName(), \
                                               pico::LogEvent::format(fmt, ##__VA_ARGS__))))

#define LOG_FMT_DEBUG(fmt, ...) LOG_FMT(g_logger, pico::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define LOG_FMT_INFO(fmt, ...) LOG_FMT(g_logger, pico::LogLevel::INFO, fmt, ##__VA_ARGS__)
#define LOG_FMT_WARN(fmt, ...) LOG_FMT(g_logger, pico::LogLevel::WARN, fmt, ##__VA_ARGS__)
#define LOG_FMT_ERROR(fmt, ...) LOG_FMT(g_logger, pico::LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define LOG_FMT_FATAL(fmt, ...) LOG_FMT(g_logger, pico::LogLevel::FATAL, fmt, ##__VA_ARGS__)

#endif