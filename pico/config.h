#ifndef __PICO_CONFIG_H__
#define __PICO_CONFIG_H__

#include <yaml-cpp/yaml.h>

#include <boost/lexical_cast.hpp>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "logging.h"
#include "mutex.h"

namespace pico {

class ConfigVarBase
{
private:
public:
    typedef std::shared_ptr<ConfigVarBase> Ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "")
        : m_name(name)
        , m_description(description) {}
    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    virtual std::string toString() = 0;

    virtual bool parse(const std::string& val) = 0;

    virtual std::string getTypeName() const = 0;

protected:
    std::string m_name;
    std::string m_description;
};

template<class F, class T>
class LexicalCast
{
public:
    T operator()(const F& val) { return boost::lexical_cast<T>(val); }
};

template<class T>
class LexicalCast<std::string, std::vector<T>>
{
public:
    std::vector<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::vector<T>, std::string>
{
public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
template<class T>
class LexicalCast<std::string, std::list<T>>
{
public:
    std::list<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> l;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            l.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return l;
    }
};

template<class T>
class LexicalCast<std::list<T>, std::string>
{
public:
    std::string operator()(const std::list<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        std::transform(v.begin(), v.end(), std::back_inserter(node), [](const T& i) {
            return YAML::Load(LexicalCast<T, std::string>()(i));
        });
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
template<class T>
class LexicalCast<std::string, std::set<T>>
{
public:
    std::set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> s;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            s.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return s;
    }
};

template<class T>
class LexicalCast<std::set<T>, std::string>
{
public:
    std::string operator()(const std::set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::unordered_set<T>>
{
public:
    std::unordered_set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> us;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            us.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return us;
    }
};

template<class T>
class LexicalCast<std::unordered_set<T>, std::string>
{
public:
    std::string operator()(const std::set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::map<std::string, T>>
{
public:
    std::map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for (auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>>
{
public:
    std::unordered_map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for (auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T, class FromStr = LexicalCast<std::string, T>,
         class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase
{
public:
    typedef RWMutex RWMutexType;
    typedef std::shared_ptr<ConfigVar> Ptr;

    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        : ConfigVarBase(name, description)
        , m_val(default_value) {}

    std::string toString() override {
        try {
            // return boost::lexical_cast<std::string>(m_val);
            RWMutexType::ReadLock lock(m_mutex);
            return ToStr()(m_val);
        }
        catch (std::exception& e) {
            LOG_ERROR("ConfigVar::toString() exception: %s", e.what());
        }
        return "";
    }

    bool parse(const std::string& val) override {
        try {
            // m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
        }
        catch (const std::exception& e) {
            LOG_ERROR("ConfigVar::parse() exception: %s", e.what());
            return false;
        }
        return false;
    }
    const T getValue() {
        RWMutexType::ReadLock lock(m_mutex);
        return m_val;
    }
    void setValue(const T& val) {
        RWMutexType::WriteLock lock(m_mutex);
        m_val = val;
    }
    std::string getTypeName() const override { return typeid(T).name(); }


private:
    T m_val;
    RWMutexType m_mutex;
};

class Config
{
public:
    typedef std::unordered_map<std::string, ConfigVarBase::Ptr> ConfigVarMap;

    typedef RWMutex RWMutexType;

    template<class T>
    static typename ConfigVar<T>::Ptr Lookup(const std::string& name, const T& default_value,
                                             const std::string& description = "") {
        RWMutexType::ReadLock lock(getMutex());
        auto it = getDatas().find(name);
        if (it != getDatas().end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if (tmp) {
                // LOG_INFO("Config::Lookup() %s found", name.c_str());
                return tmp;
            }
            else {
                LOG_ERROR("Config::Lookup() %s found, but type is not match", name.c_str());
                return nullptr;
            }
        }

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
            LOG_ERROR("Config::Lookup() %s not found", name.c_str());
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::Ptr v(new ConfigVar<T>(name, default_value, description));

        getDatas()[name] = v;

        return v;
    }
    template<class T>
    static typename ConfigVar<T>::Ptr Lookup(const std::string& name) {
        auto it = getDatas().find(name);
        if (it == getDatas().end()) {
            return nullptr;
        }

        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);

    static void LoadFromFile(const std::string& filename);

    static void LoadFromConfDir(const std::string& conf_dir);

    static ConfigVarBase::Ptr LookupBase(const std::string& name);

    static void Visit(std::function<void(ConfigVarBase::Ptr)> cb) {
        RWMutexType::ReadLock lock(getMutex());
        ConfigVarMap& m = getDatas();
        for (auto it = m.begin(); it != m.end(); ++it) {
            cb(it->second);
        }
    }

private:
    // static ConfigVarMap s_datas;

    static ConfigVarMap& getDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }

    static RWMutexType& getMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }
};

}   // namespace pico

#endif
