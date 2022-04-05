#include "timer.h"
#include "logging.h"
#include "util.h"
#include <algorithm>

namespace pico {
Timer::Timer(uint64_t timeout, Callback callback, bool repeat, TimerManager* manager)
    : m_repeat(repeat)
    , m_interval(timeout)
    , m_callback(callback)
    , m_manager(manager) {
    m_next = m_interval + pico::getCurrentTime();
}

Timer::Timer(uint64_t next)
    : m_repeat(false)
    , m_interval(0)
    , m_next(next) {}

bool Timer::cancel() {
    TimerManager::MutexType::WriteLock wlock(m_manager->m_mutex);
    if (m_callback) {
        m_callback = nullptr;
        auto it =
            std::find(m_manager->m_timers.begin(), m_manager->m_timers.end(), shared_from_this());
        if (it != m_manager->m_timers.end()) { m_manager->m_timers.erase(it); }
        return true;
    }
    return false;
}

bool Timer::refresh() {
    TimerManager::MutexType::WriteLock wlock(m_manager->m_mutex);
    if (m_callback) {
        auto it =
            std::find(m_manager->m_timers.begin(), m_manager->m_timers.end(), shared_from_this());
        if (it != m_manager->m_timers.end()) {
            m_manager->m_timers.erase(it);
            m_next = m_interval + pico::getCurrentTime();
            m_manager->m_timers.insert(shared_from_this());
            return true;
        }
    }
    return false;
}

bool Timer::reset(uint64_t timeout, bool fromNow) {
    if (timeout == m_interval && !fromNow) { return true; }
    TimerManager::MutexType::WriteLock wlock(m_manager->m_mutex);
    if (m_callback) {
        auto it =
            std::find(m_manager->m_timers.begin(), m_manager->m_timers.end(), shared_from_this());
        if (it != m_manager->m_timers.end()) {
            m_manager->m_timers.erase(it);
            m_interval = timeout;
            if (fromNow) { m_next = pico::getCurrentTime() + timeout; }
            else {
                m_next = m_interval + pico::getCurrentTime();
            }
            m_manager->m_timers.insert(shared_from_this());
            return true;
        }
    }
    return false;
}

TimerManager::TimerManager() {
    m_previous = pico::getCurrentTime();
}

TimerManager::~TimerManager() {}

Timer::Ptr TimerManager::addTimer(uint64_t timeout, Callback callback, bool repeat) {
    Timer::Ptr timer(new Timer(timeout, callback, repeat, this));
    TimerManager::MutexType::WriteLock wlock(m_mutex);
    addTimer(timer, wlock);
    return timer;
}

void TimerManager::addTimer(Timer::Ptr timer, MutexType::WriteLock& lock) {
    auto it = m_timers.insert(timer).first;
    bool at_fronted = (it == m_timers.begin()) && !m_tickled;
    if (at_fronted) { m_tickled = true; }
    lock.unlock();
    if (at_fronted) { onTimerInsertAtFront(); }
}

static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> callback) {
    if (auto cond = weak_cond.lock()) { callback(); }
}

Timer::Ptr TimerManager::addCondTimer(uint64_t timeout, Callback callback,
                                      std::weak_ptr<void> weak_cond, bool repeat) {
    return addTimer(timeout, std::bind(&OnTimer, weak_cond, callback), repeat);
}

uint64_t TimerManager::getNextTimer() {
    TimerManager::MutexType::ReadLock rlock(m_mutex);
    if (m_timers.empty()) { return ~0ull; }

    const Timer::Ptr& timer = *m_timers.begin();
    uint64_t time_now = pico::getCurrentTime();
    if (timer->m_next <= time_now) { return 0; }
    return timer->m_next - time_now;
}

void TimerManager::listExperiedCb(std::vector<Callback>& cbs) {
    uint64_t time_now = pico::getCurrentTime();
    std::vector<Timer::Ptr> expired;
    {
        TimerManager::MutexType::ReadLock rlock(m_mutex);
        if (m_timers.empty()) { return; }
    }
    MutexType::WriteLock wlock(m_mutex);
    if (m_timers.empty()) { return; }
    bool rollover = detectClockRollover(time_now);
    if (!rollover && ((*m_timers.begin())->m_next > time_now)) return;

    Timer::Ptr timer(new Timer(time_now));
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(timer);
    while (it != m_timers.end() && (*it)->m_next == time_now) { it++; }
    expired.insert(expired.end(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it);
    cbs.reserve(expired.size());
    for (auto& i : expired) {
        cbs.push_back(i->m_callback);
        if (i->m_repeat) {
            i->m_next = i->m_interval + time_now;
            m_timers.insert(i);
        }
        else {
            i->m_callback = nullptr;
        }
    }
}


bool TimerManager::detectClockRollover(uint64_t time_now) {
    if (time_now < m_previous && m_previous - time_now > 10000) { return true; }
    m_previous = time_now;
    return false;
}

bool TimerManager::hasTimer() {
    TimerManager::MutexType::ReadLock rlock(m_mutex);
    return !m_timers.empty();
}

}   // namespace pico