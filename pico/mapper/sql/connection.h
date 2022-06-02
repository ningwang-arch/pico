#ifndef __PICO_MAPPER_SQL_CONNECTION_H__
#define __PICO_MAPPER_SQL_CONNECTION_H__

#include <chrono>
#include <memory>
#include <mysql/mysql.h>
#include <string>
#include <vector>

#include "prepare_binder.h"
#include "result_binder.h"
#include "sql_option.h"

namespace pico {

class Connection
{
public:
    void setOption(const SQLOption& option) { _option = option; }

    void close();

    bool ping();

    int getLastAffectedRows();

    int getLastInsertId();

    bool connect();


    bool begin();
    bool commit();
    bool rollback();

    bool execute();

    void bindValue(int index, const Object& value) { _prepare_binder->bindValue(index, value); }

    bool prepare(const std::string& sql);

    bool next() {
        if (mysql_stmt_fetch(_stmt) != 0) {
            clear();
            return false;
        }
        return true;
    }

    Object value(int index) { return _result_binder->value(index); }

    const std::vector<std::string>& getRecords() { return _records; }

    ~Connection() {
        if (_stmt) { mysql_stmt_close(_stmt); }
        if (_conn) { mysql_close(_conn); }
        _stmt = nullptr;
        _conn = nullptr;
    }

private:
    void clear();

private:
    MYSQL* _conn = nullptr;
    MYSQL_STMT* _stmt = nullptr;
    MYSQL_RES* _result = nullptr;

    std::vector<std::string> _records;

    SQLOption _option;

    std::shared_ptr<PrepareBinder> _prepare_binder;
    std::shared_ptr<ResultBinder> _result_binder;
};

}   // namespace pico

#endif