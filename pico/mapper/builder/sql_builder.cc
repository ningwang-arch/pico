#include "sql_builder.h"

#include <iostream>

namespace pico {
SQLBuilder* SQLBuilder::UPDATE(const std::string& table) {
    _stmt._type = SQLStmt::StmtType::UPDATE;
    _stmt._tables.push_back(table);
    return this;
}
SQLBuilder* SQLBuilder::SET(const std::string& sets) {
    _stmt._sets.push_back(sets);
    return this;
}
SQLBuilder* SQLBuilder::INSERT_INTO(const std::string& table) {
    _stmt._type = SQLStmt::StmtType::INSERT;
    _stmt._tables.push_back(table);
    return this;
}
SQLBuilder* SQLBuilder::VALUES(const std::string& columns, const std::string& values) {
    INTO_COLUMNS(columns);
    INTO_VALUES(values);
    return this;
}
SQLBuilder* SQLBuilder::INTO_COLUMNS(const std::string& columns) {
    _stmt._columns.push_back(columns);
    return this;
}
SQLBuilder* SQLBuilder::INTO_VALUES(const std::string& values) {
    _stmt._valuesList.back().push_back(values);
    return this;
}
SQLBuilder* SQLBuilder::SELECT(const std::string& columns) {
    _stmt._type = SQLStmt::StmtType::SELECT;
    _stmt._select.push_back(columns);
    return this;
}
SQLBuilder* SQLBuilder::SELECT_DISTINCT(const std::string& columns) {
    _stmt._distinct = true;
    return SELECT(columns);
}
SQLBuilder* SQLBuilder::FROM(const std::string& table) {
    _stmt._tables.push_back(table);
    return this;
}
SQLBuilder* SQLBuilder::JOIN(const std::string& table) {
    _stmt._join.push_back(table);
    return this;
}
SQLBuilder* SQLBuilder::INNER_JOIN(const std::string& table) {
    _stmt._innerJoin.push_back(table);
    return this;
}
SQLBuilder* SQLBuilder::OUTER_JOIN(const std::string& table) {
    _stmt._outerJoin.push_back(table);
    return this;
}
SQLBuilder* SQLBuilder::LEFT_OUTER_JOIN(const std::string& table) {
    _stmt._leftOuterJoin.push_back(table);
    return this;
}
SQLBuilder* SQLBuilder::RIGHT_OUTER_JOIN(const std::string& table) {
    _stmt._rightOuterJoin.push_back(table);
    return this;
}
SQLBuilder* SQLBuilder::WHERE(const std::string& where) {
    _stmt._where.push_back(where);
    _stmt._lastList = &_stmt._where;
    return this;
}
SQLBuilder* SQLBuilder::HAVING(const std::string& having) {
    _stmt._having.push_back(having);
    _stmt._lastList = &_stmt._having;
    return this;
}
SQLBuilder* SQLBuilder::GROUP_BY(const std::string& group_by) {
    _stmt._groupBy.push_back(group_by);
    return this;
}
SQLBuilder* SQLBuilder::ORDER_BY(const std::string& order_by) {
    _stmt._orderBy.push_back(order_by);
    return this;
}
SQLBuilder* SQLBuilder::LIMIT(const std::string& variable) {
    _stmt._limit = variable;
    _stmt._offset = std::to_string(0);
    return this;
}
SQLBuilder* SQLBuilder::LIMIT(const std::string& variable, const std::string& offset) {
    _stmt._limit = variable;
    _stmt._offset = offset;
    return this;
}
SQLBuilder* SQLBuilder::DELETE_FROM(const std::string& table) {
    _stmt._type = SQLStmt::StmtType::DELETE;
    _stmt._tables.push_back(table);
    return this;
}
SQLBuilder* SQLBuilder::AND() {
    _stmt._lastList->push_back(_stmt.AND);
    return this;
}
SQLBuilder* SQLBuilder::OR() {
    _stmt._lastList->push_back(_stmt.OR);
    return this;
}
SQLBuilder* SQLBuilder::ADD_ROW() {
    _stmt._valuesList.resize(_stmt._valuesList.size() + 1);
    return this;
}

}   // namespace pico