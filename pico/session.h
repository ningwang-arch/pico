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

#include "http/http.h"
#include "mutex.h"
#include "singleton.h"

namespace pico {

namespace tools {

class SessionData
{
public:
    typedef std::string Key;
    typedef Json::Value Value;
    typedef Json::Value Data;

    typedef RWMutex Lock;

    typedef std::shared_ptr<SessionData> Ptr;

    SessionData() = default;
    SessionData(const SessionData&) = default;
    SessionData(SessionData&&) = default;

    template<class T>
    SessionData& set(const Key& key, const T& value) {
        Lock::WriteLock lock(m_mutex);
        m_data[key] = value;
        return *this;
    }

    template<class T>
    T get(const Key& key, const T& def = T()) {
        Lock::ReadLock lock(m_mutex);
        if (m_data.isMember(key)) { return m_data[key].as<T>(); }
        return def;
    }

    SessionData& remove(const Key& key);

    bool has(const Key& key);

    const uint64_t& getLastAccessTime() const { return m_last_access; }
    void setLastAccessTime(const uint64_t& last_access) { m_last_access = last_access; }

private:
    Data m_data = {};
    uint64_t m_last_access = 0;
    Lock m_mutex;
};

class SessionDataManager
{
public:
    typedef std::string Key;
    typedef SessionData Value;
    typedef RWMutex Lock;
    typedef std::shared_ptr<SessionDataManager> Ptr;
    typedef std::unordered_map<Key, Value::Ptr> map;


    static SessionDataManager& instance() {
        static SessionDataManager instance;
        return instance;
    }

    Value::Ptr get(const Key& key);
    void set(const Key& key, const Value::Ptr& value);
    Value::Ptr create(const Key& key);
    void remove(const Key& key);

    bool has(const Key& key);

    void clear();

    void clearExpired();

    Value::Ptr operator[](const Key& key) { return get(key); }


private:
    SessionDataManager() = default;
    map m_datas;
    Lock m_mutex;
};
}   // namespace tools

using request = pico::HttpRequest::Ptr;
using response = pico::HttpResponse::Ptr;

class Session
{
private:
#define Manager tools::SessionDataManager::instance()

    typedef Mutex Lock;

    static constexpr unsigned SESSION_DURATION_DAYS = 14;
    static constexpr unsigned SID_LENGTH = 128;

    const std::string COOKIE_KEY = "SESS_ID";
    const std::string RND_ALPHABET = "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijlmnpqrstuvwxyz";
    const char* COOKIE_DATE_FORMAT = "%a, %d %b %Y %H:%M:%S %z";

private:
    std::string randomStr(unsigned int length);

    std::string stripCookie(const std::string& cookie);

    std::string getCookie(const request& req);

    std::string makeNewSession();

    std::string craftExpiresTimestamp();

    std::string craftSecureCookieString(const std::string& sid);

    bool isValidSession(const std::string& sid);

public:
    bool hasSession(const request& req);


    tools::SessionData::Ptr getRequestSession(const request& req, response& resp,
                                              const bool& create = true);

private:
    Mutex m_mutex;
};

typedef Singleton<Session> SessionManager;


}   // namespace pico

#endif