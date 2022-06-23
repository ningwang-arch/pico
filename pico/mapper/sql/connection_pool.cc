#include "connection_pool.h"

#include "../../logging.h"
#include "mysql_conn.h"
#include "sqlite_conn.h"

namespace pico {
static ConfigVar<std::unordered_map<std::string, SQLOption>>::Ptr g_sql_options =
    Config::Lookup<std::unordered_map<std::string, SQLOption>>(
        "dbs", std::unordered_map<std::string, SQLOption>(), "sql_options");


void ConnectionPool::initInternal() {
    std::call_once(_once_flag, [this]() {
        _open_conn_num = 0;
        for (int i = 0; i < _min_conn_num; ++i) {
            auto conn = createConnection();
            if (conn) _conns.push(conn);
        }
    });
}

Connection* ConnectionPool::createConnection() {
    std::string type = _option.type;

    Connection* conn = nullptr;

    if (type == "mysql") { conn = new MySQLConnection(); }
    if (type == "sqlite") { conn = new SQLiteConnection(); }

    if (conn) {
        conn->setOption(_option);
        conn->connect();
        ++_open_conn_num;
    }
    return conn;
}

void ConnectionPool::releaseConnection(Connection* conn) {
    if (conn) {
        std::unique_lock<std::mutex> lock(_mutex);
        _conns.push(conn);
        _cond.notify_one();
    }
}

std::shared_ptr<Connection> ConnectionPool::popConnection() {
    if (_conns.empty()) { return nullptr; }
    auto conn = _conns.front();
    _conns.pop();
    return std::shared_ptr<Connection>(conn, [this](Connection* conn) { releaseConnection(conn); });
}

std::shared_ptr<Connection> ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(_mutex);

    std::shared_ptr<Connection> conn = nullptr;

    while (!_conns.empty() && !_conns.front()->ping()) {
        _conns.front()->close();
        delete _conns.front();
        _conns.pop();
        _open_conn_num--;
    }
    if (!_conns.empty()) { return popConnection(); }

    if (_open_conn_num < _max_conn_num) {
        return std::shared_ptr<Connection>(createConnection(),
                                           [this](Connection* conn) { releaseConnection(conn); });
    }

    if (!_cond.wait_for(
            lock, std::chrono::seconds(_idle_timeout), [this]() { return !_conns.empty(); })) {
        LOG_ERROR("wait for connection timeout");
        return conn;
    }

    return popConnection();
}
ConnectionManager::ConnectionManager() {
    auto options = g_sql_options->getValue();
    for (auto& option : options) {
        auto pool = std::make_shared<ConnectionPool>();
        pool->setOption(option.second);
        pool->initInternal();
        _pools[option.first] = pool;
    }
}

std::shared_ptr<Connection> ConnectionManager::getConnection(const std::string& name) {
    auto it = _pools.find(name);
    if (it == _pools.end()) { return nullptr; }
    return it->second->getConnection();
}

}   // namespace pico