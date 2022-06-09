#ifndef __PICO_MAPPER_ENTITY_ENTITY_TABLE_MAP_H__
#define __PICO_MAPPER_ENTITY_ENTITY_TABLE_MAP_H__

#include <map>
#include <set>
#include <string>
#include <vector>

namespace pico {
class EntityTable;
class EntityColumn;

class EntityTableMap
{
public:
    std::vector<EntityTable>& getEntityTables() { return m_tables; }
    void setEntityTables(const std::vector<EntityTable>& tables) { m_tables = tables; }

    std::map<std::string, EntityColumn>& getPropertyMap() { return m_property_map; }
    void setPropertyMap(const std::map<std::string, EntityColumn>& property_map) {
        m_property_map = property_map;
    }

    std::string& getKeyProperty() { return m_key_property; }
    void setKeyProperty(const std::string& key_property) { m_key_property = key_property; }

    std::set<std::string>& getKeyColumns() { return m_key_columns; }
    void setKeyColumns(const std::set<std::string>& key_columns) { m_key_columns = key_columns; }

private:
    std::vector<EntityTable> m_tables;
    std::map<std::string, EntityColumn> m_property_map;
    std::string m_key_property;
    std::set<std::string> m_key_columns;
};

}   // namespace pico

#endif