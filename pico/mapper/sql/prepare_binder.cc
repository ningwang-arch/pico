#include "prepare_binder.h"

#include <iostream>

namespace pico {

void PrepareBinder::bindValue(int index, const Object& value) {
    auto type = value.getType();
    auto it = _bind_map.find(type);
    if (it == _bind_map.end()) { throw SQLException("unsupported type"); }
    it->second(index, value);
}

void PrepareBinder::bindInt(int index, const Object& value) {
    _bind_list[index].buffer_type = MYSQL_TYPE_LONG;
    _bind_list[index].buffer = value.getValuePtr();
    _bind_list[index].is_unsigned = false;
}

void PrepareBinder::bindUInt(int index, const Object& value) {
    _bind_list[index].buffer_type = MYSQL_TYPE_LONG;
    _bind_list[index].buffer = value.getValuePtr();
    _bind_list[index].is_unsigned = true;
}

void PrepareBinder::bindTime(int index, const Object& value) {
    _bind_list[index].buffer_type = MYSQL_TYPE_TIMESTAMP;
    _bind_list[index].buffer = value.getValuePtr();
    _bind_list[index].is_unsigned = false;
}

void PrepareBinder::bindString(int index, const Object& value) {
    _bind_list[index].buffer_type = MYSQL_TYPE_STRING;
    _bind_list[index].buffer = value.getValuePtr();
    _bind_list[index].buffer_length = value.getValue<std::string>().size();
}

}   // namespace pico