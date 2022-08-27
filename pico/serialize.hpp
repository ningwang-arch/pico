#ifndef __PICO_SERIALIZE_H__
#define __PICO_SERIALIZE_H__

#include "reflection.hpp"

namespace pico {
template<typename T>
std::string serialize(
    const T& t, typename std::enable_if<std::is_arithmetic<T>::value && !pico::TypeTraits<T>::value,
                                        int>::type = 0) {
    return std::to_string(t);
}

template<typename T>
std::string serialize(const T& t,
                      typename std::enable_if<pico::TypeTraits<T>::value, int>::type = 0) {
    return t;
}

template<typename T>
std::string
serialize(const T& t,
          typename std::enable_if<!std::is_arithmetic<T>::value && !pico::TypeTraits<T>::value,
                                  int>::type = 0) {
    return t.encode();
}

template<typename T>
std::string serialize(const std::vector<T>& t) {
    Json::Value root = Json::Value(Json::arrayValue);
    for (auto& i : t) {
        root.append(serialize(i));
    }
    return pico::Json2Str(root);
}

// list
template<typename T>
std::string serialize(const std::list<T>& t) {
    Json::Value root = Json::Value(Json::arrayValue);
    for (auto& i : t) {
        root.append(serialize(i));
    }
    return pico::Json2Str(root);
}

// map
template<typename K, typename V>
std::string serialize(const std::map<K, V>& t) {
    Json::Value root = Json::Value(Json::objectValue);
    for (auto& i : t) {
        root[serialize(i.first)] = serialize(i.second);
    }
    return pico::Json2Str(root);
}

template<typename K, typename V>
std::string serialize(const std::unordered_map<K, V>& t) {
    Json::Value root = Json::Value(Json::objectValue);
    for (auto& i : t) {
        root[serialize(i.first)] = serialize(i.second);
    }
    return pico::Json2Str(root);
}

// set
template<typename T>
std::string serialize(const std::set<T>& t) {
    Json::Value root = Json::Value(Json::arrayValue);
    for (auto& i : t) {
        root.append(serialize(i));
    }
    return pico::Json2Str(root);
}

template<typename T>
std::string serialize(const std::unordered_set<T>& t) {
    Json::Value root = Json::Value(Json::arrayValue);
    for (auto& i : t) {
        root.append(serialize(i));
    }
    return pico::Json2Str(root);
}



template<typename T>
void deserialize(const std::string& s, T& t,
                 typename std::enable_if<std::is_arithmetic<T>::value || pico::TypeTraits<T>::value,
                                         int>::type = 0) {
    t = boost::lexical_cast<T>(s);
}

template<typename T>
void deserialize(const std::string& s, T& t,
                 typename std::enable_if<
                     !std::is_arithmetic<T>::value && !pico::TypeTraits<T>::value, int>::type = 0) {
    t.decode(s);
}

template<typename T>
void deserialize(const std::string& s, std::vector<T>& t) {
    Json::Value root;
    pico::Str2Json(s, root);
    for (auto& i : root) {
        T tmp;
        deserialize(i.asString(), tmp);
        t.push_back(tmp);
    }
}

template<typename T>
void deserialize(const std::string& s, std::list<T>& t) {
    Json::Value root;
    pico::Str2Json(s, root);
    for (auto& i : root) {
        T tmp;
        deserialize(i.asString(), tmp);
        t.push_back(tmp);
    }
}

template<typename K, typename V>
void deserialize(const std::string& s, std::map<K, V>& t) {
    Json::Value root;
    pico::Str2Json(s, root);
    for (auto& i : root.getMemberNames()) {
        K key;
        deserialize(i, key);
        V value;
        deserialize(root[i].asString(), value);
        t[key] = value;
    }
}

template<typename K, typename V>
void deserialize(const std::string& s, std::unordered_map<K, V>& t) {
    Json::Value root;
    pico::Str2Json(s, root);
    for (auto& i : root.getMemberNames()) {
        K key;
        deserialize(i, key);
        V value;
        deserialize(root[i].asString(), value);
        t[key] = value;
    }
}

template<typename T>
void deserialize(const std::string& s, std::set<T>& t) {
    Json::Value root;
    pico::Str2Json(s, root);
    for (auto& i : root) {
        T tmp;
        deserialize(i.asString(), tmp);
        t.insert(tmp);
    }
}

template<typename T>
void deserialize(const std::string& s, std::unordered_set<T>& t) {
    Json::Value root;
    pico::Str2Json(s, root);
    for (auto& i : root) {
        T tmp;
        deserialize(i.asString(), tmp);
        t.insert(tmp);
    }
}

template<typename T>
T deserialize(const std::string& s) {
    T t;
    deserialize(s, t);
    return t;
}


// template<typename T>
// std::vector<T> deserialize(const std::string& s) {
//     Json::Value root;
//     pico::Str2Json(s, root);
//     std::vector<T> vec;
//     for (auto& i : root) {
//         vec.push_back(deserialize<T>(i.asString()));
//     }
//     return vec;
// }

// template<typename T>
// std::list<T> deserialize(const std::string& s) {
//     Json::Value root;
//     pico::Str2Json(s, root);
//     std::list<T> vec;
//     for (auto& i : root) {
//         vec.push_back(deserialize<T>(i.asString()));
//     }
//     return vec;
// }

// template<typename K, typename V>
// std::map<K, V> deserialize(const std::string& s) {
//     Json::Value root;
//     pico::Str2Json(s, root);
//     std::map<K, V> vec;
//     for (auto& i : root.getMemberNames()) {
//         vec[deserialize<K>(i)] = deserialize<V>(root[i].asString());
//     }
//     return vec;
// }

// template<typename K, typename V>
// std::unordered_map<K, V> deserialize(const std::string& s) {
//     Json::Value root;
//     pico::Str2Json(s, root);
//     std::unordered_map<K, V> vec;
//     for (auto& i : root.getMemberNames()) {
//         vec[deserialize<K>(i)] = deserialize<V>(root[i].asString());
//     }
//     return vec;
// }

// template<typename T>
// std::set<T> deserialize(const std::string& s) {
//     Json::Value root;
//     pico::Str2Json(s, root);
//     std::set<T> vec;
//     for (auto& i : root) {
//         vec.insert(deserialize<T>(i.asString()));
//     }
//     return vec;
// }

// template<typename T>
// std::unordered_set<T> deserialize(const std::string& s) {
//     Json::Value root;
//     pico::Str2Json(s, root);
//     std::unordered_set<T> vec;
//     for (auto& i : root) {
//         vec.insert(deserialize<T>(i.asString()));
//     }
//     return vec;
// }


}   // namespace pico

#endif   // __PICO_SERIALIZE_H__