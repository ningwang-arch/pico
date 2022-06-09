#ifndef __PICO_MAPPER_HELPER_ENTITY_HELPER_H__
#define __PICO_MAPPER_HELPER_ENTITY_HELPER_H__

#include <map>
#include <memory>
#include <string>
#include <tuple>

#include "../entity/entity_table_map.h"
#include "../entity/entity_wrapper.h"

namespace pico {
template<class T>
struct getType
{ typedef T type; };

template<class T>
struct getType<T*>
{ typedef T type; };

class EntityColumn;

class EntityHelper
{
public:
    template<typename Entity>
    static void getResultMap(Entity* entity, std::shared_ptr<EntityTableMap> resultMap) {
        auto reflection_info = EntityWrapper<Entity>().getReflectionInfo(entity);
        getResultMap(entity, reflection_info, resultMap);
    }

    template<typename T, typename Entity>
    static std::string getProperty(T Entity::*member) {
        auto entity = std::make_shared<Entity>();
        auto reflection_info = EntityWrapper<Entity>().getReflectionInfo(entity.get());
        return getProperty(reflection_info, member);
    }

    template<typename Entity>
    static void appendPropertyValues(const std::string& property, const Entity* from, Entity* to) {
        auto reflection_info = EntityWrapper<Entity>().getReflectionInfo(to);
        appendPropertyValues(reflection_info, property, from, to);
    }

    template<typename Entity>
    static void clearPropertyValues(const std::string& property, Entity* entity) {
        auto reflection_info = EntityWrapper<Entity>().getReflectionInfo(entity);
        clearPropertyValues(reflection_info, property, entity);
    }

private:
    struct EntityTupleGetter
    {
        template<typename T, typename Entity>
        static void appendPropertyValues(const T&, const std::string&, const Entity*, Entity*) {}

        template<typename R, typename T, typename Entity>
        static void appendPropertyValues(const std::pair<std::vector<R> T::*, EntityColumn>& pair,
                                         const std::string& property, const Entity* from,
                                         Entity* to) {
            if (pair.second.getProperty() == property) {
                (to->*pair.first)
                    .insert((to->*pair.first).end(),
                            (from->*pair.first).begin(),
                            (from->*pair.first).end());
            }
        }

        template<typename T, typename Entity>
        static void clearPropertyValues(const T&, const std::string&, Entity*) {}

        template<typename R, typename T, typename Entity>
        static void clearPropertyValues(const std::pair<std::vector<R> T::*, EntityColumn>& pair,
                                        const std::string& property, Entity* entity) {
            if (pair.second.getProperty() == property) { (entity->*pair.first).clear(); }
        }

        template<typename T>
        static std::string getProperty(T) {
            return {};
        }

        template<typename T>
        static std::string getProperty(std::pair<T, EntityColumn> pair) {
            return pair.second.getProperty();
        }

        template<typename T, typename U>
        static bool isMatchProperty(T, U) {
            return false;
        }

        template<typename T>
        static bool isMatchProperty(std::pair<int T::*, EntityColumn> pair, int T::*u) {
            return pair.first == u;
        }

        template<typename T>
        static bool isMatchProperty(std::pair<std::string T::*, EntityColumn> pair,
                                    std::string T::*u) {
            return pair.first == u;
        }

        template<typename T>
        static bool isMatchProperty(std::pair<std::time_t T::*, EntityColumn> pair,
                                    std::time_t T::*u) {
            return pair.first == u;
        }

        template<typename T>
        static bool isMatchProperty(std::pair<uint64_t T::*, EntityColumn> pair, uint64_t T::*u) {
            return pair.first == u;
        }

        template<typename Entity, typename R, typename T>
        static auto getEntityPropertyPtr(Entity* entity,
                                         const std::pair<R T::*, EntityColumn>& pair)
            -> decltype(&(entity->*pair.first)) {
            return &(entity->*pair.first);
        }

        template<typename Entity, typename R, typename T>
        static auto getEntityPropertyPtr(Entity* entity,
                                         const std::pair<std::vector<R> T::*, EntityColumn>& pair)
            -> decltype(&(entity->*pair.first).front()) {
            (entity->*pair.first).emplace_back(R());
            return &(entity->*pair.first).front();
        }

        template<typename Entity, typename T>
        static void bind2ResultMap(Entity*, const T&, std::shared_ptr<EntityTableMap>) {}

