#ifndef __PICO_MAPPER_BUILDER_SQL_BUILDER_H__
#define __PICO_MAPPER_BUILDER_SQL_BUILDER_H__

#include <memory>
#include <string>
#include <vector>

#include "sql_stmt.h"

namespace pico {

class SQLBuilder
{
public:
    SQLBuilder* UPDATE(const std::string& table);
    SQLBuilder* SET(const std::string& sets);
    SQLBuilder* INSERT_INTO(const std::string& table);
    SQLBuilder* VALUES(const std::string& columns, const std::string& values);
    SQLBuilder* INTO_COLUMNS(const std::string& columns);
    SQLBuilder* INTO_VALUES(const std::string& values);
    SQLBuilder* SELECT(const std::string& columns);
    SQLBuilder* SELECT_DISTINCT(const std::string& columns);
    SQLBuilder* FROM(const std::string& table);
    SQLBuilder* JOIN(const std::string& table);
    SQLBuilder* INNER_JOIN(const std::string& table);
    SQLBuilder* OUTER_JOIN(const std::string& table);
    SQLBuilder* LEFT_OUTER_JOIN(const std::string& table);
    SQLBuilder* RIGHT_OUTER_JOIN(const std::string& table);
    SQLBuilder* WHERE(const std::string& where);
    SQLBuilder* HAVING(const std::string& having);
    SQLBuilder* GROUP_BY(const std::string& group_by);
    SQLBuilder* ORDER_BY(const std::string& order_by);
    SQLBuilder* LIMIT(const std::string& variable);
    SQLBuilder* LIMIT(const std::string& variable, const std::string& offset);
    SQLBuilder* DELETE_FROM(const std::string& table);
    SQLBuilder* AND();
    SQLBuilder* OR();
    SQLBuilder* ADD_ROW();

    std::string to_string() { return _stmt.sql(); }

private:
    SQLStmt _stmt;
};


}   // namespace pico
#endif