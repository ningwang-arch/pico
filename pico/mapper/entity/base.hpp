#ifndef __PICO_MAPPER_ENTITY_BASE_HPP__
#define __PICO_MAPPER_ENTITY_BASE_HPP__

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../builder/sql_builder.h"
#include "../entity/criteria.hpp"
#include "../entity/entity_column.hpp"
#include "../entity/entity_table.h"
#include "../helper/alias_helper.hpp"
#include "../helper/base_helper.hpp"
#include "../helper/entity_helper.hpp"
#include "constants.h"
#include "criterion.h"
#include "entity_enum.h"
#include "entity_table_map.h"
#include "entity_wrapper.h"
#include "join_entity_table.h"
#include "object.hpp"
#include "order_by.h"

namespace pico {
template<typename Entity>
class Base
{
public:
    Base() {
        auto result_map = std::make_shared<EntityTableMap>();
        EntityHelper::getResultMap(m_entity_class.get(), result_map);
        for (auto& table : result_map->getEntityTables()) {
            if (AliasHelper::getAliasFromType<Entity>() == table.getAlias()) { m_table = table; }
            else {
                m_join_table_map.insert(std::make_pair(table.getAlias(), table));
            }
        }

        for (auto& property : result_map->getPropertyMap()) {
            m_property_map.insert(property);
            if (property.second.getColumnType() == ColumnType::Id &&
                property.second.getTableAlias() == m_table.getAlias()) {
                m_primary_key_col = std::make_shared<EntityColumn>(property.second);
            }

            if (property.second.getJoinType() != JoinType::Null) {
                m_join_col.emplace_back(property.second);
            }

            if (property.second.getJoinType() != JoinType::Null) {
                auto& join_table = m_join_table_map[property.second.getJoinTableAlias()];
                join_table.setJoinColumn(property.second.getColumnWithTableAlias());
                join_table.setJoinProperty(property.second.getProperty());
                join_table.setJoinAlias(property.second.getTableAlias());
                join_table.setJoinedProperty(property.second.getJoinProperty());
            }

            if (AliasHelper::getAliasFromType<Entity>() == property.second.getTableAlias()) {
                m_entity_property_map.insert(property);
            }
        }

        for (auto& join_map : m_join_table_map) {
            auto join_col =
                m_property_map[join_map.second.getJoinedProperty()].getColumnWithTableAlias();
            join_map.second.setJoinedColumn(join_col);
        }
    }

    std::shared_ptr<Criteria> createCriteria() {
        auto criteria = this->createCriteriaInternal();
        if (m_ored_criteria.empty()) {
            criteria->setAndOr(Constants::AND);
            m_ored_criteria.emplace_back(criteria);
        }
        return criteria;
    }

    void orCriteria(std::shared_ptr<Criteria> criteria) {
        criteria->setAndOr(Constants::OR);
        m_ored_criteria.emplace_back(criteria);
    }

    std::shared_ptr<Criteria> orCriteria() {
        auto criteria = createCriteriaInternal();
        criteria->setAndOr(Constants::OR);
        m_ored_criteria.emplace_back(criteria);
        return criteria;
    }
    void andCriteria(std::shared_ptr<Criteria> criteria) {
        criteria->setAndOr(Constants::AND);
        m_ored_criteria.emplace_back(criteria);
    }

    std::shared_ptr<Criteria> andCriteria() {
        auto criteria = createCriteriaInternal();
        criteria->setAndOr(Constants::AND);
        m_ored_criteria.emplace_back(criteria);
        return criteria;
    }

    template<typename R, typename T>
    std::shared_ptr<OrderBy> orderByDesc(R T::*t) {
        if (m_order == nullptr) { m_order = std::make_shared<OrderBy>(&m_property_map); }
        return m_order->desc(t);
    }

    template<typename R, typename T>
    std::shared_ptr<OrderBy> orderByAsc(R T::*t) {
        if (m_order == nullptr) { m_order = std::make_shared<OrderBy>(&m_property_map); }
        return m_order->asc(t);
    }

    void limit(int offset, int size) {
        if (offset > 0 && size <= 0) { return; }
        m_limit_pair = {offset, size};
    }

