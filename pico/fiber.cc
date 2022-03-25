#include "fiber.h"
#include "logging.h"
#include "scheduler.h"
#include <assert.h>

namespace pico {
static std::atomic<uint64_t> g_fiber_id{0};
static std::atomic<uint64_t> g_fiber_count{0};

static thread_local Fiber* g_fiber = nullptr;
static thread_local Fiber::Ptr g_thread_fiber = nullptr;

static const uint64_t kFiberStackSize = 1024 * 1024;

class MallocStackAllocator
{
public:
    static void* allocate(size_t size) { return malloc(size); }

    static void deallocate(void* ptr, size_t size) { return free(ptr); }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
    if (g_fiber != nullptr) { return g_fiber->getId(); }
    return 0;
}

Fiber::Fiber() {
    m_state = RUNNING;
    SetThis(this);

    if (getcontext(&m_ctx) == -1) {
        LOG_FATAL("getcontext failed");
        assert(false);
    }

    ++g_fiber_count;

    LOG_INFO("Fiber::Fiber() main");
}

Fiber::Fiber(FiberFunc func, size_t stacksize, bool use_caller)
    : m_id(++g_fiber_id)
    , m_func(func) {
    ++g_fiber_count;
    m_stacksize = stacksize == 0 ? kFiberStackSize : stacksize;

    m_stack = StackAllocator::allocate(m_stacksize);
    if (m_stack == nullptr) {
        LOG_FATAL("malloc stack failed");
        assert(false);
    }
    if (getcontext(&m_ctx) == -1) {
        LOG_FATAL("getcontext failed");
        assert(false);
    }
    m_ctx.uc_stack.ss_sp = reinterpret_cast<stack_t*>(m_stack);
    m_ctx.uc_stack.ss_size = m_stacksize;
    m_ctx.uc_link = nullptr;

    if (!use_caller) { makecontext(&m_ctx, &Fiber::MainFunc, 0); }
    else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }

    LOG_INFO("Fiber::Fiber(%p, %zu, %d) id=%lu", func, stacksize, use_caller, m_id);
}

Fiber::~Fiber() {
    --g_fiber_count;
    if (m_stack) {
        assert(m_state == EXIT || m_state == SUSPEND || m_state == INIT);
        StackAllocator::deallocate(m_stack, m_stacksize);
    }
    else {
        assert(!m_func);
        assert(m_state == RUNNING);

        Fiber* cur = g_fiber;
        if (cur == this) { SetThis(nullptr); }
    }
    LOG_INFO("Fiber::~Fiber() id=%lu", m_id);
}

void Fiber::reset(FiberFunc func) {
    assert(m_stack);
    assert(m_state == INIT || m_state == EXIT);
    m_func = func;

    if (getcontext(&m_ctx) == -1) {
        LOG_FATAL("getcontext failed");
        assert(false);
    }

    m_ctx.uc_stack.ss_sp = reinterpret_cast<stack_t*>(m_stack);
    m_ctx.uc_stack.ss_size = m_stacksize;
    m_ctx.uc_link = nullptr;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);

    m_state = INIT;
}

void Fiber::call() {
    SetThis(this);
    m_state = RUNNING;
    if (swapcontext(&g_thread_fiber->m_ctx, &m_ctx) == -1) {
        LOG_FATAL("swapcontext failed");
        assert(false);
    }
}

void Fiber::back() {
    SetThis(g_thread_fiber.get());
    if (swapcontext(&m_ctx, &g_thread_fiber->m_ctx) == -1) {
        LOG_FATAL("swapcontext failed");
        assert(false);
    }
}

void Fiber::swapIn() {
    SetThis(this);
    assert(m_state != RUNNING);
    m_state = RUNNING;
    if (swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx) == -1) {
        LOG_FATAL("swapcontext failed");
        assert(false);
    }
}

void Fiber::swapOut() {
    SetThis(g_thread_fiber.get());
    if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx) == -1) {
        LOG_FATAL("swapcontext failed");
        assert(false);
    }
}

void Fiber::SetThis(Fiber* fiber) {
    g_fiber = fiber;
}

Fiber::Ptr Fiber::GetThis() {
    if (g_fiber) return g_fiber->shared_from_this();

    Fiber::Ptr main_fiber(new Fiber);
    assert(g_fiber == main_fiber.get());
    g_thread_fiber = main_fiber;
    return g_fiber->shared_from_this();
}

void Fiber::yieldToReady() {
    Fiber::Ptr cur = GetThis();
    assert(cur->m_state == RUNNING);
    cur->m_state = READY;
    cur->swapOut();
}

void Fiber::yieldToSuspend() {
    Fiber::Ptr cur = GetThis();
    assert(cur->m_state == RUNNING);
    cur->swapOut();
}

uint64_t Fiber::GetTotalFiberCount() {
    return g_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::Ptr cur = GetThis();
    assert(cur);
    try {
        cur->m_func();
        cur->m_func = nullptr;
        cur->m_state = EXIT;
    }
    catch (std::exception& e) {
        LOG_ERROR("Fiber::MainFunc() exception: %s", e.what());
    }
    catch (...) {
        LOG_ERROR("Fiber::MainFunc() unknown exception");
    }
    auto raw = cur.get();
    cur.reset();
    raw->swapOut();

    LOG_FATAL("Fiber::MainFunc() should not be here fiber_id=%lu", raw->getId());

    assert(false);
}

void Fiber::CallerMainFunc() {
    Fiber::Ptr cur = GetThis();
    assert(cur);
    try {
        cur->m_func();
        cur->m_func = nullptr;
        cur->m_state = EXIT;
    }
    catch (std::exception& e) {
        LOG_ERROR("Fiber::CallerMainFunction() exception: %s", e.what());
    }
    catch (...) {
        LOG_ERROR("Fiber::CallerMainFunction() unknown exception");
    }
    auto raw = cur.get();
    cur.reset();
    raw->back();

    assert(false);
}
}   // namespace pico