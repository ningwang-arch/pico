#include "Logger.h"

#include <unordered_map>

namespace pico {
class LoggerManager
{
public:
    typedef Mutex MutexType;
    LoggerManager();

    Logger::Ptr getRootLogger();

    void addLogger(std::string& name, Logger::Ptr logger);

    Logger::Ptr getLogger(const std::string& name);

private:
    MutexType m_mutex;
    Logger::Ptr m_root;

    std::unordered_map<std::string, Logger::Ptr> m_loggers;
};
}   // namespace pico