        template<typename Entity>
        static void bind2ResultMap(Entity*, const EntityTable& entityTable,
                                   std::shared_ptr<EntityTableMap> resultMap) {
            resultMap->getEntityTables().emplace_back(entityTable);
        }

        template<typename Entity, typename R, typename T>
        static void bind2ResultMap(Entity* entity, const std::pair<R T::*, EntityColumn>& pair,
                                   std::shared_ptr<EntityTableMap> resultMap) {
            auto& property_map = resultMap->getPropertyMap();
            property_map.insert(std::make_pair(pair.second.getProperty(), pair.second));
            auto entity_property_ptr = getEntityPropertyPtr(entity, pair);

            auto related_reflection_info =
                EntityWrapper<typename getType<decltype(entity_property_ptr)>::type>()
                    .getReflectionInfo(entity_property_ptr);

            EntityHelper::getResultMap(entity_property_ptr, related_reflection_info, resultMap);
        }
    };

    template<typename Tuple, size_t N>
    struct ResultGetter
    {
        template<typename Entity>
        static void getResultMap(Entity* entity, const Tuple& tuple,
                                 std::shared_ptr<EntityTableMap> resultMap) {
            ResultGetter<Tuple, N - 1>::getResultMap(entity, tuple, resultMap);
            EntityTupleGetter::bind2ResultMap(entity, std::get<N - 1>(tuple), resultMap);
        }

        template<typename T>
        static void getProperty(const Tuple& tuple, T t, std::string& property) {
            ResultGetter<Tuple, N - 1>::getProperty(tuple, t, property);
            auto value = std::get<N - 1>(tuple);
            if (EntityTupleGetter::isMatchProperty(value, t)) {
                property = EntityTupleGetter::getProperty(value);
            }
        }

        template<typename Entity>
        static void appendPropertyValues(const Tuple& tuple, const std::string& property,
                                         const Entity* from, Entity* to) {
            ResultGetter<Tuple, N - 1>::appendPropertyValues(tuple, property, from, to);
            EntityTupleGetter::appendPropertyValues(std::get<N - 1>(tuple), property, from, to);
        }

        template<typename Entity>
        static void clearPropertyValues(const Tuple& tuple, const std::string& property,
                                        Entity* entity) {
            ResultGetter<Tuple, N - 1>::clearPropertyValues(tuple, property, entity);
            EntityTupleGetter::clearPropertyValues(std::get<N - 1>(tuple), property, entity);
        }
    };

    template<typename Tuple>
    struct ResultGetter<Tuple, 0>
    {
        template<typename Entity>
        static void getResultMap(Entity*, const Tuple&, std::shared_ptr<EntityTableMap>) {}

        template<typename T>
        static void getProperty(const Tuple&, T, std::string&) {}

        template<typename Entity>
        static void appendPropertyValues(const Tuple&, const std::string&, const Entity*, Entity*) {
        }

        template<typename Entity>
        static void clearPropertyValues(const Tuple&, const std::string&, Entity*) {}
    };

    template<typename... Args, typename T>
    static std::string getProperty(const std::tuple<Args...>& tuple, T t) {
        std::string property;
        ResultGetter<decltype(tuple), sizeof...(Args)>::getProperty(tuple, t, property);
        return property;
    }

    template<typename Entity, typename T>
    static void getResultMap(Entity, T, std::shared_ptr<EntityTableMap> resultMap) {}

    template<typename Entity, typename... Args>
    static void getResultMap(Entity* entity, const std::tuple<Args...>& tuple,
                             std::shared_ptr<EntityTableMap> resultMap) {
        ResultGetter<decltype(tuple), sizeof...(Args)>::getResultMap(entity, tuple, resultMap);
    }

    template<typename Entity, typename... Args>
    static void appendPropertyValues(const std::tuple<Args...>& tuple, const std::string& property,
                                     const Entity* from, Entity* to) {
        ResultGetter<decltype(tuple), sizeof...(Args)>::appendPropertyValues(
            tuple, property, from, to);
    }

    template<typename Entity, typename... Args>
    static void clearPropertyValues(const std::tuple<Args...>& tuple, const std::string& property,
                                    Entity* entity) {
        ResultGetter<decltype(tuple), sizeof...(Args)>::clearPropertyValues(
            tuple, property, entity);
    }
};

}   // namespace pico

#endif
