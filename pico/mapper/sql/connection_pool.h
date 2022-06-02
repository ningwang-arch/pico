#ifndef __PICO_MAPPER_SQL_CONNECTION_POOL_H__
#define __PICO_MAPPER_SQL_CONNECTION_POOL_H__

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

#include "../../singleton.h"
#include "connection.h"
#include "sql_option.h"

namespace pico {
class ConnectionPool
{
public:
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    ConnectionPool() = default;


    std::shared_ptr<Connection> getConnection();

    void setOption(const SQLOption& option) { _option = option; }
    SQLOption getOption() const { return _option; }

    void initInternal();

    ~ConnectionPool() {
        while (!_conns.empty()) {
            Connection* conn = _conns.front();
            _conns.pop();
            delete conn;
        }
        _open_conn_num = 0;
    }

private:
    Connection* createConnection();
    void releaseConnection(Connection* conn);

    std::shared_ptr<Connection> popConnection();

private:
    std::mutex _mutex;
    std::condition_variable _cond;
    std::queue<Connection*> _conns;

    int _min_conn_num = 1;
    int _max_conn_num = 20;
    int _idle_timeout = 60;

    int _open_conn_num;

    std::once_flag _once_flag;

    SQLOption _option;
};

class ConnectionManager : public Singleton<ConnectionManager>
{
public:
    ConnectionManager();

    std::shared_ptr<Connection> getConnection(const std::string& name);

    ~ConnectionManager();

private:
    std::unordered_map<std::string, std::shared_ptr<ConnectionPool>> _pools;
};

}   // namespace pico

#endif
