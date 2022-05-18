#include "jwt_creator.h"

#include <iostream>

namespace pico {

using builder = JWTCreator::Builder;

builder::Ptr builder::withHeader(const Json::Value& value) {
    if (!value.isNull()) {
        auto members = value.getMemberNames();
        for (auto member : members) {
            if (value[member].isNull()) { builder::m_header.removeMember(member); }
            else {
                builder::m_header[member] = value[member];
            }
        }
    }
    return builder::shared_from_this();
}

builder::Ptr builder::withKeyId(const std::string& keyId) {
    builder::m_header["kid"] = keyId;
    return builder::shared_from_this();
}

builder::Ptr builder::withIssuer(const std::string& issuer) {
    builder::addClaim("iss", issuer);
    return builder::shared_from_this();
}

builder::Ptr builder::withSubject(const std::string& subject) {
    builder::addClaim("sub", subject);
    return builder::shared_from_this();
}

builder::Ptr builder::withAudience(const std::vector<std::string>& audience) {
    Json::Value value = Json::arrayValue;
    for (auto aud : audience) { value.append(aud); }
    builder::addClaim("aud", value);
    return builder::shared_from_this();
}
builder::Ptr builder::withExpiresAt(Date expiresAt, const std::string format) {
    expiresAt.setFomat(format);
    builder::addClaim("exp", expiresAt.to_string());
    return builder::shared_from_this();
}
builder::Ptr builder::withNotBefore(Date notBefore, const std::string format) {
    notBefore.setFomat(format);
    builder::addClaim("nbf", notBefore.to_string());
    return builder::shared_from_this();
}
builder::Ptr builder::withIssuedAt(Date issuedAt, const std::string format) {
    issuedAt.setFomat(format);
    builder::addClaim("iat", issuedAt.to_string());
    return builder::shared_from_this();
}
builder::Ptr builder::withJWTId(const std::string& jwtId) {
    builder::addClaim("jti", jwtId);
    return builder::shared_from_this();
}

builder::Ptr builder::withPayload(const Json::Value& value) {
    if (!value.isNull()) {
        auto members = value.getMemberNames();
        for (auto member : members) {
            if (value[member].isNull()) { builder::m_payload.removeMember(member); }
            else {
                builder::m_payload[member] = value[member];
            }
        }
    }
    return builder::shared_from_this();
}

std::string builder::sign(Algorithm::Ptr algorithm) {
    if (!algorithm) { throw std::runtime_error("algorithm is null"); }
    else {
        builder::m_header["alg"] = algorithm->getName();
        if (!builder::m_header.isMember("typ")) { builder::m_header["typ"] = "JWT"; }

        return std::make_shared<JWTCreator>(algorithm, builder::m_header, builder::m_payload)
            ->sign();
    }
    return "";
}

JWTCreator::JWTCreator(const Algorithm::Ptr& algorithm, Json::Value header, Json::Value payload) {
    assert(algorithm);
    m_algorithm = algorithm;
    m_header = Json2Str(header);
    m_payload = Json2Str(payload);
}

std::string JWTCreator::sign() {
    std::string signature = m_algorithm->sign(m_header, m_payload);
    std::string header_base64 = base64_encode(m_header.c_str(), m_header.size());
    std::string payload_base64 = base64_encode(m_payload.data(), m_payload.size());
    std::string signature_base64 = base64_encode(signature.c_str(), signature.size());
    return header_base64 + "." + payload_base64 + "." + signature_base64;
}

void builder::addClaim(std::string name, Json::Value value) {
    if (value.isNull()) { builder::m_payload.removeMember(name); }
    else {
        builder::m_payload[name] = value;
    }
}

}   // namespace pico