#ifndef __PICO_MAPPER_BUILDER_SQL_STMT_H__
#define __PICO_MAPPER_BUILDER_SQL_STMT_H__

#include <memory>
#include <string>
#include <vector>

namespace pico {
class SQLStmt
{
    friend class SQLBuilder;

public:
    SQLStmt() { _valuesList.resize(1); }

private:
    void sqlClause(std::string& builder, const std::string& keyword,
                   const std::vector<std::string>& parts, const std::string& open,
                   const std::string& close, const std::string& conjunction);

    void deleteSQL(std::string& builder);
    void insertSQL(std::string& builder);
    void selectSQL(std::string& builder);
    void updateSQL(std::string& builder);
    void joins(std::string& builder);

    void deleteClause(std::string& builder, const std::vector<std::string>& parts,
                      const std::string& open, const std::string& close,
                      const std::string& conjunction);

    void appendLimitClause(std::string& builder, std::string limit, std::string offset);

private:
    std::string sql();

private:
    const std::string AND = ") AND (";
    const std::string OR = ") OR (";

    enum class StmtType
    {
        SELECT,
        INSERT,
        UPDATE,
        DELETE,
        CREATE,
        DROP,
        ALTER,
        TRUNCATE,
        EXECUTE,
        UNKNOWN
    } _type;

    std::vector<std::string> _sets = {};
    std::vector<std::string> _select = {};
    std::vector<std::string> _tables = {};
    std::vector<std::string> _join = {};
    std::vector<std::string> _innerJoin = {};
    std::vector<std::string> _outerJoin = {};
    std::vector<std::string> _leftOuterJoin = {};
    std::vector<std::string> _rightOuterJoin = {};
    std::vector<std::string> _where = {};
    std::vector<std::string> _having = {};
    std::vector<std::string> _groupBy = {};
    std::vector<std::string> _orderBy = {};
    std::vector<std::string>* _lastList = nullptr;
    std::vector<std::string> _columns = {};
    std::vector<std::vector<std::string>> _valuesList = {};

    bool _distinct = false;
    std::string _offset;
    std::string _limit;
};

}   // namespace pico

#endif