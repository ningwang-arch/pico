#ifndef __PICO_SCEDULER_H__
#define __PICO_SCEDULER_H__

#include "fiber.h"
#include "mutex.h"
#include "thread.h"
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace pico {
class Scheduler
{
public:
    typedef std::shared_ptr<Scheduler> Ptr;
    typedef Mutex MutexType;

    Scheduler(int threads = 1, bool use_caller = true, const std::string& name = "scheduler");

    virtual ~Scheduler();

    const std::string getName() const { return m_name; }

    static Scheduler* GetThis();
    static Fiber* GetMainFiber();


    /**
     * @brief Run the scheduler.
     *
     * This function will block until the scheduler is stopped.
     */
    void start();

    /**
     * @brief stop
     * @return
     * @note stop the scheduler, and wait for all fibers to finish
     **/
    void stop();

    /**
     * @brief 调度协程
     * @param  fc  fiber/function
     * @param thr  线程id
     * */
    template<class FiberOrCb>
    void schedule(FiberOrCb fc, int thr = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thr);
        }

        if (need_tickle) {
            // wake up the scheduler
            tickle();
        }
    }

    /**
     * @brief 调度协程队列
     * @param  first 队列头
     * @param  last 队列尾
     * */
    template<class InputIterator>
    void schedule(InputIterator first, InputIterator last) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while (first != last) {
                need_tickle |= scheduleNoLock(*first, -1);
                ++first;
            }
        }

        if (need_tickle) {
            // wake up the scheduler
            tickle();
        }
    }

    void switchTo(int thread = -1);

protected:
    /**
     * wake up the scheduler
     */
    virtual void tickle();

    /**
     * 协程调度函数
     */
    void run();

    /**
     * 判断协程是否可停止
     */
    virtual bool stopping();

    /**
     * 无任务调度执行此函数
     */
    virtual void idle();

    void setThis();


    bool hasIdleThread() { return m_idle_count > 0; }


private:
    template<class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thr = -1) {
        bool need_tickle = m_tasks.empty();
        {
            SchedulerTask task(fc, thr);
            if (task.fiber || task.func) { m_tasks.push_back(task); }
        }

        return need_tickle;
    }


private:
    struct SchedulerTask
    {
        Fiber::Ptr fiber;
        std::function<void()> func;
        int thread;

        SchedulerTask(Fiber::Ptr f, int t)
            : fiber(f)
            , thread(t) {}

        SchedulerTask(Fiber::Ptr* f, int t)
            : thread(t) {
            fiber.swap(*f);
        }

        SchedulerTask(std::function<void()> f, int t)
            : func(f)
            , thread(t) {}

        SchedulerTask(std::function<void()>* f, int t)
            : thread(t) {
            func.swap(*f);
        }

        SchedulerTask()
            : thread(-1) {}

        void reset() {
            fiber = nullptr;
            func = nullptr;
            thread = -1;
        }
    };


private:
    std::string m_name;
    std::vector<std::shared_ptr<Thread>> m_threads;
    std::list<SchedulerTask> m_tasks;
    Fiber::Ptr m_main_fiber;
    MutexType m_mutex;

protected:
    std::vector<int> m_thread_ids;
    size_t m_thread_count = 0;
    bool m_stopping = true;
    std::atomic<size_t> m_idle_count{0};
    std::atomic<size_t> m_active_count{0};

    bool m_autoStop = false;
    int m_main_thread_id = 0;
};
}   // namespace pico

#endif