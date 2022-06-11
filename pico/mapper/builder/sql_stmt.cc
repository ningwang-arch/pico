#include "sql_stmt.h"

namespace pico {

void SQLStmt::sqlClause(std::string& builder, const std::string& keyword,
                        const std::vector<std::string>& parts, const std::string& open,
                        const std::string& close, const std::string& conjunction) {
    if (parts.empty()) { return; }
    if (!builder.empty()) { builder += " "; }
    builder.append(keyword);
    builder.append(" ");
    builder.append(open);
    std::string last = "";
    for (int i = 0, n = parts.size(); i < n; i++) {
        std::string part = parts.at(i);
        if (i > 0 && part != AND && part != OR && last != AND && last != OR) {
            builder.append(conjunction);
        }
        builder.append(part);
        last = part;
    }
    builder.append(close);
}

void SQLStmt::deleteClause(std::string& builder, const std::vector<std::string>& parts,
                           const std::string& open, const std::string& close,
                           const std::string& conjunction) {
    if (parts.empty()) { return; }
    if (!builder.empty()) { builder += " "; }
    builder.append("DELETE");
    builder.append(" ");
    std::string last = "";

    for (int i = 0, n = parts.size(); i < n; i++) {
        std::string part = parts.at(i);
        // table AS alias
        int pos = part.find("AS");
        if (pos > 0) { part = part.substr(pos + 3); }
        builder.append(part);
        if (i == (int)parts.size() - 1)
            builder.append(" ");
        else
            builder.append(", ");
    }
    builder.append("FROM ");
    builder.append(open);
    for (int i = 0, n = parts.size(); i < n; i++) {
        std::string part = parts.at(i);
        if (i > 0 && part != AND && part != OR && last != AND && last != OR) {
            builder.append(conjunction);
        }
        builder.append(part);
        last = part;
    }
    builder.append(close);
}

void SQLStmt::deleteSQL(std::string& builder) {
    // t_school as s
    // delete s from t_school as s
    deleteClause(builder, _tables, "", "", "");
    sqlClause(builder, "WHERE", _where, "(", ")", " AND ");
    appendLimitClause(builder, _limit, "");
}

void SQLStmt::updateSQL(std::string& builder) {
    sqlClause(builder, "UPDATE", _tables, "", "", ",");
    joins(builder);
    sqlClause(builder, "SET", _sets, "", "", ",");
    sqlClause(builder, "WHERE", _where, "(", ")", " AND ");
    appendLimitClause(builder, _limit, "");
}

void SQLStmt::insertSQL(std::string& builder) {
    sqlClause(builder, "INSERT INTO", _tables, "", "", "");
    sqlClause(builder, "", _columns, "(", ")", ", ");
    for (int i = 0; i < (int)_valuesList.size(); i++) {
        sqlClause(builder, i > 0 ? "," : "VALUES", _valuesList.at(i), "(", ")", ", ");
    }
}

void SQLStmt::joins(std::string& builder) {
    sqlClause(builder, "JOIN", _join, "", "", " JOIN ");
    sqlClause(builder, "INNER JOIN", _innerJoin, "", "", " INNER JOIN ");
    sqlClause(builder, "OUTER JOIN", _outerJoin, "", "", " OUTER JOIN ");
    sqlClause(builder, "LEFT OUTER JOIN", _leftOuterJoin, "", "", " LEFT OUTER JOIN ");
    sqlClause(builder, "RIGHT OUTER JOIN", _rightOuterJoin, "", "", " RIGHT OUTER JOIN ");
}

void SQLStmt::appendLimitClause(std::string& builder, std::string limit, std::string offset) {
    if (!limit.empty()) {
        builder.append(" LIMIT ");
        builder.append(limit);
    }
    if (!offset.empty()) {
        builder.append(" OFFSET ");
        builder.append(offset);
    }
}

void SQLStmt::selectSQL(std::string& builder) {
    if (_distinct) { sqlClause(builder, "SELECT DISTINCT", _select, "", "", ", "); }
    else {
        sqlClause(builder, "SELECT", _select, "", "", ", ");
    }
    // https://www.cnblogs.com/wang3680/p/4461276.html
    sqlClause(builder, "FROM", _tables, "(", ")", ",");
    joins(builder);
    sqlClause(builder, "WHERE", _where, "(", ")", " AND ");
    sqlClause(builder, "GROUP BY", _groupBy, "", "", ", ");
    sqlClause(builder, "HAVING", _having, "(", ")", " AND ");
    sqlClause(builder, "ORDER BY", _orderBy, "", "", ", ");
    appendLimitClause(builder, _limit, _offset);
}

std::string SQLStmt::sql() {
    std::string builder = "";
    switch (_type) {
    case StmtType::SELECT: selectSQL(builder); break;
    case StmtType::INSERT: insertSQL(builder); break;
    case StmtType::UPDATE: updateSQL(builder); break;
    case StmtType::DELETE: deleteSQL(builder); break;
    default: break;
    }
    return builder;
}

}   // namespace pico
