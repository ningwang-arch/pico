#ifndef __PICO_MAPPER_SQL_PREPARE_BINDER_H__
#define __PICO_MAPPER_SQL_PREPARE_BINDER_H__

#include <algorithm>
#include <functional>
#include <map>
#include <mysql/mysql.h>
#include <string>
#include <vector>

#include "../entity/object.hpp"
#include "SQLException.h"

namespace pico {
class PrepareBinder
{
public:
    explicit PrepareBinder(int size) { _bind_list.resize(size); }

    std::vector<MYSQL_BIND>& getBindBuffer() { return _bind_list; }

    void bindValue(int index, const Object& value);

private:
    void bindInt(int index, const Object& value);
    void bindUInt(int index, const Object& value);
    void bindTime(int index, const Object& value);
    void bindString(int index, const Object& value);


private:
    std::vector<MYSQL_BIND> _bind_list;

    std::map<std::type_index, std::function<void(int, const Object&)>> _bind_map = {
        {typeid(int),
         std::bind(&PrepareBinder::bindInt, this, std::placeholders::_1, std::placeholders::_2)},
        {typeid(std::time_t),
         std::bind(&PrepareBinder::bindTime, this, std::placeholders::_1, std::placeholders::_2)},
        {typeid(std::string),
         std::bind(&PrepareBinder::bindString, this, std::placeholders::_1, std::placeholders::_2)},
    };
};

}   // namespace pico

#endif