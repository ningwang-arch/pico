#ifndef __PICO_MUTEX_H__
#define __PICO_MUTEX_H__

#include <pthread.h>
#include <semaphore.h>

#include <atomic>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

#include "noncopyable.h"



namespace pico {
class Semaphore : Noncopyable
{
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();

    void notify();

private:
    Semaphore(const Semaphore&) = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

private:
    sem_t m_semaphore;
};

template<class T>
struct ScopedLockImpl
{
public:
    /**
     * @brief 构造函数
     * @param[in] mutex Mutex
     */
    explicit ScopedLockImpl(T& mutex)
        : m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }

    /**
     * @brief 析构函数,自动释放锁
     */
    ~ScopedLockImpl() { unlock(); }

    /**
     * @brief 加锁
     */
    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    /**
     * @brief 解锁
     */
    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    /// mutex
    T& m_mutex;
    /// 是否已上锁
    bool m_locked;
};
template<class T>
struct ReadScopedLockImpl
{
public:
    /**
     * @brief 构造函数
     * @param[in] mutex 读写锁
     */
    explicit ReadScopedLockImpl(T& mutex)
        : m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }

    /**
     * @brief 析构函数,自动释放锁
     */
    ~ReadScopedLockImpl() { unlock(); }

    /**
     * @brief 上读锁
     */
    void lock() {
        if (!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    /**
     * @brief 释放锁
     */
    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    /// mutex
    T& m_mutex;
    /// 是否已上锁
    bool m_locked;
};

template<class T>
struct WriteScopedLockImpl
{
public:
    /**
     * @brief 构造函数
     * @param[in] mutex 读写锁
     */
    explicit WriteScopedLockImpl(T& mutex)
        : m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }

    /**
     * @brief 析构函数
     */
    ~WriteScopedLockImpl() { unlock(); }

    /**
     * @brief 上写锁
     */
    void lock() {
        if (!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    /**
     * @brief 解锁
     */
    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    /// Mutex
    T& m_mutex;
    /// 是否已上锁
    bool m_locked;
};

class Mutex : Noncopyable
{
public:
    typedef ScopedLockImpl<Mutex> Lock;
    Mutex() { pthread_mutex_init(&m_mutex, nullptr); }
    ~Mutex() { pthread_mutex_destroy(&m_mutex); }

    void lock() { pthread_mutex_lock(&m_mutex); }

    void unlock() { pthread_mutex_unlock(&m_mutex); }

private:
    pthread_mutex_t m_mutex;
};

class NullMutex : Noncopyable
{
public:
    /// 局部锁
    typedef ScopedLockImpl<NullMutex> Lock;

    /**
     * @brief 构造函数
     */
    NullMutex() {}

    /**
     * @brief 析构函数
     */
    ~NullMutex() {}

    /**
     * @brief 加锁
     */
    void lock() {}

    /**
     * @brief 解锁
     */
    void unlock() {}
};

class RWMutex : Noncopyable
{
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;
    RWMutex() { pthread_rwlock_init(&m_lock, nullptr); }
    ~RWMutex() { pthread_rwlock_destroy(&m_lock); }

    void rdlock() { pthread_rwlock_rdlock(&m_lock); }

    void wrlock() { pthread_rwlock_wrlock(&m_lock); }

    void unlock() { pthread_rwlock_unlock(&m_lock); }

private:
    pthread_rwlock_t m_lock;
};

class NullRWMutex : Noncopyable
{
public:
    /// 局部读锁
    typedef ReadScopedLockImpl<NullMutex> ReadLock;
    /// 局部写锁
    typedef WriteScopedLockImpl<NullMutex> WriteLock;

    /**
     * @brief 构造函数
     */
    NullRWMutex() {}
    /**
     * @brief 析构函数
     */
    ~NullRWMutex() {}

    /**
     * @brief 上读锁
     */
    void rdlock() {}

    /**
     * @brief 上写锁
     */
    void wrlock() {}
    /**
     * @brief 解锁
     */
    void unlock() {}
};

class Spinlock : Noncopyable
{
public:
    typedef ScopedLockImpl<Spinlock> Lock;
    Spinlock() { pthread_spin_init(&m_mutex, 0); }
    ~Spinlock() { pthread_spin_destroy(&m_mutex); }

    void lock() { pthread_spin_lock(&m_mutex); }

    void unlock() { pthread_spin_unlock(&m_mutex); }

private:
    pthread_spinlock_t m_mutex;
};

class CASLock : Noncopyable
{
public:
    typedef ScopedLockImpl<CASLock> Lock;

    CASLock() { m_mutex.clear(); }
    ~CASLock() {}

    void lock() {
        while (std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire))
            ;
    }

    void unlock() { std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release); }

private:
    volatile std::atomic_flag m_mutex;
};


}   // namespace pico

#endif