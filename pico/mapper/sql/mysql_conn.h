#ifndef __PICO_MAPPER_SQL_MYSQL_CONN_H__
#define __PICO_MAPPER_SQL_MYSQL_CONN_H__

#include <mysql/mysql.h>

#include "../../class_factory.h"
#include "connection.h"
#include "prepare_binder.h"
#include "result_binder.h"
#include "sql_option.h"


namespace pico {

class MySQLConnection : public Connection
{
public:
    virtual void close() override;
    virtual bool ping() override;

    virtual int getLastAffectedRows() override;

    virtual int getLastInsertId() override;

    virtual bool connect() override;

    virtual void bindValue(int index, const Object& value) override {
        _prepare_binder->bindValue(index, value);
    }

    virtual Object value(int index) override { return _result_binder->value(index); }

    virtual bool begin() override;
    virtual bool commit() override;
    virtual bool rollback() override;

    virtual bool execute() override;

    virtual bool prepare(const std::string& sql) override;

    virtual bool next() override;

    ~MySQLConnection();

protected:
    virtual void clear() override;

private:
    MYSQL* _conn = nullptr;
    MYSQL_STMT* _stmt = nullptr;
    MYSQL_RES* _result = nullptr;

    std::shared_ptr<PrepareBinder> _prepare_binder;
    std::shared_ptr<ResultBinder> _result_binder;
};

REGISTER_CLASS(MySQLConnection);

}   // namespace pico

#endif