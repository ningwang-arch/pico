#include "thread.h"
#include "logging.h"
#include <string.h>

#include "util.h"

namespace pico {
static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_threadName = "UNKNOWN";

Thread* Thread::GetThis() {
    return t_thread;
}

const std::string& Thread::GetName() {
    return t_threadName;
}

void Thread::SetName(const std::string& name) {
    if (t_thread) t_thread->m_name = name;

    t_threadName = name;
}

Thread::Thread(const ThreadFunc& func, const std::string& name)
    : m_func(func)
    , m_name(name) {
    if (m_name.empty()) { m_name = "pico::Thread"; }

    int rt = pthread_create(&m_pthreadId, nullptr, &Thread::run, this);
    if (rt) {
        LOG_FMT_FATAL("pthread_create error: %s, name= %s", strerror(rt), m_name);
        throw std::logic_error("pthread_create error");
    }

    m_sem.wait();
}

Thread::~Thread() {
    if (m_pthreadId) {
        pthread_detach(m_pthreadId);
        m_pthreadId = 0;
    }
}

void* Thread::run(void* arg) {
    Thread* thread = static_cast<Thread*>(arg);
    t_thread = thread;
    t_threadName = thread->m_name;
    thread->m_id = pico::getThreadId();

    pthread_setname_np(pthread_self(), thread->m_name.substr(1, 15).c_str());

    ThreadFunc func;

    func.swap(thread->m_func);

    thread->m_sem.notify();

    func();

    return 0;
}

void Thread::join() {
    if (m_pthreadId) {
        int rt = pthread_join(m_pthreadId, nullptr);
        if (rt) {
            LOG_FMT_FATAL("pthread_join error: %s, name= %s", strerror(rt), m_name);
            throw std::logic_error("pthread_join error");
        }

        m_pthreadId = 0;
    }
}

}   // namespace pico