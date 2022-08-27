#ifndef __PICO_TIMER_H__
#define __PICO_TIMER_H__

#include <algorithm>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "mutex.h"

namespace pico {

class TimerManager;

class Timer : public std::enable_shared_from_this<Timer>
{
    friend class TimerManager;

public:
    typedef std::shared_ptr<Timer> Ptr;
    typedef std::function<void(void)> Callback;

    bool cancel();
    bool refresh();
    bool reset(uint64_t timeout, bool fromNow);

private:
    Timer(uint64_t timeout, Callback callback, bool repeat, TimerManager* manager);
    explicit Timer(uint64_t next);

private:
    bool m_repeat;
    uint64_t m_interval;
    uint64_t m_next;
    Callback m_callback;
    TimerManager* m_manager;

private:
    struct Compare
    {
        bool operator()(const Ptr& lhs, const Ptr& rhs) const { return lhs->m_next < rhs->m_next; }
    };
};

class TimerManager
{
    friend class Timer;

public:
    typedef RWMutex MutexType;
    typedef std::function<void(void)> Callback;

    TimerManager();
    virtual ~TimerManager();

    Timer::Ptr addTimer(uint64_t timeout, Timer::Callback callback, bool repeat = false);
    void addTimer(Timer::Ptr timer, MutexType::WriteLock& lock);

    Timer::Ptr addCondTimer(uint64_t timeout, Callback callback, std::weak_ptr<void> weak_cond,
                            bool repeat = false);

    uint64_t getNextTimer();

    void listExpiredCb(std::vector<Callback>& callbacks);
    bool hasTimer();

protected:
    virtual void onTimerInsertAtFront() = 0;

private:
    bool detectClockRollover(uint64_t time_now);

private:
    std::set<Timer::Ptr, Timer::Compare> m_timers;
    uint64_t m_previous;
    bool m_tickled = false;
    MutexType m_mutex;
};

}   // namespace pico


#endif