    void limit(int size) { limit(0, size); }

public:
    std::pair<std::string, std::vector<Object>> getInsertContext(const Entity& record,
                                                                 bool selective) {
        std::shared_ptr<SQLBuilder> sql_builder = std::make_shared<SQLBuilder>();
        sql_builder->INSERT_INTO(m_table.getTableName());
        std::shared_ptr<EntityTableMap> result_map = std::make_shared<EntityTableMap>();
        std::shared_ptr<Criteria> insert_criteria =
            std::make_shared<Criteria>(const_cast<decltype(m_property_map)*>(&m_property_map),
                                       const_cast<decltype(m_table)*>(&m_table));
        EntityHelper::getResultMap(const_cast<Entity*>(&record), result_map);
        for (auto& property : result_map->getPropertyMap()) {
            if (property.second.getJoinType() != JoinType::OneToMany &&
                property.second.getTableAlias() == m_table.getAlias() &&
                property.second.getColumnType() != ColumnType::Id) {
                if (selective && property.second.isNull()) { continue; }
                sql_builder->VALUES("`" + property.second.getCloumn() + "`",
                                    Constants::PLACEHOLDER);
                insert_criteria->andEqualTo(property.second.getProperty(),
                                            property.second.getEntityFieldValue());
            }
        }
        return std::make_pair(sql_builder->to_string(),
                              BaseHelper::getValuesFromOredCriteria({insert_criteria}));
    }

    std::pair<std::string, std::vector<Object>> getDeleteContext() const {
        auto sql_builder = std::make_shared<SQLBuilder>();
        sql_builder->DELETE_FROM(m_table.getTableName() + " " + Constants::AS + " " +
                                 m_table.getAlias());
        for (auto& criteria : m_ored_criteria) {
            if (criteria->getAndOr() == Constants::OR) { sql_builder->OR(); }
            sql_builder->WHERE(BaseHelper::getConditionFromCriteria(*criteria));
        }
        return std::make_pair(sql_builder->to_string(),
                              BaseHelper::getValuesFromOredCriteria(m_ored_criteria));
    }

    std::pair<std::string, std::vector<Object>> getUpdateContext(const Entity& param,
                                                                 bool selective) const {
        auto sql_builder = std::make_shared<SQLBuilder>();
        sql_builder->UPDATE(m_table.getTableName() + " " + Constants::AS + " " +
                            m_table.getAlias());
        std::shared_ptr<EntityTableMap> result_map = std::make_shared<EntityTableMap>();
        EntityHelper::getResultMap(const_cast<Entity*>(&param), result_map);
        std::shared_ptr<Criteria> update_criteria =
            std::make_shared<Criteria>(const_cast<decltype(m_property_map)*>(&m_property_map),
                                       const_cast<decltype(m_table)*>(&m_table));
        for (auto& property : result_map->getPropertyMap()) {
            if (property.second.getJoinType() != JoinType::OneToMany &&
                property.second.getTableAlias() == m_table.getAlias() &&
                property.second.getColumnType() != ColumnType::Id) {
                if (selective && property.second.isNull()) { continue; }
                sql_builder->SET(property.second.getColumnWithTableAlias() + " " +
                                 Constants::EQUAL_TO + " " + Constants::PLACEHOLDER);
                update_criteria->andEqualTo(property.second.getProperty(),
                                            property.second.getEntityFieldValue());
            }
        }
        auto update_values = BaseHelper::getValuesFromOredCriteria({update_criteria});
        buildOredCriteria(sql_builder);
        auto where_values = BaseHelper::getValuesFromOredCriteria(m_ored_criteria);
        update_values.insert(update_values.end(), where_values.begin(), where_values.end());
        return std::make_pair(sql_builder->to_string(), update_values);
    }

    std::pair<std::string, std::vector<Object>> getSelectCount() const {
        auto sql_builder = std::make_shared<SQLBuilder>();
        sql_builder->SELECT(Constants::COUNT);
        buildFromeWhereStmt(sql_builder);
        return std::make_pair(sql_builder->to_string(),
                              BaseHelper::getValuesFromOredCriteria(m_ored_criteria));
    }

