#ifndef __PICO_MAPPER_ENTITY_ITERABLE_H__
#define __PICO_MAPPER_ENTITY_ITERABLE_H__

#include <list>
#include <stdexcept>

#include "object.hpp"

namespace pico {
class Iterable : public Object
{
public:
    // set
    template<typename T>
    Iterable(const std::set<T>& value)
        : Object(typeid(std::set<T>), true, false) {
        save2Collection(value);
    }

    // list
    template<typename T>
    Iterable(const std::list<T>& value)
        : Object(typeid(std::list<T>), true, false) {
        save2Collection(value);
    }

    // vector
    template<typename T>
    Iterable(const std::vector<T>& value)
        : Object(typeid(std::vector<T>), true, false) {
        save2Collection(value);
    }

    // initializer_list
    template<typename T>
    Iterable(const std::initializer_list<T>& value)
        : Object(typeid(std::initializer_list<T>), true, false) {
        save2Collection(value);
    }

    Iterable() = default;

    size_t size() const { return m_buff.object_value.size(); }

    const Object& operator[](int index) const {
        if (index > (int)size()) { std::runtime_error("[out of max index]"); }
        return m_buff.object_value[index];
    }

private:
    template<typename Collection>
    void save2Collection(const Collection& collection) {
        for (const auto& c : collection) { this->m_buff.object_value.emplace_back(c); }
    }
};

}   // namespace pico

#endif
