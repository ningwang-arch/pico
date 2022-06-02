#ifndef __PICO_MAPPER_ENTITY_COLLECTION_INFO_H__
#define __PICO_MAPPER_ENTITY_COLLECTION_INFO_H__

#include "object.hpp"

namespace pico {
class CollectionInfo
{
public:
    enum class Type
    {
        Null,
        List,
        Set,
        Vector
    };

    CollectionInfo(Type type, const std::vector<Object>& values)
        : m_type(type)
        , m_values(values) {
        m_size = values.size();
    }
    CollectionInfo() = default;

    Type type() const { return m_type; }
    int size() const { return m_size; }
    const std::vector<Object>& values() const { return m_values; }

private:
    Type m_type = Type::Null;
    int m_size = 0;
    std::vector<Object> m_values;
};


}   // namespace pico

#endif