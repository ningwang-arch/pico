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
    typedef std::function<void()> Callback;
    typedef RWMutex MutexType;

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
        typedef Mutex Type;
        struct EventContext
        {
            Scheduler* scheduler = nullptr;
            Fiber::Ptr fiber;
            Callback callback;
        };

        EventContext& getEventContext(Event event);
        void resetEventContext(EventContext& eventContext);
        void triggerEvent(Event event);

        Type mutex;
        EventContext read;
        EventContext write;
        Event events = NONE;
        int fd = 0;
    };

public:
    IOManager(int threads = 1, bool use_caller = true, const std::string& name = "IOManager");
    ~IOManager();

    int addEvent(int fd, Event event, Callback callback = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);
    bool cancelAllEvent(int fd);

    static IOManager* getThis();

protected:
    void tickle() override;
    bool stopping() override;
    bool stopping(uint64_t ms);
    void idle() override;
    void onTimerInsertAtFront() override;
    void contextResize(size_t size);


private:
    int m_epoll_fd = -1;
    std::vector<FdContext*> m_contexts;
    int m_tickle_fd[2];
    std::atomic<size_t> m_penddingEvent = {0};
    MutexType m_mutex;
};

}   // namespace pico

#endif