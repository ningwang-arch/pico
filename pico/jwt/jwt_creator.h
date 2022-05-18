#ifndef __PICO_JWT_CREATOR_H__
#define __PICO_JWT_CREATOR_H__

#include <assert.h>
#include <iostream>
#include <json/json.h>
#include <memory>
#include <string>

#include "../date.h"
#include "../util.h"
#include "algorithm.h"

namespace pico {

class JWTCreator
{
public:
    typedef std::shared_ptr<JWTCreator> Ptr;
    class Builder;

    JWTCreator(const Algorithm::Ptr& algorithm, Json::Value header, Json::Value payload);

    std::string sign();

    static std::shared_ptr<Builder> init() { return std::make_shared<Builder>(); }


    class Builder : public std::enable_shared_from_this<Builder>
    {
    public:
        typedef std::shared_ptr<Builder> Ptr;

        Builder(){};

        Builder::Ptr withHeader(const Json::Value& vaule);
        Builder::Ptr withKeyId(const std::string& keyId);

        Builder::Ptr withIssuer(const std::string& issuer);
        Builder::Ptr withSubject(const std::string& subject);
        Builder::Ptr withAudience(const std::vector<std::string>& audience);
        Builder::Ptr withExpiresAt(Date expiresAt, const std::string format = "%Y-%m-%d %H:%M:%S");
        Builder::Ptr withNotBefore(Date notBefore, const std::string format = "%Y-%m-%d %H:%M:%S");
        Builder::Ptr withIssuedAt(Date issuedAt, const std::string format = "%Y-%m-%d %H:%M:%S");
        Builder::Ptr withJWTId(const std::string& jwtId);

        template<typename T>
        Builder::Ptr withClaim(std::string name, T value) {
            assert(!name.empty());
            m_payload[name] = value;
            return shared_from_this();
        }

        template<typename T>
        Builder::Ptr withClaim(std::string name, std::vector<T> value) {
            assert(!name.empty());
            Json::Value array = Json::arrayValue;
            for (auto& i : value) { array.append(i); }
            m_payload[name] = array;
            return shared_from_this();
        }

        Builder::Ptr withPayload(const Json::Value& value);

        std::string sign(Algorithm::Ptr algorithm);

        std::string to_string() {
            return m_header.toStyledString() + "." + m_payload.toStyledString();
        }

    private:
        void addClaim(std::string name, Json::Value value);

    private:
        Json::Value m_header;
        Json::Value m_payload;
    };


private:
    Algorithm::Ptr m_algorithm;
    std::string m_header;
    std::string m_payload;
};

}   // namespace pico

#endif