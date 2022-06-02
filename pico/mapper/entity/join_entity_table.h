#ifndef __PICO_MAPPER_ENTITY_JOIN_ENTITY_TABLE_H__
#define __PICO_MAPPER_ENTITY_JOIN_ENTITY_TABLE_H__

#include <string>

#include "entity_table.h"

namespace pico {
class JoinEntityTable : public EntityTable
{
public:
    JoinEntityTable() = default;

    explicit JoinEntityTable(const EntityTable& table)
        : EntityTable(table) {}

    // setter
    void setJoinedColumn(const std::string& column) { m_joined_column = column; }
    void setJoinedProperty(const std::string& property) { m_joined_property = property; }
    void setJoinColumn(const std::string& column) { m_join_column = column; }
    void setJoinProperty(const std::string& property) { m_join_property = property; }
    void setJoinAlias(const std::string& alias) { m_join_alias = alias; }

    // getter
    const std::string& getJoinedColumn() const { return m_joined_column; }
    const std::string& getJoinedProperty() const { return m_joined_property; }
    const std::string& getJoinColumn() const { return m_join_column; }
    const std::string& getJoinProperty() const { return m_join_property; }
    const std::string& getJoinAlias() const { return m_join_alias; }

private:
    /*
     * 存储左表的列名
     */
    std::string m_joined_column;
    std::string m_joined_property;

    /*
     * 存储右表的列名
     */
    std::string m_join_column;
    std::string m_join_property;
    std::string m_join_alias;
};

}   // namespace pico

#endif