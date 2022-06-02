#ifndef __PICO_MAPPER_ENTITY_ENTITY_TABLE_H__
#define __PICO_MAPPER_ENTITY_ENTITY_TABLE_H__

#include <string>

#include "../helper/alias_helper.hpp"

#include "../../util.h"

namespace pico {
class EntityTable
{
public:
    EntityTable() = default;

    template<typename Entity>
    EntityTable(Entity*, std::string class_name, std::string table_name)
        : m_table_name(std::move(table_name))
        , m_class_name(std::move(class_name)) {
        _alias = AliasHelper::getAliasFromType<Entity>();
    }

    template<typename Entity>
    EntityTable(Entity*, std::string class_name)
        : m_class_name(std::move(class_name)) {
        _alias = AliasHelper::getAliasFromType<Entity>();
        m_table_name = camel2underline(m_class_name);
    }

    const std::string& getTableName() const { return m_table_name; }
    const std::string& getClassName() const { return m_class_name; }
    const std::string& getAlias() const { return _alias; }

private:
    std::string m_table_name;
    std::string m_class_name;
    std::string _alias;
};

}   // namespace pico

#endif