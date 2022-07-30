#include "http.h"

#include <iostream>
#include <sstream>

#include "../util.h"
#include "pico/mustache.h"

namespace pico {
HttpMethod http_method_from_string(const std::string& method) {
#define XX(num, name, string)                   \
    if (strcmp(#string, method.c_str()) == 0) { \
        return HttpMethod::name;                \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}   // namespace pico

const char* http_method_to_string(HttpMethod method) {
#define XX(num, name, string)         \
    if (method == HttpMethod::name) { \
        return #string;               \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return "INVALID_METHOD";
}


const char* http_status_to_string(HttpStatus status) {
    switch (status) {
#define XX(code, name, msg) \
    case HttpStatus::name: return #msg;
        HTTP_STATUS_MAP(XX);
#undef XX
    default: return "INVALID_STATUS";
    }
}

HttpRequest::HttpRequest(std::string version, bool is_close)
    : m_is_close(is_close)
    , m_websocket(false)
    , m_method(HttpMethod::GET)
    , m_path("/")
    , m_version(version)
    , m_parserParamFlag(0) {}

void HttpRequest::init() {
    std::string conn = get_header("connection");
    if (!conn.empty()) {
        if (strcasecmp(conn.c_str(), "keep-alive") == 0) {
            m_is_close = false;
        }
        else {
            m_is_close = true;
        }
    }
}

void HttpRequest::initParam() {
    initQueryParam();
    initBodyParam();
    initCookies();
}

void HttpRequest::initQueryParam() {
    if (m_parserParamFlag & 0x1) {
        return;
    }

#define PARSE_PARAM(str, m, flag, trim)                                                            \
    size_t pos = 0;                                                                                \
    do {                                                                                           \
        size_t last = pos;                                                                         \
        pos = str.find('=', pos);                                                                  \
        if (pos == std::string::npos) {                                                            \
            break;                                                                                 \
        }                                                                                          \
        size_t key = pos;                                                                          \
        pos = str.find(flag, pos);                                                                 \
        m.insert(std::make_pair(trim(str.substr(last, key - last)),                                \
                                pico::StringUtil::UrlDecode(str.substr(key + 1, pos - key - 1)))); \
        if (pos == std::string::npos) {                                                            \
            break;                                                                                 \
        }                                                                                          \
        ++pos;                                                                                     \
    } while (true);

    PARSE_PARAM(m_query, m_params, '&', );
    m_parserParamFlag |= 0x1;
}

void HttpRequest::initBodyParam() {
    if (m_parserParamFlag & 0x2) {
        return;
    }
    std::string content_type = get_header("content-type");
    if (strcasestr(content_type.c_str(), "application/x-www-form-urlencoded") == nullptr) {
        m_parserParamFlag |= 0x2;
        return;
    }
    PARSE_PARAM(m_body, m_params, '&', );
    m_parserParamFlag |= 0x2;
}

void HttpRequest::initCookies() {
    if (m_parserParamFlag & 0x4) {
        return;
    }
    std::string cookie = get_header("cookie");
    if (cookie.empty()) {
        m_parserParamFlag |= 0x4;
        return;
    }
    PARSE_PARAM(cookie, m_cookies, ';', pico::StringUtil::Trim);
    m_parserParamFlag |= 0x4;
}

std::string HttpRequest::get_header(const std::string& key, const std::string& def) {
    auto it = m_headers.find(key);
    if (it == m_headers.end()) {
        return def;
    }
    return it->second;
}

std::string HttpRequest::get_param(const std::string& key, const std::string& def) {
    initQueryParam();
    initBodyParam();
    auto it = m_params.find(key);
    if (it == m_params.end()) {
        return def;
    }
    return it->second;
}

std::string HttpRequest::get_cookie(const std::string& key, const std::string& def) {
    initCookies();
    auto it = m_cookies.find(key);
    if (it == m_cookies.end()) {
        return def;
    }
    return it->second;
}

std::string HttpRequest::get_token() {
    std::string token_str = get_header("Authorization");
    if (token_str.empty()) {
        return "";
    }
    size_t pos = token_str.find(' ');
    if (pos == std::string::npos) {
        return "";
    }
    return token_str.substr(pos + 1);
}

void HttpRequest::set_header(const std::string& key, const std::string& value) {
    m_headers[key] = value;
}

void HttpRequest::set_param(const std::string& key, const std::string& value) {
    m_params[key] = value;
}

void HttpRequest::set_cookie(const std::string& key, const std::string& value) {
    m_cookies[key] = value;
}

void HttpRequest::set_token(const std::string& token) {
    if (token.empty()) {
        m_headers.erase("Authorization");
        return;
    }
    set_header("Authorization", "Bearer " + token);
}

void HttpRequest::del_header(const std::string& key) {
    m_headers.erase(key);
}

void HttpRequest::del_param(const std::string& key) {
    m_params.erase(key);
}

void HttpRequest::del_cookie(const std::string& key) {
    m_cookies.erase(key);
}

bool HttpRequest::has_header(const std::string& key, std::string* value) {
    auto it = m_headers.find(key);
    if (it == m_headers.end()) {
        return false;
    }
    if (value != nullptr) {
        *value = it->second;
    }
    return true;
}

bool HttpRequest::has_param(const std::string& key, std::string* value) {
    initQueryParam();
    initBodyParam();
    auto it = m_params.find(key);
    if (it == m_params.end()) {
        return false;
    }
    if (value != nullptr) {
        *value = it->second;
    }
    return true;
}

bool HttpRequest::has_cookie(const std::string& key, std::string* value) {
    initCookies();
    auto it = m_cookies.find(key);
    if (it == m_cookies.end()) {
        return false;
    }
    if (value != nullptr) {
        *value = it->second;
    }
    return true;
}

std::string HttpRequest::to_string() const {
    std::stringstream ss;
    ss << http_method_to_string(m_method) << " " << m_path << (m_query.empty() ? "" : "?")
       << m_query << (m_fragment.empty() ? "" : "#") << m_fragment << " " << m_version << "\r\n";
    if (!m_websocket) {
        ss << "connection: " << (m_is_close ? "close" : "keep-alive") << "\r\n";
    }
    for (auto& i : m_headers) {
        if (!m_websocket && strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        ss << i.first << ": " << i.second << "\r\n";
    }

    if (!m_body.empty()) {
        ss << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
    }
    else {
        ss << "\r\n";
    }
    return ss.str();
}

HttpResponse::HttpResponse(std::string version, bool is_close)
    : m_version(std::move(version))
    , m_status(HttpStatus::OK)
    , m_is_close(is_close)
    , m_websocket(false) {}

std::string HttpResponse::get_header(const std::string& key, const std::string& def) {
    auto it = m_headers.find(key);
    if (it == m_headers.end()) {
        return def;
    }
    return it->second;
}

void HttpResponse::set_header(const std::string& key, const std::string& value) {
    m_headers[key] = value;
}

void HttpResponse::del_header(const std::string& key) {
    m_headers.erase(key);
}

void HttpResponse::set_redirect(const std::string& url) {
    set_header("Location", url);
    set_status(HttpStatus::FOUND);
}

void HttpResponse::set_token(const std::string& token) {
    if (token.empty()) {
        m_headers.erase("Authorization");
        return;
    }
    set_header("Authorization", "Bearer " + token);
}

void HttpResponse::set_cookie(const std::string& key, const std::string& val, time_t expired,
                              const std::string& path, const std::string& domain, bool secure) {
    std::stringstream ss;
    ss << key << "=" << val;
    if (expired != 0) {
        ss << "; expires=" << pico::Time2Str(expired);
    }
    if (!path.empty()) {
        ss << "; path=" << path;
    }
    if (!domain.empty()) {
        ss << "; domain=" << domain;
    }
    if (secure) {
        ss << "; secure";
    }
    m_cookies.push_back(ss.str());
}

void HttpResponse::write(mustache::RenderedTemplate& tpl) {
    m_body = tpl.dump();
    set_header("Content-Length", std::to_string(m_body.size()));
    set_header("Content-Type", tpl.getContentType());
}

void HttpResponse::write(const std::string& body) {
    m_body = body;
    set_header("Content-Length", std::to_string(m_body.size()));
    set_header("Content-Type", "text/plain");
}

void HttpResponse::write(const char* str) {
    m_body = str;
    set_header("Content-Length", std::to_string(m_body.size()));
    set_header("Content-Type", "text/plain");
}

void HttpResponse::write(const char* str, size_t len) {
    m_body.append(str, len);
    set_header("Content-Length", std::to_string(m_body.size()));
    set_header("Content-Type", "text/plain");
}

void HttpResponse::write(const Json::Value& json) {
    m_body = Json2Str(json);
    set_header("Content-Length", std::to_string(m_body.size()));
    set_header("Content-Type", "application/json");
}

std::string HttpResponse::to_string() const {
    std::stringstream ss;
    ss << m_version << " " << (uint32_t)m_status << " "
       << (m_reason.empty() ? http_status_to_string(m_status) : m_reason) << "\r\n";
    for (auto& i : m_headers) {
        if (!m_websocket && strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        ss << i.first << ": " << i.second << "\r\n";
    }
    for (auto& i : m_cookies) {
        ss << "Set-Cookie: " << i << "\r\n";
    }
    if (!m_websocket) {
        ss << "connection: " << (m_is_close ? "close" : "keep-alive") << "\r\n";
    }
    if (!m_body.empty()) {
        ss << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
    }
    else {
        ss << "\r\n";
    }
    return ss.str();
}

}   // namespace pico

std::ostream& operator<<(std::ostream& os, const pico::HttpRequest& req) {
    os << req.to_string();
    return os;
}

std::ostream& operator<<(std::ostream& os, const pico::HttpResponse& res) {
    os << res.to_string();
    return os;
}