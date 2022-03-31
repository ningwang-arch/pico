#ifndef __PICO_TCP_SERVER_H__
#define __PICO_TCP_SERVER_H__

#include "address.h"
#include "iomanager.h"
#include "socket.h"
#include <memory>
#include <string>
#include <vector>

namespace pico {
class TcpServer : Noncopyable, public std::enable_shared_from_this<TcpServer>
{
public:
    typedef std::shared_ptr<TcpServer> Ptr;

    TcpServer(IOManager* worker = IOManager::getThis(), IOManager* acceptor = IOManager::getThis());


    virtual ~TcpServer();

    virtual bool bind(Address::Ptr& addr);
    virtual bool bind(std::vector<Address::Ptr>& addrs, std::vector<Address::Ptr>& fails);

    virtual bool start();

    virtual void stop();

    bool isStop() const { return m_is_stop; }

    virtual uint64_t getRecvTimeout() const { return m_recvTimeout; }
    virtual void setRecvTimeout(uint64_t& timeout) { m_recvTimeout = timeout; }

    virtual std::string getName() const { return m_name; }
    virtual void setName(const std::string& name) { m_name = name; }

    virtual std::string to_string();

protected:
    void startAccept(Socket::Ptr& sock);

    void handleClient(Socket::Ptr& sock);


private:
    std::vector<Socket::Ptr> m_sockets;

    std::string m_name;
    IOManager* m_worker;
    IOManager* m_acceptor;

    uint64_t m_recvTimeout;

    bool m_is_stop;
};

}   // namespace pico

#endif