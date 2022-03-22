#ifndef __PICO_THREAD_H__
#define __PICO_THREAD_H__

#include <functional>
#include <memory>
#include <pthread.h>
#include <string>

#include "mutex.h"
#include "noncopyable.h"

namespace pico {
class Thread : Noncopyable
{
public:
    typedef std::shared_ptr<Thread> Ptr;
    typedef std::function<void()> ThreadFunc;

    explicit Thread(const ThreadFunc& func, const std::string& name = std::string());
    ~Thread();

    pid_t getId() const { return m_id; }
    const std::string& getName() const { return m_name; }

    void join();

    static Thread* GetThis();

    static const std::string& GetName();

    static void SetName(const std::string& name);

private:
    static void* run(void* arg);

private:
    pid_t m_id = -1;
    pthread_t m_pthreadId = 0;
    ThreadFunc m_func;
    std::string m_name;
    Semaphore m_sem;
};
}   // namespace pico

#endif
