#include "result_binder.h"

#include <iostream>

#include "SQLException.h"

namespace pico {
void ResultBinder::bindValue(int index, enum_field_types type) {
    auto it = _bind_map.find(type);
    if (it == _bind_map.end()) {
        throw SQLException("unsupported type");
    }
    it->second(index);
}

void ResultBinder::bindInt(int index) {
    _result_list[index] = Object(std::type_index(typeid(int)));

    _bind_list[index].buffer_type = MYSQL_TYPE_LONG;
    _bind_list[index].buffer = _result_list[index].getValuePtr();
    _bind_list[index].is_null = (my_bool*)_result_list[index].isNullPtr();
}

void ResultBinder::bindTime(int index) {
    _result_list[index] = Object(std::type_index(typeid(std::time_t)));

    _bind_list[index].buffer_type = MYSQL_TYPE_TIMESTAMP;
    _bind_list[index].buffer = _result_list[index].getValuePtr();
    _bind_list[index].is_null = (my_bool*)_result_list[index].isNullPtr();
}

void ResultBinder::bindString(int index) {
    _result_list[index] = Object(std::type_index(typeid(std::string)));
    _result_list[index].resize(MAX_STRING_LENGTH);

    _bind_list[index].buffer_type = MYSQL_TYPE_STRING;
    _bind_list[index].buffer = _result_list[index].getValuePtr();
    _bind_list[index].buffer_length = MAX_STRING_LENGTH;
    _bind_list[index].is_null = (my_bool*)_result_list[index].isNullPtr();
}

}   // namespace pico