    std::pair<std::string, std::vector<Object>> getSelectContext() const {
        auto sql_builder = std::make_shared<SQLBuilder>();
        for (const auto& property : m_property_map) {
            if (property.second.getJoinType() != JoinType::OneToMany) {
                sql_builder->SELECT(property.second.getColumnWithTableAlias() + " " +
                                    Constants::AS + " " + property.second.getAlias());
            }
        }

        buildFromeWhereStmt(sql_builder);
        buildOrderBy(sql_builder);
        auto ored_values = BaseHelper::getValuesFromOredCriteria(m_ored_criteria);

        if (m_limit_pair.second != INT32_MIN) {
            buildLimit(sql_builder);
            ored_values.emplace_back(m_limit_pair.second);
            ored_values.emplace_back(m_limit_pair.first);
        }
        return std::make_pair(sql_builder->to_string(), ored_values);
    }

    std::unordered_map<std::string, EntityColumn> getColumnAliasMap() {
        std::unordered_map<std::string, EntityColumn> res;
        for (auto& pm : m_property_map) {
            if (pm.second.getJoinType() != JoinType::OneToMany) {
                res.insert({pm.second.getAlias(), pm.second});
            }
        }
        return res;
    }

    std::shared_ptr<EntityColumn> getKeyEntityColumn(const Entity* entity = nullptr) const {
        if (!entity) { return m_primary_key_col; }

        auto result_map = std::make_shared<EntityTableMap>();
        auto primary_entity_col = std::make_shared<EntityColumn>();
        EntityHelper::getResultMap(const_cast<Entity*>(entity), result_map);
        for (auto& property : result_map->getPropertyMap()) {
            if (property.second.getTableAlias() == m_table.getAlias() &&
                property.second.getColumnType() == ColumnType::Id) {
                *primary_entity_col = property.second;
                return primary_entity_col;
            }
        }
        return nullptr;
    }

    const std::vector<EntityColumn>& getJoinEntityColumns() const { return m_join_col; }

    const std::shared_ptr<Entity>& getEntity() const { return m_entity_class; }

private:
    std::shared_ptr<Criteria> createCriteriaInternal() {
        return std::make_shared<Criteria>(&m_property_map, &m_table);
    }

    void buildOredCriteria(const std::shared_ptr<SQLBuilder>& sql_builder) const {
        for (auto& criteria : m_ored_criteria) {
            if (criteria->getAndOr() == Constants::OR) { sql_builder->OR(); }
            sql_builder->WHERE(BaseHelper::getConditionFromCriteria(*criteria));
        }
    }

    void buildOrderBy(const std::shared_ptr<SQLBuilder>& sql_builder) const {
        if (m_order) {
            for (auto& order : m_order->getOrderByList()) { sql_builder->ORDER_BY(order); }
        }
    }

    void buildFromeWhereStmt(const std::shared_ptr<SQLBuilder>& sql_builder) const {
        sql_builder->FROM(m_table.getTableName() + " " + Constants::AS + " " + m_table.getAlias());
        for (const auto& jt : m_join_table_map) {
            sql_builder->LEFT_OUTER_JOIN(jt.second.getTableName() + " " + Constants::AS + " " +
                                         jt.second.getAlias() + " " + Constants::ON + " " +
                                         jt.second.getJoinColumn() + " " + Constants::EQUAL_TO +
                                         " " + jt.second.getJoinedColumn());
        }
        buildOredCriteria(sql_builder);
    }

    void buildLimit(const std::shared_ptr<SQLBuilder>& sql_builder) const {
        sql_builder->LIMIT(Constants::PLACEHOLDER, Constants::PLACEHOLDER);
    }

private:
    std::vector<std::shared_ptr<Criteria>> m_ored_criteria;
    std::unordered_map<std::string, EntityColumn> m_property_map;
    std::shared_ptr<Entity> m_entity_class = std::make_shared<Entity>();
    EntityTable m_table;
    std::unordered_map<std::string, JoinEntityTable> m_join_table_map;
    std::unordered_map<std::string, EntityColumn> m_entity_property_map;

    std::shared_ptr<EntityColumn> m_primary_key_col = nullptr;
    std::vector<EntityColumn> m_join_col;

    std::shared_ptr<OrderBy> m_order = nullptr;
    std::pair<int /* offset */, int /* limit */> m_limit_pair = {0, INT32_MIN};
};

}   // namespace pico

#endif
