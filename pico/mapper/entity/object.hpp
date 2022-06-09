#ifndef __PICO_MAPPER_ENTITY_OBJECT_H__
#define __PICO_MAPPER_ENTITY_OBJECT_H__

#include <ctime>
#include <exception>
#include <iostream>
#include <mysql/mysql.h>
#include <stdexcept>
#include <string.h>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>

#include "../../util.h"

namespace pico {

class Object
{
public:
    Object(const std::type_index& type)
        : _type(type) {}
    Object(const std::string& value)
        : _type(std::type_index(typeid(std::string))) {
        m_buff.str_value.resize(value.size());
        std::copy(value.begin(), value.end(), m_buff.str_value.begin());
        m_buff.str_value.push_back('\0');
        _is_null = false;
    }

    Object(const char* value)
        : _type(std::type_index(typeid(std::string))) {
        m_buff.str_value.resize(strlen(value));
        std::copy(value, value + strlen(value), m_buff.str_value.begin());
        m_buff.str_value.push_back('\0');
        _is_null = false;
    }

    Object(std::time_t value)
        : _type(std::type_index(typeid(std::time_t))) {
        m_buff.time_value = time_t2mysql_time(value);
        _is_null = false;
    }

    Object(int value)
        : _type(std::type_index(typeid(int))) {
        m_buff.int_value = value;
        _is_null = false;
    }

    Object() = default;

protected:
    Object(std::type_index type, bool is_container, bool is_null)
        : _type(type)
        , _is_container(is_container)
        , _is_null(is_null) {}

public:
    bool isNull() const { return _type == typeid(void) || _is_null; }

    void clear() {
        _type = std::type_index(typeid(void));
        _is_container = false;
        _is_null = true;

        m_buff.str_value.clear();
        m_buff.int_value = 0;
        m_buff.time_value.time_type = MYSQL_TIMESTAMP_DATETIME;
        m_buff.object_value.clear();
        m_buff.str_value = {'\0'};
    }

    // int
    template<typename T>
    typename std::enable_if<std::is_same<int, T>::value, int>::type getValue() const {
        if (!isNull()) { return m_buff.int_value; }
        return 0;
    }

    // std::string
    template<typename T>
    typename std::enable_if<std::is_same<std::string, T>::value, std::string>::type
    getValue() const {
        if (!isNull()) { return m_buff.str_value.data(); }
        return std::string{};
    }

    // std::time_t
    template<typename T>
    typename std::enable_if<std::is_same<std::time_t, T>::value, std::time_t>::type
    getValue() const {
        if (!isNull()) { return mysql_time2time_t(m_buff.time_value); }
        return 0;
    }

    void* getValuePtr() const {
        if (_type == typeid(int)) { return (void*)&m_buff.int_value; }
        if (_type == typeid(std::string)) { return (void*)m_buff.str_value.data(); }
        if (_type == typeid(std::time_t)) { return (void*)&m_buff.time_value; }
        return nullptr;
    }

    bool* isNullPtr() const { return (bool*)&_is_null; }

    const std::type_index& getType() const { return _type; }

    bool isContainer() const { return _is_container; }

    const std::vector<Object>& getObjects() const { return m_buff.object_value; }

    void resize(int size) { m_buff.str_value.resize(size); }


protected:
    struct buffer
    {
        std::vector<char> str_value;
        int int_value;
        MYSQL_TIME time_value = {0};
        std::vector<Object> object_value;
    } m_buff;

    std::type_index _type = std::type_index(typeid(void));
    bool _is_container = false;
    bool _is_null = true;
};

}   // namespace pico

#endif
