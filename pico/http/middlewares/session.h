#ifndef __PICO_HTTP_MIDDLEWARES_SESSION_H__
#define __PICO_HTTP_MIDDLEWARES_SESSION_H__

#include "../../mutex.h"
#include "../http.h"
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <json/json.h>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

namespace pico {
namespace tools {
class Session
{
public:
    using Key = std::string;
    using Value = Json::Value;
    using State = Json::Value;

protected:
    State state = {};

public:
    Session() = default;
    Session(const Session&) = delete;
    Session(Session&&) = default;

    bool has(const Key& key) const { return state.isMember(key); }
    Value& get(const Key& key) {
        if (!has(key)) { state[key] = {}; }
        return state[key];
    }

    template<class T>
    Session& set(const Key& key, const T& value) {
        state[key] = value;
        return *this;
    }

    Session& remove(const Key& key) {
        state.removeMember(key);
        return *this;
    }

    const State& json() const { return state; }
};

class Sessions
{
protected:
    Sessions() {}

public:
    static Sessions& instance() {
        static Sessions instance;
        return instance;
    }

    using Key = std::string;
    using Value = std::shared_ptr<Session>;
    using map = std::unordered_map<Key, Value>;

    Sessions(const Sessions&) = default;
    Sessions(Sessions&&) = default;

protected:
    map sessions = map{};

public:
    bool has(const Key& key) const { return sessions.find(key) != sessions.end(); }

    const Value& get(const Key& key) const {
        auto it = sessions.find(key);
        if (it == sessions.end()) { throw std::runtime_error("Session not found"); }
        return it->second;
    }

    Value& get(const Key& key) {
        auto it = sessions.find(key);
        if (it == sessions.end()) { throw std::runtime_error("Session not found"); }
        return it->second;
    }

    Sessions& set(const Key& key, const Value& value) {
        sessions.emplace(key, value);
        return *this;
    }

    Sessions& newSessionFor(const Key& key) {
        return this->set(key, std::make_shared<Session>(Session{}));
    }

    Sessions& remove(const Key& key) {
        sessions.erase(key);
        return *this;
    }

    const Value& operator[](const Key& key) const { return get(key); }

    Value& operator[](const Key& key) { return get(key); }
};

}   // namespace tools

class Session
{

public:
    using request = HttpRequest::Ptr;
    using response = HttpResponse::Ptr;
    using UserSession = tools::Session;

    static constexpr unsigned SESSION_DURATION_DAYS = 14;
    static constexpr unsigned SID_LENGTH = 128;

    static const std::string COOKIE_KEY;
    static const std::string RND_ALPHABET;
    static const char* COOKIE_DATE_FORMAT;

public:
    static tools::Sessions& sessions;
    Mutex mutex;

    struct context
    {
        std::shared_ptr<UserSession> session = nullptr;
    };

    bool hasSession(request& req) { return !req->get_cookie(COOKIE_KEY, "").empty(); }

    std::string craftExpiresDate() {
        std::stringstream ss;
        boost::gregorian::date today = boost::gregorian::day_clock::local_day() +
                                       boost::gregorian::days(SESSION_DURATION_DAYS);
        ss.imbue(std::locale(ss.getloc(), new boost::posix_time::time_facet(COOKIE_DATE_FORMAT)));
        ss << today;
        return ss.str();
    }

    std::string craftSecureCookieString(const std::string id) {
        static const char* fmt = "%1%; Expires=%2%; HttpOnly;";
        return boost::str(boost::format(fmt) % id % craftExpiresDate());
    }

    std::string randomStr(unsigned length) {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        std::string source;

        for (unsigned i = 0, amount = length / RND_ALPHABET.size() + 1; i < amount; ++i) {
            source += RND_ALPHABET;
        }

        std::shuffle(source.begin(), source.end(), gen);
        return source.substr(0, length);
    }

    std::string makeNewSession(request& req, context& ctx) {
        std::string sid;
        {
            Mutex::Lock lock(mutex);
            do { } while (sessions.has(sid = randomStr(SID_LENGTH))); }

        Mutex::Lock lock(mutex);
        sessions.newSessionFor(sid);

        return craftSecureCookieString(sid);
    }

    std::string stripCookie(const std::string& cookie) {
        std::istringstream ss{cookie};
        std::string ret;
        std::getline(ss, ret, ';');
        return ret;
    }

    bool isValidSessionCookie(const std::string& cookie) {
        Mutex::Lock lock(mutex);
        return sessions.has(stripCookie(cookie));
    }

    std::string getSessionCookie(request& req) { return req->get_cookie(COOKIE_KEY, ""); }

    void before_handle(request& req, response& res, context& ctx) {
        auto createCookie = [&] {
            auto cookie = makeNewSession(req, ctx);
            cookie_to_add.emplace(COOKIE_KEY, cookie);
            return cookie;
        };

        auto sessionCookie = stripCookie([&] {
            if (!this->hasSession(req)) { return createCookie(); }

            return [&] {
                auto cookie = getSessionCookie(req);
                if (!isValidSessionCookie(cookie)) { return createCookie(); }

                return cookie;
            }();
        }());

        Mutex::Lock lock(mutex);
        ctx.session = sessions.get(sessionCookie);
    }

    void after_handle(request& req, response& res, context& ctx) {
        if (cookie_to_add.empty()) { return; }

        for (auto& cookie : cookie_to_add) { res->set_cookie(cookie.first, cookie.second); }
        cookie_to_add.clear();
    }

    std::unordered_map<std::string, std::string> cookie_to_add{};
};
const std::string Session::COOKIE_KEY = "SESS_ID";
const std::string Session::RND_ALPHABET =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijlmnpqrstuvwxyz";
const char* Session::COOKIE_DATE_FORMAT = "%a, %d %b %Y %H:%M:%S %z";

tools::Sessions& Session::sessions = tools::Sessions::instance();

}   // namespace pico



#endif