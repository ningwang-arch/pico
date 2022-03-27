#include "scheduler.h"
#include "hook.h"
#include "logging.h"
#include "util.h"
#include <assert.h>

namespace pico {
static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_fiber = nullptr;

Scheduler::Scheduler(int threads, bool use_caller, const std::string& name)
    : m_name(name) {
    assert(threads > 0);

    if (use_caller) {
        pico::Fiber::GetThis();
        --threads;
        assert(GetThis() == nullptr);

        t_scheduler = this;
        m_main_fiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        pico::Thread::SetName(m_name);

        t_fiber = m_main_fiber.get();
        m_main_thread_id = pico::getThreadId();
        m_thread_ids.push_back(m_main_thread_id);
    }
    else {
        m_main_thread_id = -1;
    }

    m_thread_count = threads;
}

Scheduler::~Scheduler() {
    assert(m_stopping);
    if (GetThis() == this) { t_scheduler = nullptr; }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if (!m_stopping) { return; }
    m_stopping = false;
    assert(m_threads.empty());

    m_threads.resize(m_thread_count);

    for (size_t i = 0; i < m_thread_count; ++i) {
        m_threads[i].reset(
            new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
        m_thread_ids.push_back(m_threads[i]->getId());
    }

    lock.unlock();
}

void Scheduler::stop() {
    m_autoStop = true;
    if (m_main_fiber && m_thread_count == 0 &&
        (m_main_fiber->getState() == Fiber::INIT || m_main_fiber->getState() == Fiber::EXIT)) {
        LOG_INFO("Scheduler::stop() called from main fiber, exiting immediately");
        m_stopping = true;

        if (stopping()) { return; }
    }

    if (m_main_thread_id != -1) { assert(GetThis() == this); }
    else {
        assert(GetThis() != this);
    }

    m_stopping = true;

    for (size_t i = 0; i < m_thread_count; ++i) { tickle(); }

    if (m_main_fiber) { tickle(); }

    if (m_main_fiber) {
        if (!stopping()) { m_main_fiber->call(); }
    }
    std::vector<Thread::Ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for (auto& thr : thrs) { thr->join(); }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::run() {
    LOG_INFO("Scheduler::run() started");
    set_hook_enable(true);
    setThis();
    if (pico::getThreadId() != m_main_thread_id) { t_fiber = Fiber::GetThis().get(); }

    Fiber::Ptr idle_ptr(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::Ptr cb_fiber;

    SchedulerTask task;
    while (true) {
        task.reset();
        bool tickle_me = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_tasks.begin();
            while (it != m_tasks.end()) {
                if (it->thread != -1 && it->thread != pico::getThreadId()) {
                    it++;
                    tickle_me = true;
                    continue;
                }
                assert(it->fiber || it->func);
                if (it->fiber && it->fiber->getState() == Fiber::RUNNING) {
                    ++it;
                    continue;
                }
                task = *it;
                m_tasks.erase(it);
                ++m_active_count;
                break;
            }

            tickle_me |= (it != m_tasks.end());
        }
        if (tickle_me) { tickle(); }
        if (task.fiber && task.fiber->getState() != Fiber::EXIT) {
            task.fiber->swapIn();
            --m_active_count;
            if (task.fiber->getState() == Fiber::READY) { schedule(task.fiber); }
            else if (task.fiber->getState() != Fiber::EXIT) {
                task.fiber->m_state = Fiber::SUSPEND;
            }
            task.reset();
        }
        else if (task.func) {
            if (cb_fiber) { cb_fiber->reset(task.func); }
            else {
                cb_fiber.reset(new Fiber(task.func));
            }
            task.reset();
            cb_fiber->swapIn();
            --m_active_count;
            if (cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            }
            else if (cb_fiber->getState() == Fiber::EXIT) {
                cb_fiber->reset(nullptr);
            }
            else {
                cb_fiber->m_state = Fiber::SUSPEND;
                cb_fiber.reset();
            }
        }
        else {
            if (idle_ptr->getState() == Fiber::EXIT) {
                LOG_INFO("idle fiber exit");
                break;
            }
            ++m_idle_count;
            idle_ptr->swapIn();
            --m_idle_count;
            if (idle_ptr->getState() != Fiber::EXIT) idle_ptr->m_state = Fiber::SUSPEND;
        }
    }
}

void Scheduler::tickle() {
    LOG_INFO("Scheduler::tickle()");
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping && m_tasks.empty() && m_active_count == 0;
}

void Scheduler::idle() {
    LOG_INFO("Scheduler::idle()");
    while (!stopping()) { pico::Fiber::yieldToSuspend(); }
}

void Scheduler::switchTo(int thread) {
    assert(Scheduler::GetThis() != nullptr);
    if (Scheduler::GetThis() == this) {
        if (thread == -1 || thread == pico::getThreadId()) { return; }
    }
    schedule(Fiber::GetThis(), thread);
    Fiber::yieldToSuspend();
}

}   // namespace pico