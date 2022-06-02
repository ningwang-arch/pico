#ifndef __PICO_MAPPER_ENTITY_CRITERION_H__
#define __PICO_MAPPER_ENTITY_CRITERION_H__

#include <string>

#include "constants.h"
#include "object.hpp"

namespace pico {
class Criterion
{
public:
    explicit Criterion(const std::string& cond, bool is_or = false) {
        m_cond = cond;
        is_no_value = true;
        m_and_or = is_or ? Constants::OR : Constants::AND;
    }

    Criterion(const std::string& cond, const Object& obj, const Object& obj2, bool is_or = false) {
        m_cond = cond;
        m_values.emplace_back(obj);
        m_values.emplace_back(obj2);
        is_between_value = true;
        is_no_value = false;
        m_and_or = is_or ? Constants::OR : Constants::AND;
    }

    Criterion(const std::string& cond, const Object& obj, bool is_or = false) {
        m_cond = cond;
        m_and_or = is_or ? Constants::OR : Constants::AND;
        if (obj.isContainer()) {
            is_list_value = true;
            m_values.insert(m_values.end(), obj.getObjects().begin(), obj.getObjects().end());
        }
        else {
            is_single_value = true;
            m_values.emplace_back(obj);
        }
    }

    const std::string& getCondition() const { return m_cond; }
    const std::vector<Object>& getValues() const { return m_values; }
    const std::string& getAndOr() const { return m_and_or; }

    bool isNoValue() const { return is_no_value; }
    bool isSingleValue() const { return is_single_value; }
    bool isListValue() const { return is_list_value; }
    bool isBetweenValue() const { return is_between_value; }

private:
    std::string m_cond;
    std::vector<Object> m_values;

    std::string m_and_or;

    bool is_no_value = false;
    bool is_single_value = false;
    bool is_between_value = false;
    bool is_list_value = false;
};

}   // namespace pico

#endif