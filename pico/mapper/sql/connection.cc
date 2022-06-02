#include "connection.h"

#include "pico/logging.h"

namespace pico {

namespace {
struct MySQLThreadIniter
{
    MySQLThreadIniter() { ::mysql_thread_init(); }
    ~MySQLThreadIniter() { ::mysql_thread_end(); }
};
}   // namespace

void Connection::clear() {
    if (!_result) { ::mysql_free_result(_result); }
    if (!_stmt) { ::mysql_stmt_free_result(_stmt); }

    _records.clear();

    _prepare_binder.reset();
    _result_binder.reset();
}

void Connection::close() {
    if (_conn) { ::mysql_close(_conn); }
    _conn = nullptr;
}

bool Connection::ping() {
    return ::mysql_ping(_conn) == 0;
}

int Connection::getLastAffectedRows() {
    return (int)::mysql_affected_rows(_conn);
}

int Connection::getLastInsertId() {
    return (int)::mysql_insert_id(_conn);
}

bool Connection::connect() {
    static thread_local MySQLThreadIniter initer;

    _conn = ::mysql_init(nullptr);
    if (!_conn) {
        LOG_ERROR("mysql_init failed");
        return false;
    }


    if (_option.connect_timeout > 0) {
        if (::mysql_options(_conn, MYSQL_OPT_CONNECT_TIMEOUT, &_option.connect_timeout) != 0) {
            LOG_ERROR("mysql_options failed");
            return false;
        }
    }

    ::mysql_options(_conn, MYSQL_OPT_RECONNECT, &_option.reconnect_time);
    ::mysql_options(_conn, MYSQL_SET_CHARSET_NAME, _option.charset.c_str());

    if (::mysql_real_connect(_conn,
                             _option.host.c_str(),
                             _option.user.c_str(),
                             _option.password.c_str(),
                             _option.database.c_str(),
                             _option.port,
                             nullptr,
                             0) == nullptr) {
        LOG_ERROR("mysql_real_connect failed: %s", mysql_error(_conn));
        return false;
    }
    return true;
}

bool Connection::begin() {
    if (::mysql_real_query(_conn, "BEGIN", 5) != 0) {
        LOG_ERROR("mysql_real_query failed: %s", mysql_error(_conn));
        return false;
    }
    return true;
}

bool Connection::commit() {
    if (::mysql_real_query(_conn, "COMMIT", 6) != 0) {
        LOG_ERROR("mysql_real_query failed: %s", mysql_error(_conn));
        return false;
    }
    return true;
}

bool Connection::rollback() {
    if (::mysql_real_query(_conn, "ROLLBACK", 8) != 0) {
        LOG_ERROR("mysql_real_query failed: %s", mysql_error(_conn));
        return false;
    }
    return true;
}

bool Connection::execute() {
    if (::mysql_stmt_bind_param(_stmt, &_prepare_binder->getBindBuffer()[0]) != 0) {
        LOG_ERROR("mysql_stmt_bind_param failed: %s", mysql_stmt_error(_stmt));
        return false;
    }

    _result = ::mysql_stmt_result_metadata(_stmt);
    if (_result) {
        auto column_count = ::mysql_num_fields(_result);
        _result_binder = std::make_shared<ResultBinder>(column_count);
        for (int i = 0; i < (int)column_count; ++i) {
            _result_binder->bindValue(i, _result->fields[i].type);
            _records.emplace_back(_result->fields[i].name);
        }

        if (::mysql_stmt_bind_result(_stmt, &_result_binder->getBindResult()[0]) != 0) {
            LOG_ERROR("mysql_stmt_bind_result failed: %s", mysql_stmt_error(_stmt));
            return false;
        }
    }

    if (::mysql_stmt_execute(_stmt) != 0) {
        LOG_ERROR("mysql_stmt_execute failed: %s", mysql_stmt_error(_stmt));
        return false;
    }

    return true;
}

bool Connection::prepare(const std::string& sql) {
    _stmt = ::mysql_stmt_init(_conn);
    if (!_stmt) {
        LOG_ERROR("mysql_stmt_init failed: %s", mysql_error(_conn));
        return false;
    }
    if (::mysql_stmt_prepare(_stmt, sql.c_str(), sql.size()) != 0) {
        LOG_ERROR("mysql_stmt_prepare failed: %s", mysql_stmt_error(_stmt));
        return false;
    }
    _prepare_binder = std::make_shared<PrepareBinder>(::mysql_stmt_param_count(_stmt));
    return true;
}

}   // namespace pico
