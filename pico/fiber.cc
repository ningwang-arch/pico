#include "fiber.h"
#include "logging.h"
#include "scheduler.h"
#include <assert.h>

namespace pico {
static std::atomic<uint64_t> s_fiber_id{0};
static std::atomic<uint64_t> s_fiber_count{0};

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::Ptr t_threadFiber = nullptr;

static uint32_t g_fiber_stack_size = 128 * 1024;

class MallocStackAllocator
{
public:
    static void* Alloc(size_t size) { return malloc(size); }

    static void Dealloc(void* vp, size_t size) { return free(vp); }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
    if (t_fiber) { return t_fiber->getId(); }
    return 0;
}

Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if (getcontext(&m_ctx)) { assert(false); }

    ++s_fiber_count;
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
    : m_id(++s_fiber_id)
    , m_cb(cb) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size;

    m_stack = StackAllocator::Alloc(m_stacksize);
    if (getcontext(&m_ctx)) { assert(false); }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if (!use_caller) { makecontext(&m_ctx, &Fiber::MainFunc, 0); }
    else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }
}

Fiber::~Fiber() {
    --s_fiber_count;
    if (m_stack) {
        assert(m_state == TERM || m_state == EXCEPT || m_state == INIT);

        StackAllocator::Dealloc(m_stack, m_stacksize);
    }
    else {
        assert(!m_cb);
        assert(m_state == EXEC);

        Fiber* cur = t_fiber;
        if (cur == this) { SetThis(nullptr); }
    }
}

//重置协程函数，并重置状态
// INIT，TERM, EXCEPT
void Fiber::reset(std::function<void()> cb) {
    assert(m_stack);
    assert(m_state == TERM || m_state == EXCEPT || m_state == INIT);
    m_cb = cb;
    if (getcontext(&m_ctx)) { assert(false); }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) { assert(false); }
}

void Fiber::back() {
    SetThis(t_threadFiber.get());
    if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) { assert(false); }
}

//切换到当前协程执行
void Fiber::swapIn() {
    SetThis(this);
    assert(m_state != EXEC);
    m_state = EXEC;
    if (swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) { assert(false); }
}

//切换到后台执行
void Fiber::swapOut() {
    SetThis(Scheduler::GetMainFiber());
    if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) { assert(false); }
}

//设置当前协程
void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

//返回当前协程
Fiber::Ptr Fiber::GetThis() {
    if (t_fiber) { return t_fiber->shared_from_this(); }
    Fiber::Ptr main_fiber(new Fiber);
    assert(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

//协程切换到后台，并且设置为Ready状态
void Fiber::yieldToReady() {
    Fiber::Ptr cur = GetThis();
    assert(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}

//协程切换到后台，并且设置为Hold状态
void Fiber::yieldToHold() {
    Fiber::Ptr cur = GetThis();
    assert(cur->m_state == EXEC);
    cur->swapOut();
}

//总协程数
uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::Ptr cur = GetThis();
    assert(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }
    catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        LOG_ERROR("Fiber::MainFunc exception: %s", ex.what());
    }
    catch (...) {
        LOG_ERROR("Fiber::MainFunc unknown exception");
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    assert(false);
}

void Fiber::CallerMainFunc() {
    Fiber::Ptr cur = GetThis();
    assert(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }
    catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        LOG_ERROR("Fiber::CallerMainFunc exception: %s", ex.what());
    }
    catch (...) {
        cur->m_state = EXCEPT;
        LOG_ERROR("Fiber::CallerMainFunc unknown exception");
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    assert(false);
}

}   // namespace pico