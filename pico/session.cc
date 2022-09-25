#include "session.h"

#include "config.h"
#include "util.h"

namespace pico {

static ConfigVar<uint64_t>::Ptr session_timeout =
    Config::Lookup<uint64_t>("other.session.timeout", 3600, "session timeout");

namespace tools {

typedef HttpSession Value;


HttpSession& HttpSession::remove(const Key& key) {
    Lock::WriteLock lock(m_mutex);
    m_data.erase(key);
    return *this;
}

bool HttpSession::has(const Key& key) {
    Lock::ReadLock lock(m_mutex);
    return m_data.find(key) != m_data.end();
}

Value::Ptr SessionManager::get(const Key& key) {
    clearExpired();
    Lock::ReadLock lock(m_mutex);
    auto it = get_data().find(key);
    if (it == get_data().end()) {
        return nullptr;
    }
    return it->second;
}

void SessionManager::set(const Key& key, const Value::Ptr& value) {
    Lock::WriteLock lock(m_mutex);
    get_data()[key] = value;
}

Value::Ptr SessionManager::create(const Key& key) {
    Lock::WriteLock lock(m_mutex);
    auto it = get_data().find(key);
    if (it != get_data().end()) {
        return it->second;
    }
    auto value = std::make_shared<Value>();
    get_data()[key] = value;
    return value;
}

void SessionManager::remove(const Key& key) {
    Lock::WriteLock lock(m_mutex);
    get_data().erase(key);
}

bool SessionManager::has(const Key& key) {
    Lock::ReadLock lock(m_mutex);
    return get_data().find(key) != get_data().end();
}

void SessionManager::clear() {
    Lock::WriteLock lock(m_mutex);
    get_data().clear();
}

void SessionManager::clearExpired() {
    Lock::WriteLock lock(m_mutex);
    for (auto it = get_data().begin(); it != get_data().end();) {
        if (it->second->getLastAccessTime() + session_timeout->getValue() <
            (uint64_t)time(nullptr)) {
            it = get_data().erase(it);
        }
        else {
            ++it;
        }
    }
}

}   // namespace tools

}   // namespace pico
