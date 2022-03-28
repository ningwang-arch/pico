#include "fdmanager.h"
#include "hook.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>

namespace pico {
FdCtx::FdCtx(int fd)
    : m_fd(fd)
    , m_isInit(false)
    , m_isSocket(false)
    , m_isSysNonBlock(false)
    , m_isClosed(false)
    , m_isUserNonBlock(false)
    , m_recvTimeout(-1)
    , m_sendTimeout(-1) {
    init();
}

FdCtx::~FdCtx() {}

bool FdCtx::init() {
    if (m_isInit) { return true; }
    m_recvTimeout = -1;
    m_sendTimeout = -1;

    struct stat fstat_;
    if (::fstat(m_fd, &fstat_) < 0) {
        m_isInit = false;
        m_isSocket = false;
    }
    else {
        m_isInit = true;
        m_isSocket = S_ISSOCK(fstat_.st_mode);
    }

    if (m_isSocket) {
        int flags = fcntl_f(m_fd, F_GETFL, 0);
        if (!(flags & O_NONBLOCK)) { fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK); }
        m_isSysNonBlock = true;
    }
    else {
        m_isSysNonBlock = false;
    }
    m_isUserNonBlock = false;
    m_isClosed = false;
    return m_isInit;
}

void FdCtx::setTimeout(int type, int timeout) {
    if (type == SO_RCVTIMEO) { m_recvTimeout = timeout; }
    else {
        m_sendTimeout = timeout;
    }
}

uint64_t FdCtx::getTimeout(int type) const {
    if (type == SO_RCVTIMEO) { return m_recvTimeout; }
    else {
        return m_sendTimeout;
    }
}

FdManager::FdManager() {
    m_fdCtxs.resize(64);
}

FdCtx::Ptr FdManager::getFdCtx(int fd, bool autoCreate) {
    if (fd < 0) { return nullptr; }
    MutexType::ReadLock rlock(m_mutex);
    if ((int)m_fdCtxs.size() <= fd) {
        if (!autoCreate) { return nullptr; }
    }
    else {
        if (m_fdCtxs[fd] || !autoCreate) { return m_fdCtxs[fd]; }
    }
    rlock.unlock();
    MutexType::WriteLock wlock(m_mutex);
    FdCtx::Ptr fdCtx(new FdCtx(fd));
    if ((int)m_fdCtxs.size() <= fd) { m_fdCtxs.resize(fd * 1.5); }
    m_fdCtxs[fd] = fdCtx;
    wlock.unlock();
    return nullptr;
}

void FdManager::delFdCtx(int fd) {
    MutexType::WriteLock wlock(m_mutex);
    if ((int)m_fdCtxs.size() < fd) { return; }
    m_fdCtxs[fd].reset();
}

}   // namespace pico