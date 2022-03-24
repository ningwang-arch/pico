#ifndef __PI_FIBER_H__
#define __PI_FIBER_H__

#include <functional>
#include <memory>
#include <ucontext.h>

namespace pico {
class Scheduler;
class Fiber : public std::enable_shared_from_this<Fiber>
{
    friend class Scheduler;

public:
    typedef std::shared_ptr<Fiber> Ptr;
    typedef std::function<void()> FiberFunc;

    enum State
    {
        // 初始状态
        INIT,
        // 准备就绪
        READY,
        // 运行中
        RUNNING,
        // 挂起
        SUSPEND,
        // 已经完成
        EXIT
    };

private:
    Fiber();

public:
    Fiber(FiberFunc func, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();

    // 重置协程函数
    // pre: getState() == INIT||READY
    // after: getState=INIT
    void reset(FiberFunc func);

    // 切换到当前fiber
    // pre: getState() != RUNNING
    // after: getState() == RUNNING
    void swapIn();
    // 切换到另一个fiber
    void swapOut();

    // 将当前协程切换到执行状态
    // 执行者为主协程
    void call();
    // 将当前协程挂起
    // 执行者为该协程
    // 返回主协程
    void back();

    uint64_t getId() const { return m_id; }
    State getState() const { return m_state; }

public:
    static void SetThis(Fiber* f);
    static Fiber::Ptr GetThis();

    // 将当前协程切换到后台并设置状态为READY
    static void yieldToReady();

    // 将当前协程切换到后台并设置状态为SUSPEND
    static void yieldToSuspend();

    static uint64_t GetTotalFiberCount();

    static void MainFunc();

    static void CallerMainFunc();

    static uint64_t GetFiberId();


private:
    uint64_t m_id;
    ucontext_t m_ctx;
    FiberFunc m_func;
    State m_state = INIT;
    void* m_stack = nullptr;
    uint32_t m_stacksize = 0;
};
};   // namespace pico

#endif