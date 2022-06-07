#ifndef __PICO_MAPPER_ENTITY_ENTITY_COLUMN_H__
#define __PICO_MAPPER_ENTITY_ENTITY_COLUMN_H__

#include <string>
#include <typeindex>

#include "../../util.h"
#include "../helper/alias_helper.hpp"
#include "../helper/entity_helper.hpp"
#include "entity_enum.h"
#include "object.hpp"

namespace pico {
class EntityColumn
{
public:
    EntityColumn() = default;

    template<typename Entity, typename T>
    EntityColumn(Entity*, T* p_property, const std::string& property, const std::string& column,
                 ColumnType column_type, PrimaryKeyPolicy policy, JoinType join_type) {
        m_type = typeid(T);
        m_table_alias = AliasHelper::getAliasFromType<Entity>();
        mp_property = (void*)p_property;
        m_property = m_table_alias + "." + property;
        m_col = column;
        m_column_type = column_type;
        m_pk_policy = policy;
        m_join_type = join_type;
        m_alias = m_table_alias + "_" + m_col;
        m_is_container = isContainer(T{});
    }

    template<typename Entity, typename T, typename J>
    EntityColumn(Entity* entity, T* p_property, const std::string& property,
                 const std::string& column, ColumnType column_type, PrimaryKeyPolicy policy,
                 JoinType join_type, const J join_col) {
        new (this)
            EntityColumn(entity, p_property, property, column, column_type, policy, join_type);
        this->getJoinPropertyAndTableAlias(join_col, m_join_property, m_join_table_alias);
        this->getJoinValue(m_join_value, p_property, join_col);
    }

    template<typename Entity, typename T, typename J>
    EntityColumn(Entity* entity, T* p_property, const std::string& property,
                 const std::string& column, JoinType join_type, const J join_col) {
        new (this) EntityColumn(entity,
                                p_property,
                                property,
                                column,
                                ColumnType::Null,
                                PrimaryKeyPolicy::Null,
                                join_type,
                                join_col);
    }

    template<typename Entity, typename T, typename J>
    EntityColumn(Entity* entity, T* p_property, const std::string& property, JoinType join_type,
                 const J join_col) {
        new (this) EntityColumn(
            entity, p_property, property, camel2underline(property), join_type, join_col);
    }

    template<typename Entity, typename T>
    EntityColumn(Entity* entity, T* p_property, const std::string& property,
                 const std::string& column, ColumnType column_type, PrimaryKeyPolicy policy) {
        new (this)
            EntityColumn(entity, p_property, property, column, column_type, policy, JoinType::Null);
    }

    template<typename Entity, typename T>
    EntityColumn(Entity* entity, T* p_property, const std::string& property,
                 const std::string& column, ColumnType column_type) {
        new (this) EntityColumn(
            entity,
            p_property,
            property,
            column,
            column_type,
            column_type == ColumnType::Id ? PrimaryKeyPolicy::Id : PrimaryKeyPolicy::Null);
    }

    template<typename Entity, typename T>
    EntityColumn(Entity* entity, T* p_property, const std::string& property,
                 const std::string& column) {
        new (this) EntityColumn(entity, p_property, property, column, ColumnType::Null);
    }

    template<typename Entity, typename T>
    EntityColumn(Entity* entity, T* p_property, const std::string& property, ColumnType column_type,
                 PrimaryKeyPolicy policy) {
        new (this) EntityColumn(
            entity, p_property, property, camel2underline(property), column_type, policy);
    }

    template<typename Entity, typename T>
    EntityColumn(Entity* entity, T* p_property, const std::string& property,
                 ColumnType column_type) {
        new (this) EntityColumn(
            entity,
            p_property,
            property,
            camel2underline(property),
            column_type,
            column_type == ColumnType::Id ? PrimaryKeyPolicy::Id : PrimaryKeyPolicy::Null);
    }

    template<typename Entity, typename T>
    EntityColumn(Entity* entity, T* p_property, const std::string& property) {
        new (this) EntityColumn(entity, p_property, property, camel2underline(property));
    }

    const std::string& getCloumn() const { return m_col; }
    const std::string& getProperty() const { return m_property; }
    const std::string& getTableAlias() const { return m_table_alias; }
    const std::string& getAlias() const { return m_alias; }

    std::string getColumnWithTableAlias() const { return m_table_alias + "." + m_col; }

    JoinType getJoinType() const { return m_join_type; }

    const std::string getJoinProperty() const { return m_join_property; }
    const std::string getJoinTableAlias() const { return m_join_table_alias; }
    const std::type_index& getType() const { return m_type; }
    ColumnType getColumnType() const { return m_column_type; }


    void bindValue2EntityField(const Object& value) {
        auto type = getType();
        if (type == typeid(int)) { *(int*)mp_property = value.getValue<int>(); }
        if (type == typeid(std::time_t)) {
            *(std::time_t*)mp_property = value.getValue<std::time_t>();
        }
        if (type == typeid(std::string)) {
            *(std::string*)mp_property = value.getValue<std::string>();
        }
    }

    Object getEntityFieldValue() const {
        auto type = getType();
        if (type == typeid(int)) { return Object(*(int*)mp_property); }
        if (type == typeid(std::time_t)) { return Object(*(std::time_t*)mp_property); }
        if (type == typeid(std::string)) { return Object(*(std::string*)mp_property); }

        if (this->getJoinType() == JoinType::OneToOne) { return m_join_value; }

        return {};
    }

    bool isNull() {
        if (getType() == typeid(std::string)) { return ((std::string*)mp_property)->empty(); }
        return false;
    }

    bool container() const { return m_is_container; }


private:
    template<typename T>
    void getJoinPropertyAndTableAlias(const T&, std::string&, std::string&) {}

    template<typename Entity, typename Ret>
    void getJoinPropertyAndTableAlias(Ret Entity::*join_column, std::string& join_property,
                                      std::string& join_table_alias) {
        join_property = EntityHelper::getProperty(join_column);
        join_table_alias = AliasHelper::getAliasFromType<Entity>();
    }

    template<typename P, typename Entity, typename Ret>
    void getJoinValue(Object& value, P* property, Ret Entity::*join_column) {
        value = Object(property->*join_column);
    }

    template<typename P, typename Entity, typename Ret>
    void getJoinValue(Object&, std::vector<P>*, Ret Entity::*join_column) {}

private:
    void* mp_property = nullptr;
    std::type_index m_type = std::type_index(typeid(void));
    std::string m_col = "";
    std::string m_property = "";
    std::string m_alias;
    ColumnType m_column_type = ColumnType::Null;
    PrimaryKeyPolicy m_pk_policy = PrimaryKeyPolicy::Null;
    JoinType m_join_type = JoinType::Null;
    std::string m_join_property;
    Object m_join_value;

    std::string m_table_alias;
    std::string m_join_table_alias;
    bool m_is_container = false;
};

}   // namespace pico

#endif
