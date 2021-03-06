#ifndef __PICO_IOMANAGER_H__
#define __PICO_IOMANAGER_H__

#include <functional>
#include <memory>
#include <string>

#include "scheduler.h"
#include "timer.h"

namespace pico {

class IOManager : public Scheduler, public TimerManager
{
public:
    typedef std::shared_ptr<IOManager> Ptr;
    typedef RWMutex RWMutexType;
    enum Event
    {
        /// 无事件
        NONE = 0x0,
        /// 读事件(EPOLLIN)
        READ = 0x1,
        /// 写事件(EPOLLOUT)
        WRITE = 0x4,
    };

private:
    struct FdContext
    {
        typedef Mutex MutexType;
        struct EventContext
        {
            Scheduler* scheduler = nullptr;
            Fiber::Ptr fiber;
            std::function<void()> cb;
        };
        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);

        void triggerEvent(Event event);

        EventContext read;
        EventContext write;
        int fd = 0;
        Event events = NONE;
        MutexType mutex;
    };

public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    ~IOManager();
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);

    bool cancelAll(int fd);

    static IOManager* GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;
    void onTimerInsertAtFront() override;
    void contextResize(size_t size);
    bool stopping(uint64_t& timeout);

private:
    int m_epfd = 0;
    int m_tickleFds[2];
    std::atomic<size_t> m_pendingEventCount = {0};
    RWMutexType m_mutex;
    std::vector<FdContext*> m_fdContexts;
};


}   // namespace pico

#endif