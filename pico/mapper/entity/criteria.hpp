#ifndef __PICO_MAPPER_ENTITY_CRITERIA_H__
#define __PICO_MAPPER_ENTITY_CRITERIA_H__

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "../helper/entity_helper.hpp"
#include "constants.h"
#include "criterion.h"
#include "entity_column.hpp"
#include "iterable.hpp"

namespace pico {
class Criteria
{
public:
    typedef std::map<std::string, EntityColumn> ColumnMap;

    explicit Criteria(ColumnMap* property_map, EntityTable* table)
        : m_property_map(property_map)
        , m_table(table) {}

    const std::vector<Criterion>& getCriteria() const { return m_criterions; }
    void setAndOr(const std::string& and_or) { m_and_or = and_or; }
    const std::string& getAndOr() const { return m_and_or; }

public:
    /*and*/
    template<typename Property>
    Criteria* andIsNull(const Property& property) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::IS_NULL)));
        return this;
    }

    template<typename Property>
    Criteria* andIsNotNull(const Property& property) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::IS_NOT_NULL)));
        return this;
    }

    template<typename Property>
    Criteria* andEqualTo(const Property& property, const Object& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::EQUAL_TO), value));
        return this;
    }

    template<typename Property>
    Criteria* andNotEqualTo(const Property& property, const Object& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::NOT_EQUAL_TO), value));
        return this;
    }

    template<typename Property>
    Criteria* andGreaterThan(const Property& property, const Object& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::GREATER_THAN), value));
        return this;
    }

    template<typename Property>
    Criteria* andGreaterThanOrEqualTo(const Property& property, const Object& value) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::GREATER_THAN_OR_EQUAL_TO), value));
        return this;
    }

    template<typename Property>
    Criteria* andLessThan(const Property& property, const Object& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::LESS_THAN), value));
        return this;
    }

    template<typename Property>
    Criteria* andLessThanOrEqualTo(const Property& property, const Object& value) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::LESS_THAN_OR_EQUAL_TO), value));
        return this;
    }

    template<typename Property>
    Criteria* andLike(const Property& property, const std::string& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::LIKE), value));
        return this;
    }

    template<typename Property>
    Criteria* andNotLike(const Property& property, const std::string& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::NOT_LIKE), value));
        return this;
    }

    template<typename Property>
    Criteria* andIn(const Property& property, const Iterable& values) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::IN), values));
        return this;
    }

    template<typename Property>
    Criteria* andNotIn(const Property& property, const Iterable& values) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::NOT_IN), values));
        return this;
    }

    template<typename Property>
    Criteria* andBetween(const Property& property, const Object& value1, const Object& value2) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::BETWEEN), value1, value2));
        return this;
    }

    template<typename Property>
    Criteria* andNotBetween(const Property& property, const Object& value1, const Object& value2) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::NOT_BETWEEN), value1, value2));
        return this;
    }

    template<typename Property>
    Criteria* andRegexp(const Property& property, std::string& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::REGEXP), value));
        return this;
    }

    template<typename Property>
    Criteria* andNotRegexp(const Property& property, std::string& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::NOT_REGEXP), value));
        return this;
    }

public:
    /* and condition provided by user */
    Criteria* andCondition(const std::string& condition) {
        m_criterions.emplace_back(Criterion(condition));
        return this;
    }

    Criteria* andCondition(const std::string& condition, const std::string& value) {
        m_criterions.emplace_back(Criterion(condition, value));
        return this;
    }

    template<typename Object>
    Criteria* andEqualTo(const Object& object) {
        std::shared_ptr<EntityTableMap> result_map = std::make_shared<EntityTableMap>();
        EntityHelper::getResultMap(const_cast<Object*>(&object), result_map);
        for (auto& property : result_map->getPropertyMap()) {
            if (property.second.getJoinType() != JoinType::OneToMany &&
                property.second.getTableAlias() != m_table->getAlias()) {
                this->andEqualTo(property.second.getProperty(),
                                 property.second.getEntityFieldValue());
            }
        }
        return this;
    }

public:
    /* or */
    template<typename Property>
    Criteria* orIsNull(const Property& property) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::IS_NULL), true));
        return this;
    }

    template<typename Property>
    Criteria* orIsNotNull(const Property& property) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::IS_NOT_NULL), true));
        return this;
    }

    template<typename Property>
    Criteria* orEqualTo(const Property& property, const Object& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::EQUAL_TO), value, true));
        return this;
    }

    template<typename Property>
    Criteria* orNotEqualTo(const Property& property, const Object& value) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::NOT_EQUAL_TO), value, true));
        return this;
    }

    template<typename Property>
    Criteria* orGreaterThan(const Property& property, const Object& value) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::GREATER_THAN), value, true));
        return this;
    }

    template<typename Property>
    Criteria* orGreaterThanOrEqualTo(const Property& property, const Object& value) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::GREATER_THAN_OR_EQUAL_TO), value, true));
        return this;
    }

    template<typename Property>
    Criteria* orLessThan(const Property& property, const Object& value) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::LESS_THAN), value, true));
        return this;
    }

    template<typename Property>
    Criteria* orLessThanOrEqualTo(const Property& property, const Object& value) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::LESS_THAN_OR_EQUAL_TO), value, true));
        return this;
    }

    template<typename Property>
    Criteria* orLike(const Property& property, const std::string& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::LIKE), value, true));
        return this;
    }

    template<typename Property>
    Criteria* orNotLike(const Property& property, const std::string& value) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::NOT_LIKE), value, true));
        return this;
    }

    template<typename Property>
    Criteria* orIn(const Property& property, const Iterable& values) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::IN), values, true));
        return this;
    }

    template<typename Property>
    Criteria* orNotIn(const Property& property, const Iterable& values) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::NOT_IN), values, true));
        return this;
    }

    template<typename Property>
    Criteria* orBetween(const Property& property, const Object& value1, const Object& value2) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::BETWEEN), value1, value2, true));
        return this;
    }

    template<typename Property>
    Criteria* orNotBetween(const Property& property, const Object& value1, const Object& value2) {
        m_criterions.emplace_back(
            Criterion(condition(property, Constants::NOT_BETWEEN), value1, value2, true));
        return this;
    }

    template<typename Property>
    Criteria* orRegexp(const Property& property) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::REGEXP), true));
        return this;
    }

    template<typename Property>
    Criteria* orNotRegexp(const Property& property) {
        m_criterions.emplace_back(Criterion(condition(property, Constants::NOT_REGEXP), true));
        return this;
    }

public:
    /* or condition provided by user */
    Criteria* orCondition(const std::string& condition) {
        m_criterions.emplace_back(Criterion(condition, true));
        return this;
    }

    Criteria* orCondition(const std::string& condition, const std::string& value) {
        m_criterions.emplace_back(Criterion(condition, value, true));
        return this;
    }


private:
    std::string column(const std::string property) const {
        auto it = m_property_map->find(property);
        if (it == m_property_map->end()) {
            throw std::runtime_error("Property not found: " + property);
        }
        return it->second.getColumnWithTableAlias();
    }

    std::string condition(const std::string& property, const std::string& comp_sql) {
        return column(property) + " " + comp_sql;
    }

    template<typename T, typename Entity>
    std::string condition(T Entity::*property_ptr, const std::string& comp_sql) {
        auto property = EntityHelper::getProperty(property_ptr);
        return column(property) + " " + comp_sql;
    }

private:
    std::vector<Criterion> m_criterions;
    ColumnMap* m_property_map = nullptr;

    std::string m_and_or;
    EntityTable* m_table = nullptr;
};

}   // namespace pico

#endif
