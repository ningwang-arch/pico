#ifndef __PICO_MAPPER_ENTITY_ORDER_BY_H__
#define __PICO_MAPPER_ENTITY_ORDER_BY_H__

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "../helper/entity_helper.hpp"
#include "constants.h"
#include "entity_column.hpp"

namespace pico {
class OrderBy : public std::enable_shared_from_this<OrderBy>
{
public:
    typedef std::shared_ptr<OrderBy> Ptr;
    typedef std::unordered_map<std::string, EntityColumn> ColumnMap;

    OrderBy() = default;
    explicit OrderBy(ColumnMap property_map) {
        m_property_map = std::make_shared<ColumnMap>(property_map);
    }

    explicit OrderBy(std::shared_ptr<ColumnMap> property_map) { m_property_map = property_map; }

    template<typename T, typename Entity>
    std::shared_ptr<OrderBy> desc(T Entity::*property) {
        m_order_by_list.emplace_back(column(EntityHelper::getProrperty(property)) + " " +
                                     Constants::DESC);
        return shared_from_this();
    }

    template<typename T, typename Entity>
    std::shared_ptr<OrderBy> asc(T Entity::*property) {
        m_order_by_list.emplace_back(column(EntityHelper::getProrperty(property)) + " " +
                                     Constants::ASC);
        return shared_from_this();
    }

    const std::vector<std::string>& getOrderByList() const { return m_order_by_list; }

private:
    std::string column(const std::string property) const {
        auto it = m_property_map->find(property);
        if (it == m_property_map->end()) {
            throw std::runtime_error("property " + property + " not found");
        }
        return it->second.getColumnWithTableAlias();
    }

private:
    std::shared_ptr<ColumnMap> m_property_map;
    std::vector<std::string> m_order_by_list;
};

}   // namespace pico

#endif