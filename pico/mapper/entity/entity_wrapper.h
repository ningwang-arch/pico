#ifndef __PICO_MAPPER_ENTITY_ENTITY_WRAPPER_H__
#define __PICO_MAPPER_ENTITY_ENTITY_WRAPPER_H__

#include <string>
#include <type_traits>

// #include "entity_column.hpp"
#include "entity_table.h"
#include "entity_table_map.h"

namespace pico {
class EntityColumn;

template<typename Entity>
class EntityWrapper
{
public:
    void* getReflectionInfo(Entity* entity) { return nullptr; }
};



#define PropertyMap(property, ...)         \
    std::make_pair(&EntityClass::property, \
                   pico::EntityColumn(entity, &entity->property, #property, ##__VA_ARGS__))

#define EntityMap(Entity, ...) \
    std::make_pair(Entity{}, pico::EntityTable((Entity*)nullptr, #Entity, ##__VA_ARGS__))

#define ResultMap(EntityTable, ...)                                           \
    template<>                                                                \
    class pico::EntityWrapper<decltype(EntityTable.first)>                    \
    {                                                                         \
        using EntityClass = decltype(EntityTable.first);                      \
                                                                              \
    public:                                                                   \
        auto getReflectionInfo(EntityClass* entity)                           \
            -> decltype(std::make_tuple(EntityTable.second, ##__VA_ARGS__)) { \
            return std::make_tuple(EntityTable.second, ##__VA_ARGS__);        \
        }                                                                     \
    };


}   // namespace pico

#endif
