#include "iomanager.h"
#include <algorithm>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "logging.h"

namespace pico {
IOManager::FdContext::EventContext& IOManager::FdContext::getEventContext(Event event) {
    switch (event) {
    case READ: return read;
    case WRITE: return write;
    default: return read;
    }
}

void IOManager::FdContext::resetEventContext(EventContext& eventContext) {
    eventContext.scheduler = nullptr;
    eventContext.fiber.reset();
    eventContext.callback = nullptr;
}

void IOManager::FdContext::triggerEvent(Event event) {
    assert(events && event);
    events = (Event)(events & ~event);
    EventContext& eventContext = getEventContext(event);
    if (eventContext.callback) { eventContext.scheduler->schedule(&eventContext.callback); }
    else {
        eventContext.scheduler->schedule(&eventContext.fiber);
    }

    eventContext.scheduler = nullptr;
    return;
}

IOManager::IOManager(int threads, bool use_caller, const std::string& name)
    : Scheduler(threads, use_caller, name) {
    m_epoll_fd = epoll_create(10);
    assert(m_epoll_fd > 0);

    int rt = pipe(m_tickle_fd);
    assert(!rt);

    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_tickle_fd[0];

    rt = fcntl(m_tickle_fd[0], F_SETFL, fcntl(m_tickle_fd[0], F_GETFL) | O_NONBLOCK);
    assert(!rt);

    rt = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_tickle_fd[0], &event);
    assert(!rt);

    contextResize(32);

    start();
}

IOManager::~IOManager() {
    stop();
    close(m_epoll_fd);
    close(m_tickle_fd[0]);
    close(m_tickle_fd[1]);

    for (auto& context : m_contexts) { delete context; }
    m_contexts.clear();
}

void IOManager::contextResize(size_t size) {
    if (m_contexts.size() >= size) { return; }

    m_contexts.resize(size);
    for (size_t i = 0; i < m_contexts.size(); ++i) {
        if (!m_contexts[i]) {
            m_contexts[i] = new FdContext;
            m_contexts[i]->fd = i;
        }
    }
}

int IOManager::addEvent(int fd, Event event, Callback callback) {
    FdContext* fd_ctx = nullptr;
    MutexType::ReadLock rlock(m_mutex);
    if (fd < (int)m_contexts.size()) {
        fd_ctx = m_contexts[fd];
        rlock.unlock();
    }
    else {
        rlock.unlock();
        MutexType::WriteLock wlock(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_contexts[fd];
    }

    FdContext::Type::Lock lock(fd_ctx->mutex);
    if (fd_ctx->events & event) {
        LOG_ERROR("fd:%d event:%d already exist", fd, event);
        return -1;
    }

    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = event | EPOLLET | fd_ctx->events;
    ev.data.ptr = fd_ctx;
    int rt = epoll_ctl(m_epoll_fd, op, fd, &ev);


    if (rt) {
        LOG_ERROR("epoll_ctl fd:%d event:%d error:%d", fd, event, rt);
        return -1;
    }

    m_penddingEvent++;

    fd_ctx->events = (Event)(fd_ctx->events | event);

    FdContext::EventContext& eventContext = fd_ctx->getEventContext(event);
    assert(!eventContext.scheduler && !eventContext.fiber && !eventContext.callback);

    eventContext.scheduler = Scheduler::GetThis();
    if (callback) { eventContext.callback.swap(callback); }
    else {
        eventContext.fiber = Fiber::GetThis();
        assert(eventContext.fiber->getState() == Fiber::State::RUNNING);
    }

    return 0;
}

bool IOManager::delEvent(int fd, Event event) {
    MutexType::ReadLock rlock(m_mutex);
    if (fd >= (int)m_contexts.size()) { return false; }

    FdContext* fd_ctx = m_contexts[fd];
    rlock.unlock();

    FdContext::Type::Lock lock(fd_ctx->mutex);
    if (!(fd_ctx->events & event)) { return false; }

    Event new_events = (Event)(fd_ctx->events & ~event);

    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

    epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = new_events | EPOLLET;
    ev.data.ptr = fd_ctx;
    int rt = epoll_ctl(m_epoll_fd, op, fd, &ev);

    if (rt) {
        LOG_ERROR("epoll_ctl fd:%d event:%d error:%d", fd, event, rt);
        return false;
    }

    --m_penddingEvent;

    fd_ctx->events = new_events;

    FdContext::EventContext& eventContext = fd_ctx->getEventContext(event);
    fd_ctx->resetEventContext(eventContext);

    return true;
}

bool IOManager::cancelEvent(int fd, Event event) {
    MutexType::ReadLock rlock(m_mutex);
    if (fd >= (int)m_contexts.size()) { return false; }

    FdContext* fd_ctx = m_contexts[fd];
    rlock.unlock();

    FdContext::Type::Lock lock(fd_ctx->mutex);
    if (!(fd_ctx->events & event)) { return false; }

    Event new_events = (Event)(fd_ctx->events & ~event);

    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

    epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = new_events | EPOLLET;
    ev.data.ptr = fd_ctx;
    int rt = epoll_ctl(m_epoll_fd, op, fd, &ev);

    if (rt) {
        LOG_ERROR("epoll_ctl fd:%d event:%d error:%d", fd, event, rt);
        return false;
    }
    fd_ctx->triggerEvent(event);
    --m_penddingEvent;
    return true;
}


bool IOManager::cancelAllEvent(int fd) {
    MutexType::ReadLock rlock(m_mutex);
    if (fd >= (int)m_contexts.size()) { return false; }

    FdContext* fd_ctx = m_contexts[fd];
    rlock.unlock();

    FdContext::Type::Lock lock(fd_ctx->mutex);
    if (!fd_ctx->events) { return false; }

    int op = EPOLL_CTL_DEL;

    epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = 0;
    ev.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epoll_fd, op, fd, &ev);
    if (rt) {
        LOG_ERROR("epoll_ctl fd:%d error:%d", fd, rt);
        return false;
    }

    if (fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_penddingEvent;
    }
    if (fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_penddingEvent;
    }

    assert(fd_ctx->events == 0);

    return true;
}

