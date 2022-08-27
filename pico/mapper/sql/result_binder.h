#ifndef __PICO_MAPPER_SQL_RESULT_BINDER_H__
#define __PICO_MAPPER_SQL_RESULT_BINDER_H__

#include <deque>
#include <functional>
#include <mysql/mysql.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "../entity/object.hpp"

namespace pico {
class ResultBinder
{
public:
    explicit ResultBinder(int size) {
        _bind_list.resize(size);
        _result_list.resize(size);
    }

    void bindValue(int index, enum_field_types type);

    Object value(int index) { return _result_list[index]; }

    std::vector<MYSQL_BIND> getBindResult() { return _bind_list; }

private:
    void bindInt(int index);
    void bindTime(int index);
    void bindString(int index);

private:
    const static int MAX_STRING_LENGTH = 65535;
    std::vector<MYSQL_BIND> _bind_list;
    std::vector<Object> _result_list;

    std::unordered_map<int, std::function<void(int)>> _bind_map = {
        {MYSQL_TYPE_LONG, std::bind(&ResultBinder::bindInt, this, std::placeholders::_1)},
        {MYSQL_TYPE_DATETIME, std::bind(&ResultBinder::bindTime, this, std::placeholders::_1)},
        {MYSQL_TYPE_VAR_STRING, std::bind(&ResultBinder::bindString, this, std::placeholders::_1)},
        {MYSQL_TYPE_LONGLONG, std::bind(&ResultBinder::bindInt, this, std::placeholders::_1)},
    };
};

}   // namespace pico

#endif