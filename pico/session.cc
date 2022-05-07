#include "session.h"

#include "config.h"
#include "macro.h"


namespace pico {

ConfigVar<uint64_t>::Ptr session_timeout =
    Config::Lookup<uint64_t>(CONF_ROOT + std::string("session.timeout"), 3600, "session_timeout");

namespace tools {

typedef SessionData Value;


SessionData& SessionData::remove(const Key& key) {
    Lock::WriteLock lock(m_mutex);
    m_data.removeMember(key);
    return *this;
}

bool SessionData::has(const Key& key) {
    Lock::ReadLock lock(m_mutex);
    return m_data.isMember(key);
}

Value::Ptr SessionDataManager::get(const Key& key) {
    clearExpired();
    Lock::ReadLock lock(m_mutex);
    auto it = m_datas.find(key);
    if (it == m_datas.end()) { return nullptr; }
    return it->second;
}

void SessionDataManager::set(const Key& key, const Value::Ptr& value) {
    Lock::WriteLock lock(m_mutex);
    m_datas[key] = value;
}

Value::Ptr SessionDataManager::create(const Key& key) {
    Lock::WriteLock lock(m_mutex);
    auto it = m_datas.find(key);
    if (it != m_datas.end()) { return it->second; }
    auto value = std::make_shared<Value>();
    m_datas[key] = value;
    return value;
}

void SessionDataManager::remove(const Key& key) {
    Lock::WriteLock lock(m_mutex);
    m_datas.erase(key);
}

bool SessionDataManager::has(const Key& key) {
    Lock::ReadLock lock(m_mutex);
    return m_datas.find(key) != m_datas.end();
}

void SessionDataManager::clear() {
    Lock::WriteLock lock(m_mutex);
    m_datas.clear();
}

void SessionDataManager::clearExpired() {
    Lock::WriteLock lock(m_mutex);
    for (auto it = m_datas.begin(); it != m_datas.end();) {
        if (it->second->getLastAccessTime() + session_timeout->getValue() <
            (uint64_t)time(nullptr)) {
            it = m_datas.erase(it);
        }
        else {
            ++it;
        }
    }
}

}   // namespace tools

bool Session::hasSession(const request& req) {
    return req->get_cookie(COOKIE_KEY, "") != "";
}


std::string Session::randomStr(unsigned int length) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::string source;

    for (unsigned i = 0, amount = length / RND_ALPHABET.size() + 1; i < amount; ++i) {
        source += RND_ALPHABET;
    }

    std::shuffle(source.begin(), source.end(), gen);
    return source.substr(0, length);
}

std::string Session::stripCookie(const std::string& cookie) {
    std::istringstream ss{cookie};
    std::string ret;
    std::getline(ss, ret, ';');
    return ret;
}


std::string Session::getCookie(const request& req) {
    return req->get_cookie(COOKIE_KEY, "");
}

std::string Session::craftExpiresTimestamp() {
    return std::to_string(time(nullptr) + session_timeout->getValue());
}

std::string Session::craftSecureCookieString(const std::string& sid) {
    static const char* fmt = "%s; expires=%s; path=/; HttpOnly";
    return boost::str(boost::format(fmt) % sid % craftExpiresTimestamp());
}

std::string Session::makeNewSession() {
    std::string sid;
    {
        Lock::Lock lock(m_mutex);
        do { } while (Manager.has(sid = randomStr(SID_LENGTH))); }

    Lock::Lock lock(m_mutex);
    Manager.create(sid);

    return craftSecureCookieString(sid);
}

bool Session::isValidSession(const std::string& sid) {
    Lock::Lock lock(m_mutex);
    return Manager.has(stripCookie(sid));
}



tools::SessionData::Ptr Session::getRequestSession(const request& req, response& resp,
                                                   const bool& create) {
    auto createCoookie = [&] {
        if (!create) { return std::string(); }
        auto cookie = this->makeNewSession();
        resp->set_cookie(COOKIE_KEY, cookie);
        return cookie;
    };

    auto sessionCookie = stripCookie([&] {
        if (!this->hasSession(req)) { return createCoookie(); }

        return [&] {
            auto cookie = this->getCookie(req);
            if (!this->isValidSession(cookie)) { return createCoookie(); }
            return cookie;
        }();
    }());

    auto session = Manager.get(sessionCookie);
    if (session == nullptr) {
        if (create) { session = Manager.create(sessionCookie); }
        else {
            return nullptr;
        }
    }
    session->setLastAccessTime(time(nullptr));
    return session;
}
}   // namespace pico
