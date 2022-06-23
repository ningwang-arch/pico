#ifndef __PICO_MAPPER_SQL_SQLITE_CONN_H__
#define __PICO_MAPPER_SQL_SQLITE_CONN_H__

#include <sqlite3.h>

#include "../../class_factory.h"
#include "connection.h"
#include "prepare_binder.h"
#include "result_binder.h"
#include "sql_option.h"


/**
 * Warning: Because SQLite3 does not have an official datetime type,
 *         so you should not use std::time_t to store datetime.
 *        Instead, you should use std::string.
 */

namespace pico {

class SQLiteConnection : public Connection
{
public:
    virtual void close() override;
    virtual bool ping() override;

    virtual int getLastAffectedRows() override;

    virtual int getLastInsertId() override;

    virtual bool connect() override;


    virtual bool begin() override;
    virtual bool commit() override;
    virtual bool rollback() override;

    virtual bool execute() override;

    virtual bool prepare(const std::string& sql) override;

    virtual bool next() override;

    virtual void bindValue(int index, const Object& value) override;

    virtual Object value(int index) override { return _values[_current_row_index][index]; }


    ~SQLiteConnection();

protected:
    virtual void clear() override {}

private:
    void bindInt(int index, const Object& value);
    void bindUInt(int index, const Object& value);
    void bindTime(int index, const Object& value);
    void bindString(int index, const Object& value);

private:
    sqlite3_stmt* _stmt = nullptr;
    sqlite3* _conn = nullptr;


    std::vector<std::vector<Object>> _values;

    std::atomic<int> _current_row_index{-1};

    std::map<std::type_index, std::function<void(int, const Object&)>> _bind_map = {
        {typeid(int),
         std::bind(&SQLiteConnection::bindInt, this, std::placeholders::_1, std::placeholders::_2)},
        {typeid(std::time_t),
         std::bind(&SQLiteConnection::bindTime, this, std::placeholders::_1,
                   std::placeholders::_2)},
        {typeid(std::string),
         std::bind(&SQLiteConnection::bindString, this, std::placeholders::_1,
                   std::placeholders::_2)},
    };
};

REGISTER_CLASS(SQLiteConnection);

}   // namespace pico

#endif