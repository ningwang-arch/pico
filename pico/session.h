#ifndef __PICO_SESSION_H__
#define __PICO_SESSION_H__

#include <algorithm>
#include <boost/format.hpp>
#include <functional>
#include <json/json.h>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>

#include "mutex.h"
#include "redis.hpp"
#include "serialize.hpp"
#include "singleton.h"

namespace pico {

namespace tools {

class HttpSession
{
public:
    typedef std::string Key;
    typedef Json::Value Value;
    typedef std::unordered_map<std::string, std::string> Data;

    typedef RWMutex Lock;

    typedef std::shared_ptr<HttpSession> Ptr;

    HttpSession() = default;
    HttpSession(const HttpSession&) = default;
    HttpSession(HttpSession&&) = default;

    template<class T>
    HttpSession& set(const Key& key, const T& value) {
        Lock::WriteLock lock(m_mutex);
        setLastAccessTime((uint64_t)time(nullptr));
        m_data[key] = serialize(value);
        return *this;
    }

    template<class T>
    T get(const Key& key, const T& def = T()) {
        Lock::ReadLock lock(m_mutex);
        setLastAccessTime((uint64_t)time(nullptr));
        if (m_data.find(key) == m_data.end()) {
            return def;
        }
        return deserialize<T>(m_data[key]);
    }

    HttpSession& remove(const Key& key);

    bool has(const Key& key);

    const uint64_t& getLastAccessTime() const { return m_last_access; }
    void setLastAccessTime(const uint64_t& last_access) { m_last_access = last_access; }

    void setData(const Data& data) { m_data = data; }

    Data getData() const { return m_data; }

private:
    Data m_data = {};
    uint64_t m_last_access = (uint64_t)time(nullptr);
    Lock m_mutex;
};

class SessionManager : public pico::Singleton<SessionManager>
{
public:
    typedef std::string Key;
    typedef HttpSession Value;
    typedef RWMutex Lock;
    typedef std::unordered_map<Key, Value::Ptr> map;

    Value::Ptr get(const Key& key);
    void set(const Key& key, const Value::Ptr& value);
    Value::Ptr create(const Key& key);
    void remove(const Key& key);

    bool has(const Key& key);

    void clear();

    void clearExpired();


private:
    static map& get_data() {
        static map m_datas;
        return m_datas;
    }
    Lock m_mutex;
};
}   // namespace tools

}   // namespace pico

#endif