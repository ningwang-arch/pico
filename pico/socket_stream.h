#ifndef __PICO_SOCKET_STREAM_H__
#define __PICO_SOCKET_STREAM_H__

#include <memory>
#include <string>
#include <vector>

#include "socket.h"

namespace pico {
class SocketStream
{
public:
    typedef std::shared_ptr<SocketStream> Ptr;
    explicit SocketStream(Socket::Ptr sock, bool owner = true);
    virtual ~SocketStream();

    virtual int read(void* buf, size_t len);
    virtual int write(const void* buf, size_t len);
    virtual int readFixSize(void* buf, size_t length);
    virtual int writeFixSize(const void* buf, size_t length);

    virtual void close();

    bool isConnected() const;

    Address::Ptr getPeerAddress();
    Address::Ptr getLocalAddress();

    Socket::Ptr getSocket() const { return m_sock; }

private:
    Socket::Ptr m_sock;
    bool m_owner;
};

}   // namespace pico

#endif
