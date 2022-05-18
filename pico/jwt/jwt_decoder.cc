#include "jwt_decoder.h"

#include "../util.h"

namespace pico {

JWTDecoder::JWTDecoder(const std::string& jwt)
    : m_jwt(jwt) {
    std::vector<std::string> parts;
    split(jwt, parts, ".");
    if (parts.size() != 3) { throw std::runtime_error("invalid jwt"); }
    if (!Str2Json(base64_decode(parts[0].data(), parts[0].size()), m_header) ||
        !Str2Json(base64_decode(parts[1].data(), parts[1].size()), m_payload)) {
        throw std::runtime_error("invalid jwt");
    }

    m_signature = base64_decode(parts[2].data(), parts[2].size());
}

std::string JWTDecoder::getHeader() {
    return Json2Str(m_header);
}

std::string JWTDecoder::getPayload() {
    return Json2Str(m_payload);
}

std::string JWTDecoder::getSignature() {
    return m_signature;
}

std::string JWTDecoder::getToken() {
    return m_jwt;
}

std::string JWTDecoder::getAlgorithm() {
    return getHeaderValueAs<std::string>("alg", "");
}
std::string JWTDecoder::getType() {
    return getHeaderValueAs<std::string>("typ", "");
}

std::string JWTDecoder::getKeyId() {
    return getPayloadValueAs<std::string>("kid", "");
}

std::string JWTDecoder::getIssuer() {
    return getPayloadValueAs<std::string>("iss", "");
}

std::string JWTDecoder::getSubject() {
    return getPayloadValueAs<std::string>("sub", "");
}

Date JWTDecoder::getExpiration(const std::string format) {
    return Date(getPayloadValueAs<std::string>("exp", ""), format);
}

Date JWTDecoder::getNotBefore(const std::string format) {
    return Date(getPayloadValueAs<std::string>("nbf", ""), format);
}

Date JWTDecoder::getIssuedAt(const std::string format) {
    return Date(getPayloadValueAs<std::string>("iat", ""), format);
}

std::string JWTDecoder::getJwtId() {
    return getPayloadValueAs<std::string>("jti", "");
}

std::vector<std::string> JWTDecoder::getAudience() {
    return getPayloadValueAsArray<std::string>("aud", std::vector<std::string>());
}

}   // namespace pico