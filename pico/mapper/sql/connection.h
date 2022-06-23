#ifndef __PICO_MAPPER_SQL_CONNECTION_H__
#define __PICO_MAPPER_SQL_CONNECTION_H__

#include <chrono>
#include <memory>

#include <string>
#include <vector>

#include "prepare_binder.h"
#include "result_binder.h"
#include "sql_option.h"

namespace pico {

class Connection
{
public:
    virtual void setOption(const SQLOption& option) { _option = option; }

    virtual void close() = 0;

    virtual bool ping() = 0;

    virtual int getLastAffectedRows() = 0;

    virtual int getLastInsertId() = 0;

    virtual bool connect() = 0;


    virtual bool begin() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;

    virtual bool execute() = 0;

    virtual void bindValue(int index, const Object& value) = 0;

    virtual bool prepare(const std::string& sql) = 0;

    virtual bool next() = 0;

    virtual Object value(int index) = 0;

    virtual const std::vector<std::string>& getRecords() { return _records; }

    virtual ~Connection() = default;

protected:
    virtual void clear() = 0;

protected:
    // store column names
    std::vector<std::string> _records;

    SQLOption _option;
};

}   // namespace pico

#endif