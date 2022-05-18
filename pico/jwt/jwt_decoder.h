#ifndef __PICO_JWT_DECODER_H__
#define __PICO_JWT_DECODER_H__

#include <iostream>
#include <json/json.h>
#include <memory>
#include <string>

#include "../date.h"

namespace pico {
class JWTDecoder
{
public:
    typedef std::shared_ptr<JWTDecoder> Ptr;

    JWTDecoder(const std::string& jwt);


    std::string getHeader();
    std::string getPayload();
    std::string getSignature();
    std::string getToken();

    std::string getAlgorithm();
    std::string getType();

    template<typename T>
    T getHeaderValueAs(const std::string& key, const T& default_value = T()) {
        if (m_header.isNull()) { return default_value; }
        if (m_header.isMember(key)) { return m_header[key].as<T>(); }
        return default_value;
    }

    template<typename T>
    std::vector<T> getHeaderValueAsArray(const std::string& key,
                                         const std::vector<T>& default_value = std::vector<T>()) {
        if (m_header.isNull()) { return default_value; }
        if (m_header.isMember(key)) {
            std::vector<T> ret;
            for (auto& i : m_header[key]) { ret.push_back(i.as<T>()); }
            return ret;
        }
        return default_value;
    }


    std::string getKeyId();
    std::string getIssuer();
    std::string getSubject();
    std::vector<std::string> getAudience();
    Date getExpiration(const std::string format = "%Y-%m-%d %H:%M:%S");
    Date getNotBefore(const std::string format = "%Y-%m-%d %H:%M:%S");
    Date getIssuedAt(const std::string format = "%Y-%m-%d %H:%M:%S");
    std::string getJwtId();

    Json::Value getClaim(const std::string& name) {
        if (m_payload.isNull()) { return Json::Value(); }
        if (m_payload.isMember(name)) { return m_payload[name]; }
        return Json::Value();
    }

    template<typename T>
    T getPayloadValueAs(const std::string& key, const T& default_value = T()) {
        if (m_payload.isNull()) { return default_value; }
        if (m_payload.isMember(key)) { return m_payload[key].as<T>(); }
        return default_value;
    }

    template<typename T>
    std::vector<T> getPayloadValueAsArray(const std::string& key,
                                          const std::vector<T>& default_value = std::vector<T>()) {
        if (m_payload.isNull()) { return default_value; }
        if (m_payload.isMember(key)) {
            std::vector<T> ret;
            for (auto& i : m_payload[key]) { ret.push_back(i.as<T>()); }
            return ret;
        }
        return default_value;
    }

    std::string to_string() {
        return m_header.toStyledString() + "." + m_payload.toStyledString() + "." + m_signature;
    }

private:
    std::string m_jwt;
    Json::Value m_header;
    Json::Value m_payload;
    std::string m_signature;
};
}   // namespace pico

#endif