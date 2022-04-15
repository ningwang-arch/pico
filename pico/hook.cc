#include "hook.h"
#include "fdmanager.h"
#include "iomanager.h"
#include "logging.h"
#include <dlfcn.h>
#include <iostream>
#include <stdarg.h>

namespace pico {
static uint64_t g_tcp_connect_timeout = 5000;
static thread_local bool g_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep)        \
    XX(socket)       \
    XX(connect)      \
    XX(accept)       \
    XX(read)         \
    XX(readv)        \
    XX(recv)         \
    XX(recvfrom)     \
    XX(recvmsg)      \
    XX(write)        \
    XX(writev)       \
    XX(send)         \
    XX(sendto)       \
    XX(sendmsg)      \
    XX(close)        \
    XX(fcntl)        \
    XX(ioctl)        \
    XX(setsockopt)   \
    XX(getsockopt)


void hook_init() {
    static bool is_init = false;
    if (is_init) { return; }
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX)
#undef XX
}

struct _HookIniter
{
    _HookIniter() { hook_init(); }
};

static _HookIniter _hook_initer;

bool is_hook_enable() {
    return g_hook_enable;
}

void set_hook_enable(bool enable) {
    g_hook_enable = enable;
}

}   // namespace pico


struct timer_info
{
    int cancelled = 0;
};


template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun origin_fun, const char* hook_fun, uint32_t event,
                     int timeout_so, Args&&... args) {
    if (!pico::is_hook_enable()) { return origin_fun(fd, std::forward<Args>(args)...); }
    pico::FdCtx::Ptr ctx = pico::FdMgr::getInstance()->getFdCtx(fd);
    if (ctx == nullptr) { return origin_fun(fd, std::forward<Args>(args)...); }
    if (ctx->isClosed()) {
        errno = EBADF;
        return -1;
    }
    if (!ctx->isSocket() || ctx->isUserNonBlock()) {
        return origin_fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    ssize_t ret = origin_fun(fd, std::forward<Args>(args)...);
    while (ret == -1 && errno == EINTR) { ret = origin_fun(fd, std::forward<Args>(args)...); }
    if (ret == -1 && errno == EAGAIN) {
        pico::IOManager* iom = pico::IOManager::getThis();
        pico::Timer::Ptr timer;
        std::weak_ptr<timer_info> wp(tinfo);

        if (to != UINT64_C(-1)) {
            timer = iom->addCondTimer(
                to,
                [fd, wp, iom, event]() {
                    std::shared_ptr<timer_info> t = wp.lock();
                    if (t == nullptr || t->cancelled) { return; }

                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, (pico::IOManager::Event)(event));
                },
                wp);
        }

        int rt = iom->addEvent(fd, (pico::IOManager::Event)(event));

        if (rt) {
            LOG_ERROR("add event failed, fd: %d, event: %d", fd, event);
            if (timer) { timer->cancel(); }
            return -1;
        }
        else {
            pico::Fiber::yieldToSuspend();
            if (timer) { timer->cancel(); }
            if (tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }

            goto retry;
        }
    }
    return ret;
}


extern "C" {
#define XX(name) name##_fun name##_f = nullptr;
HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if (!pico::is_hook_enable()) { return sleep_f(seconds); }

    pico::Fiber::Ptr fiber = pico::Fiber::GetThis();
    pico::IOManager* io_manager = pico::IOManager::getThis();

    io_manager->addTimer(seconds * 1000,
                         std::bind((void(pico::IOManager::*)(pico::Fiber::Ptr, int thread)) &
                                       pico::IOManager::schedule,
                                   io_manager,
                                   fiber,
                                   -1));
    pico::Fiber::yieldToSuspend();

    return 0;
}

