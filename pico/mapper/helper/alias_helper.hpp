#ifndef __PICO_MAPPER_HELPER_ALIAS_HELPER_H__
#define __PICO_MAPPER_HELPER_ALIAS_HELPER_H__


#include <set>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "../../singleton.h"
#include "../../util.h"

namespace pico {

class AliasHelper : public Singleton<AliasHelper>
{
public:
    template<typename T>
    static std::string getAliasFromType() {
        auto helper = getInstance();
        if (helper->_alias_map.find(typeid(T)) != helper->_alias_map.end()) {
            return helper->_alias_map[typeid(T)];
        }
        return helper->genTypeAlias<T>();
    }

private:
    template<typename T>
    std::string genTypeAlias() {
        std::string alias;
        while (alias.empty()) {
            char c = (char)genRandom('a', 'z');
            int num = genRandom(0, 9);
            alias = std::string(1, c) + std::to_string(num);
            if (!_alias_set.insert(alias).second) { alias.clear(); }
        }
        _alias_map[std::type_index(typeid(T))] = alias;
        return alias;
    }

private:
    std::unordered_map<std::type_index, std::string> _alias_map;
    std::set<std::string> _alias_set;
};

}   // namespace pico

#endif