IOManager* IOManager::getThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle() {
    if (!hasIdleThread()) { return; }
    int rt = write(m_tickle_fd[1], "", 1);
    if (rt != 1) { LOG_ERROR("write error:%d", rt); }
}

bool IOManager::stopping(uint64_t ms) {
    ms = getNextTimer();
    return ms == ~0ull && m_penddingEvent == 0 && Scheduler::stopping();
}

bool IOManager::stopping() {
    return stopping(0);
}


void IOManager::idle() {

    const uint64_t MAX_EVENTS = 256;
    epoll_event* events = new epoll_event[MAX_EVENTS];
    std::shared_ptr<epoll_event> events_ptr(events, [](epoll_event* ptr) { delete[] ptr; });

    while (true) {
        uint64_t timeout = 0;
        if (stopping(timeout)) {
            timeout = getNextTimer();
            LOG_INFO("IOManager idle exit, name = %s", getName().c_str());
            break;
        }
        int rt = 0;
        do {
            static const int MAX_TIMEOUT = 3000;
            if (timeout != ~0ull) { timeout = (int)timeout > MAX_TIMEOUT ? MAX_TIMEOUT : timeout; }
            else {
                timeout = MAX_TIMEOUT;
            }
            rt = epoll_wait(m_epoll_fd, events, MAX_EVENTS, (int)timeout);
            if (rt < 0 && errno == EINTR) { continue; }
            else {
                break;
            }
        } while (true);
        std::vector<Callback> cbs;
        listExperiedCb(cbs);
        if (!cbs.empty()) {
            schedule(cbs.begin(), cbs.end());
            cbs.clear();
        }
        for (int i = 0; i < rt; ++i) {
            epoll_event& ev = events[i];
            if (ev.data.fd == m_tickle_fd[0]) {
                uint8_t buf[256];
                while (read(m_tickle_fd[0], buf, sizeof(buf)) > 0)
                    ;
                continue;
            }


            FdContext* fd_ctx = static_cast<FdContext*>(ev.data.ptr);
            if (fd_ctx == nullptr) { continue; }
            FdContext::Type::Lock lock(fd_ctx->mutex);
            if (ev.events & (EPOLLERR | EPOLLHUP)) { ev.events |= EPOLLIN | EPOLLOUT; }

            int real_event = NONE;
            if (ev.events & EPOLLIN) { real_event |= READ; }
            if (ev.events & EPOLLOUT) { real_event |= WRITE; }

            if ((fd_ctx->events & real_event) == NONE) { continue; }

            int left_events = fd_ctx->events & ~real_event;
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            ev.events = EPOLLET | left_events;

            int rt = epoll_ctl(m_epoll_fd, op, fd_ctx->fd, &ev);

            if (rt) {
                LOG_ERROR("epoll_ctl fd:%d error:%d", fd_ctx->fd, rt);
                continue;
            }

            if (real_event & READ) {
                fd_ctx->triggerEvent(READ);
                --m_penddingEvent;
            }
            if (real_event & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --m_penddingEvent;
            }
        }

        Fiber::Ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        raw_ptr->swapOut();
    }   // while
}

void IOManager::onTimerInsertAtFront() {
    tickle();
}

}   // namespace pico