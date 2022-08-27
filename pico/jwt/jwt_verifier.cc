#include "jwt_verifier.h"

#include <iostream>

namespace pico {

std::shared_ptr<JWTVerifier::Verification> JWTVerifier::init(Algorithm::Ptr algorithm) {
    if (!algorithm) throw std::runtime_error("algorithm is null");
    return std::make_shared<JWTVerifier::Verification>(algorithm);
}

bool JWTVerifier::isNullOrEmpty(Json::Value& value) {
    if (value.isNull()) {
        return true;
    }
    for (auto& i : value) {
        if (!i.isNull()) {
            return false;
        }
    }
    return true;
}

JWTDecoder::Ptr JWTVerifier::verify(const std::string& jwt) {
    JWTDecoder::Ptr decoder = std::make_shared<JWTDecoder>(jwt);
    return verify(decoder);
}

JWTDecoder::Ptr JWTVerifier::verify(const JWTDecoder::Ptr& decoder) {
    this->verifyAlgorithm(decoder, this->m_algorithm);
    this->m_algorithm->verify(decoder);
    this->verifyClaims(decoder, this->m_claims);
    return decoder;
}

void JWTVerifier::verifyAlgorithm(JWTDecoder::Ptr decoder, Algorithm::Ptr expectedAlgorithm) {
    if (decoder->getAlgorithm().empty()) {
        throw std::runtime_error("algorithm is null");
    }
    if (expectedAlgorithm->getName() != decoder->getAlgorithm()) {
        throw VerificationException(
            "The provided Algorithm doesn't match the one defined in the JWT's Header.");
    }
}

void JWTVerifier::verifyClaims(JWTDecoder::Ptr decoder, Json::Value value) {
    if (isNullOrEmpty(value)) {
        return;
    }
    auto members = value.getMemberNames();
    for (auto member : members) {
        auto claim = value[member];
        if (claim.isNull()) {
            continue;
        }

        this->verifyClaimValues(decoder, member, claim);
    }
}

void JWTVerifier::verifyClaimValues(JWTDecoder::Ptr decoder, const std::string& name,
                                    Json::Value expectedClaim) {
    if (name == "AUDIENCE_EXACT") {
        std::vector<std::string> expectedAudiences;
        for (auto& i : expectedClaim) {
            expectedAudiences.push_back(i.asString());
        }
        this->assertValidAudience(decoder->getAudience(), expectedAudiences, true);
    }
    else if (name == "AUDIENCE_CONTAINS") {
        std::vector<std::string> expectedAudiences;
        for (auto& i : expectedClaim) {
            expectedAudiences.push_back(i.asString());
        }
        this->assertValidAudience(decoder->getAudience(), expectedAudiences, false);
    }
    else if (name == "exp") {
        this->assertValidDateClaim(decoder->getExpiration(), expectedClaim.asInt64(), true);
    }
    else if (name == "iat") {
        this->assertValidDateClaim(decoder->getIssuedAt(), expectedClaim.asInt64(), false);
    }
    else if (name == "nbf") {
        this->assertValidDateClaim(decoder->getNotBefore(), expectedClaim.asInt64(), false);
    }
    else if (name == "iss") {
        std::vector<std::string> expectedIssuers =
            decoder->getPayloadValueAsArray<std::string>("iss");
        this->assertValidIssuer(expectedClaim.asString(), expectedIssuers);
    }
    else if (name == "sub") {
        this->assertValidStringClaim(name, decoder->getSubject(), expectedClaim.asString());
    }
    else if (name == "jti") {
        this->assertValidStringClaim(name, decoder->getJwtId(), expectedClaim.asString());
    }
    else {
        this->assertValidClaim(decoder->getClaim(name), name, expectedClaim);
    }
}

void JWTVerifier::assertValidClaim(Json::Value claim, const std::string& name,
                                   Json::Value expectedClaim) {
    bool isValid = false;
    if (expectedClaim.isString()) {
        isValid = claim.asString() == expectedClaim.asString();
    }
    else if (expectedClaim.isInt64()) {
        isValid = claim.asInt64() == expectedClaim.asInt64();
    }
    else if (expectedClaim.isBool()) {
        isValid = claim.asBool() == expectedClaim.asBool();
    }
    else if (expectedClaim.isDouble()) {
        isValid = claim.asDouble() == expectedClaim.asDouble();
    }
    else if (expectedClaim.isArray()) {
        isValid = true;
        // TODO: implement
        Json::Value tmp = Json::arrayValue;
        if (claim.isArray()) {
            for (auto& i : claim) {
                tmp.append(i);
            }
        }
        else {
            tmp.append(claim);
        }
        // if tmp contains all elements of expectedClaim, isValid = true
        for (auto i : expectedClaim) {
            if (std::find(tmp.begin(), tmp.end(), i) == tmp.end()) {
                isValid = false;
                break;
            }
        }
    }

    if (!isValid) {
        throw VerificationException(
            "The provided Claim doesn't match the one defined in the JWT's Payload.");
    }
}

void JWTVerifier::assertValidStringClaim(const std::string& name, const std::string& claim,
                                         const std::string& expectedClaim) {
    if (claim != expectedClaim) {
        throw VerificationException("The Claim " + name + " value doesn't match the required one.");
    }
}

void JWTVerifier::assertValidDateClaim(const Date& date, long leeway, bool shouldBeFuture) {
    Date today;
    if (shouldBeFuture) {
        this->assertDateIsFuture(date, leeway, today);
    }
    else {
        this->assertDateIsPast(date, leeway, today);
    }
}

void JWTVerifier::assertDateIsFuture(Date date, long leeway, Date today) {
    today = today - leeway;
    if (today.isAfter(date)) {
        throw VerificationException("The Token has expired on " + date.to_string());
    }
}

void JWTVerifier::assertDateIsPast(Date date, long leeway, Date today) {
    today = today + leeway;
    if (today.isBefore(date)) {
        throw VerificationException("The Token can't be used before " + date.to_string());
    }
}

void JWTVerifier::assertValidAudience(std::vector<std::string> audiences,
                                      std::vector<std::string> expectedAudiences, bool exact) {
    if (audiences.empty()) {
        throw VerificationException("The Token doesn't contain an Audience.");
    }

    if (exact) {
        if (audiences.size() != expectedAudiences.size()) {
            throw VerificationException("The Token doesn't contain the expected Audience.");
        }

        for (auto audience : audiences) {
            if (std::find(expectedAudiences.begin(), expectedAudiences.end(), audience) ==
                expectedAudiences.end()) {
                throw VerificationException("The Token doesn't contain the expected Audience.");
            }
        }
    }
    else {
        for (auto expectedAudience : expectedAudiences) {
            if (std::find(audiences.begin(), audiences.end(), expectedAudience) ==
                audiences.end()) {
                throw VerificationException("The Token doesn't contain the expected Audience.");
            }
        }
    }
}

void JWTVerifier::assertValidIssuer(std::string issuer, std::vector<std::string> expectedIssuers) {
    if (issuer.empty() || std::find(expectedIssuers.begin(), expectedIssuers.end(), issuer) ==
                              expectedIssuers.end()) {
        throw VerificationException("The Token doesn't contain the expected Issuer.");
    }
}

using verif = JWTVerifier::Verification;

verif::Ptr verif::withIssuer(const std::vector<std::string>& issuers) {
    Json::Value value = Json::arrayValue;
    for (auto& issuer : issuers) {
        value.append(issuer);
    }
    this->requireClaim("iss", value);
    return shared_from_this();
}

verif::Ptr verif::withSubject(const std::string& subject) {
    this->requireClaim("iss", subject);
    return shared_from_this();
}

verif::Ptr verif::withAudience(const std::vector<std::string>& audiences) {
    this->m_claims.removeMember("AUDIENCE_CONTAINS");
    Json::Value value = Json::arrayValue;
    for (auto& audience : audiences) {
        value.append(audience);
    }
    this->requireClaim("AUDIENCE_EXACT", value);
    return shared_from_this();
}

verif::Ptr verif::withAnyOfAudience(const std::vector<std::string>& audiences) {
    this->m_claims.removeMember("AUDIENCE_EXACT");
    Json::Value value = Json::arrayValue;
    for (auto& audience : audiences) {
        value.append(audience);
    }
    this->requireClaim("AUDIENCE_CONTAINS", value);
    return shared_from_this();
}

verif::Ptr verif::acceptLeeWay(long leeway) {
    if (leeway < 0L) {
        throw VerificationException("The leeway must be greater than 0");
    }
    this->m_defaultLeeway = leeway;
    return shared_from_this();
}

verif::Ptr verif::acceptExpiresAt(long leeway) {
    if (leeway < 0L) {
        throw VerificationException("The leeway must be greater than 0");
    }
    this->requireClaim("exp", leeway);
    return shared_from_this();
}

verif::Ptr verif::acceptNotBefore(long leeway) {
    if (leeway < 0L) {
        throw VerificationException("The leeway must be greater than 0");
    }
    this->requireClaim("nbf", leeway);
    return shared_from_this();
}

verif::Ptr verif::acceptIssuedAt(long leeway) {
    if (leeway < 0L) {
        throw VerificationException("The leeway must be greater than 0");
    }
    this->requireClaim("iat", leeway);
    return shared_from_this();
}

verif::Ptr verif::ignoreIssuedAt() {
    this->m_ignoreIssuedAt = true;
    return shared_from_this();
}

verif::Ptr verif::withJwtId(const std::string& jwtId) {
    this->requireClaim("jti", jwtId);
    return shared_from_this();
}

verif::Ptr verif::withClaimPresence(const std::string& claimName) {
    this->requireClaim(claimName, Json::Value());
    return shared_from_this();
}


JWTVerifier::Ptr verif::build() {
    return this->build(Date());
}

JWTVerifier::Ptr verif::build(const Date& date) {
    this->addLeewayToDateClaims();
    return std::make_shared<JWTVerifier>(this->m_algorithm, this->m_claims, date);
}

void verif::addLeewayToDateClaims() {
    if (!this->m_claims.isMember("exp")) {
        this->m_claims["exp"] = this->m_defaultLeeway;
    }

    if (!this->m_claims.isMember("nbf")) {
        this->m_claims["nbf"] = this->m_defaultLeeway;
    }

    if (this->m_ignoreIssuedAt) {
        this->m_claims.removeMember("iat");
    }
    else {
        if (!this->m_claims.isMember("iat")) {
            this->m_claims["iat"] = this->m_defaultLeeway;
        }
    }
}

void verif::requireClaim(const std::string& claimName, Json::Value value) {
    if (value.isNull()) {
        this->m_claims.removeMember(claimName);
    }
    if (!this->m_claims.isMember(claimName)) {
        this->m_claims[claimName] = value;
    }
}

}   // namespace pico