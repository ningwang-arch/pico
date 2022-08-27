#ifndef __PICO_FDMANAGER_H__
#define __PICO_FDMANAGER_H__

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "mutex.h"
#include "singleton.h"

namespace pico {
class FdCtx : public std::enable_shared_from_this<FdCtx>
{
public:
    typedef std::shared_ptr<FdCtx> Ptr;
    explicit FdCtx(int fd);
    virtual ~FdCtx();
    bool init();
    bool isInit() const { return m_isInit; }
    bool isSocket() const { return m_isSocket; }
    bool isClosed() const { return m_isClosed; }
    bool close();

    void setUserNonBlock(bool nonBlock) { m_isUserNonBlock = nonBlock; }
    bool isUserNonBlock() const { return m_isUserNonBlock; }

    void setSysNonBlock(bool nonBlock) { m_isSysNonBlock = nonBlock; }
    bool isSysNonBlock() const { return m_isSysNonBlock; }

    void setTimeout(int type, int timeout);
    uint64_t getTimeout(int type) const;

private:
    int m_fd;
    bool m_isInit : 1;
    bool m_isSocket : 1;
    bool m_isSysNonBlock : 1;
    bool m_isClosed : 1;
    bool m_isUserNonBlock : 1;

    uint64_t m_recvTimeout = -1;
    uint64_t m_sendTimeout = -1;
};

class FdManager
{
public:
    typedef std::shared_ptr<FdManager> Ptr;
    typedef RWMutex MutexType;

    FdManager();
    FdCtx::Ptr getFdCtx(int fd, bool autoCreate = false);
    void delFdCtx(int fd);

private:
    MutexType m_mutex;
    std::vector<FdCtx::Ptr> m_fdCtxs;
};

typedef Singleton<FdManager> FdMgr;


}   // namespace pico

#endif