int connect_with_timeout(int sockfd, const struct sockaddr* addr, socklen_t addrlen,
                         uint64_t timeout) {
    if (!pico::is_hook_enable()) { return connect_f(sockfd, addr, addrlen); }

    pico::FdCtx::Ptr ctx = pico::FdMgr::getInstance()->getFdCtx(sockfd);
    if (!ctx || ctx->isClosed()) {
        errno = EBADF;
        return -1;
    }

    if (!ctx->isSocket()) { return connect_f(sockfd, addr, addrlen); }

    if (ctx->isUserNonBlock()) { return connect_f(sockfd, addr, addrlen); }

    int n = connect_f(sockfd, addr, addrlen);
    if (n == 0) { return 0; }
    else if (n != -1 || errno != EINPROGRESS) {
        return n;
    }

    pico::IOManager* iom = pico::IOManager::getThis();
    pico::Timer::Ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if (timeout != (uint64_t)-1) {
        timer = iom->addCondTimer(
            timeout,
            [winfo, sockfd, iom]() {
                auto t = winfo.lock();
                if (!t || t->cancelled) { return; }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(sockfd, pico::IOManager::WRITE);
            },
            winfo);
    }

    int rt = iom->addEvent(sockfd, pico::IOManager::WRITE);
    if (rt == 0) {
        pico::Fiber::yieldToSuspend();
        if (timer) { timer->cancel(); }
        if (tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    }
    else {
        if (timer) { timer->cancel(); }
        LOG_ERROR("addEvent failed");
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if (-1 == getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len)) { return -1; }
    if (!error) { return 0; }
    else {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, pico::g_tcp_connect_timeout);
}

int socket(int domain, int type, int protocol) {
    if (!pico::is_hook_enable()) { return socket_f(domain, type, protocol); }

    int fd = socket_f(domain, type, protocol);
    if (fd == -1) { return fd; }
    pico::FdMgr::getInstance()->getFdCtx(fd, true);
    return fd;
}

int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    int fd = do_io(sockfd, accept_f, "accept", pico::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if (fd >= 0) { pico::FdMgr::getInstance()->getFdCtx(fd, true); }
    return fd;
}

ssize_t read(int fd, void* buf, size_t count) {
    return do_io(fd, read_f, "read", pico::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec* iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", pico::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int fd, void* buf, size_t len, int flags) {
    return do_io(fd, recv_f, "recv", pico::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int fd, void* buf, size_t len, int flags, struct sockaddr* src_addr,
                 socklen_t* addrlen) {
    return do_io(fd,
                 recvfrom_f,
                 "recvfrom",
                 pico::IOManager::READ,
                 SO_RCVTIMEO,
                 buf,
                 len,
                 flags,
                 src_addr,
                 addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmmsg", pico::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void* buf, size_t count) {
    return do_io(fd, write_f, "write", pico::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec* iov, int iovcnt) {
    return do_io(fd, write_f, "writev", pico::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int fd, const void* buf, size_t len, int flags) {
    return do_io(fd, send_f, "send", pico::IOManager::WRITE, SO_SNDTIMEO, buf, len, flags);
}

ssize_t sendto(int fd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr,
               socklen_t addrlen) {
    return do_io(fd,
                 sendto_f,
                 "sendto",
                 pico::IOManager::WRITE,
                 SO_SNDTIMEO,
                 buf,
                 len,
                 flags,
                 dest_addr,
                 addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr* msg, int flags) {
    return do_io(sockfd, sendmsg_f, "sendmsg", pico::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd) {
    if (!pico::is_hook_enable()) { return close_f(fd); }

    pico::FdCtx::Ptr ctx = pico::FdMgr::getInstance()->getFdCtx(fd);
    if (ctx) {
        auto iom = pico::IOManager::getThis();
        iom->cancelAllEvent(fd);
        pico::FdMgr::getInstance()->delFdCtx(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ...) {
    va_list va;
    va_start(va, cmd);
    switch (cmd) {
    case F_SETFL:
    {
        int arg = va_arg(va, int);
        va_end(va);
        pico::FdCtx::Ptr ctx = pico::FdMgr::getInstance()->getFdCtx(fd);
        if (!ctx || ctx->isClosed() || !ctx->isSocket()) { return fcntl_f(fd, cmd, arg); }
        ctx->setUserNonBlock(arg & O_NONBLOCK);
        if (ctx->isSysNonBlock()) { arg |= O_NONBLOCK; }
        else {
            arg &= ~O_NONBLOCK;
        }
        return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETFL:
    {
        va_end(va);
        int arg = fcntl_f(fd, cmd);
        pico::FdCtx::Ptr ctx = pico::FdMgr::getInstance()->getFdCtx(fd);
        if (!ctx || ctx->isClosed() || !ctx->isSocket()) { return arg; }
        if (ctx->isUserNonBlock()) { return arg | O_NONBLOCK; }
        else {
            return arg & ~O_NONBLOCK;
        }
    } break;
    case F_DUPFD:
    case F_DUPFD_CLOEXEC:
    case F_SETFD:
    case F_SETOWN:
    case F_SETSIG:
    case F_SETLEASE:
    case F_NOTIFY:
#ifdef F_SETPIPE_SZ
    case F_SETPIPE_SZ:
#endif
    {
        int arg = va_arg(va, int);
        va_end(va);
        return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETFD:
    case F_GETOWN:
    case F_GETSIG:
    case F_GETLEASE:
#ifdef F_GETPIPE_SZ
    case F_GETPIPE_SZ:
#endif
    {
        va_end(va);
        return fcntl_f(fd, cmd);
    } break;
    case F_SETLK:
    case F_SETLKW:
    case F_GETLK:
    {
        struct flock* arg = va_arg(va, struct flock*);
        va_end(va);
        return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETOWN_EX:
    case F_SETOWN_EX:
    {
        struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
        va_end(va);
        return fcntl_f(fd, cmd, arg);
    } break;
    default: va_end(va); return fcntl_f(fd, cmd);
    }
}

int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen) {
    if (!pico::is_hook_enable()) { return setsockopt_f(sockfd, level, optname, optval, optlen); }

    if (level == SOL_SOCKET) {
        if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            pico::FdCtx::Ptr ctx = pico::FdMgr::getInstance()->getFdCtx(sockfd);
            if (ctx) {
                const timeval* tv = (const timeval*)optval;
                ctx->setTimeout(optname, tv->tv_sec * 1000 + tv->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

int getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int ioctl(int fd, unsigned long request, ...) {
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if (request == FIONBIO) {
        bool user_nonblock = *(int*)arg;
        pico::FdCtx::Ptr ctx = pico::FdMgr::getInstance()->getFdCtx(fd);
        if (!ctx || ctx->isClosed() || !ctx->isSocket()) { return ioctl_f(fd, request, arg); }
        if (ctx) { ctx->setUserNonBlock(user_nonblock); }
    }

    return ioctl_f(fd, request, arg);
}

}   // extern "C"