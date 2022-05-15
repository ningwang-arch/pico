#ifndef __PI_FIBER_H__
#define __PI_FIBER_H__

#include <functional>
#include <memory>
#include <ucontext.h>

namespace pico {
class Scheduler;
class Scheduler;

/**
 * @brief 协程类
 */
class Fiber : public std::enable_shared_from_this<Fiber>
{
    friend class Scheduler;

public:
    typedef std::shared_ptr<Fiber> Ptr;

    enum State
    {
        /// 初始化状态
        INIT,
        /// 暂停状态
        HOLD,
        /// 执行中状态
        EXEC,
        /// 结束状态
        TERM,
        /// 可执行状态
        READY,
        /// 异常状态
        EXCEPT
    };

private:
    Fiber();

public:
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();

    void reset(std::function<void()> cb);

    void swapIn();
    void swapOut();
    void call();

    void back();
    uint64_t getId() const { return m_id; }
    State getState() const { return m_state; }

public:
    static void SetThis(Fiber* f);
    static Fiber::Ptr GetThis();
    static void yieldToReady();
    static void yieldToHold();
    static uint64_t TotalFibers();
    static void MainFunc();
    static void CallerMainFunc();
    static uint64_t GetFiberId();

private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;
    ucontext_t m_ctx;
    void* m_stack = nullptr;
    std::function<void()> m_cb;
};
};   // namespace pico

#endif