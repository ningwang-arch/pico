#include "Logger.h"


namespace pico {
class LoggerManager
{
public:
    typedef Mutex MutexType;
    LoggerManager();

    Logger::Ptr getLogger();

private:
    MutexType m_mutex;
    Logger::Ptr m_root;
};
}   // namespace pico