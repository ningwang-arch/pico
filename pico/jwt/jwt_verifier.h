#ifndef __PICO_JWT_VERIFIER_H__
#define __PICO_JWT_VERIFIER_H__

#include <json/json.h>
#include <memory>
#include <string>

#include "../date.h"
#include "algorithm.h"
#include "jwt_decoder.h"

namespace pico {
class JWTVerifier
{

public:
    class Verification;
    typedef std::shared_ptr<JWTVerifier> Ptr;

    JWTVerifier(const Algorithm::Ptr& algorithm, const Json::Value& cliams, const Date& date) {
        this->m_algorithm = algorithm;
        this->m_claims = cliams;
        this->m_date = date;
    }

    static std::shared_ptr<Verification> init(Algorithm::Ptr algorithm);

    JWTDecoder::Ptr verify(const std::string& jwt);
    JWTDecoder::Ptr verify(const JWTDecoder::Ptr& decoder);

    void verifyAlgorithm(JWTDecoder::Ptr decoder, Algorithm::Ptr expectedAlgorithm);
    void verifyClaims(JWTDecoder::Ptr decoder, Json::Value value);

    void verifyClaimValues(JWTDecoder::Ptr decoder, std::string name, Json::Value expectedClaim);


    void assertValidStringClaim(std::string name, std::string value, std::string expected);

    void assertValidDateClaim(Date date, long leeway, bool shouldBeFuture);

    void assertDateIsFuture(Date date, long leeway, Date today);

    void assertDateIsPast(Date date, long leeway, Date today);

    void assertValidClaim(Json::Value claim, std::string name, Json::Value value);

    void assertValidAudience(std::vector<std::string> value, std::vector<std::string> expected,
                             bool shouldContainAll);

    void assertValidIssuer(std::string issuer, std::vector<std::string> value);




public:
    class Verification : public std::enable_shared_from_this<Verification>
    {
    public:
        typedef std::shared_ptr<Verification> Ptr;

        Verification(Algorithm::Ptr algorithm) {
            if (!algorithm) throw std::runtime_error("algorithm is null");
            m_algorithm = algorithm;
            m_defaultLeeway = 0L;
        }

        Verification::Ptr withIssuer(const std::vector<std::string>& issuers);
        Verification::Ptr withSubject(const std::string& subject);
        Verification::Ptr withAudience(const std::vector<std::string>& audiences);
        Verification::Ptr withAnyOfAudience(const std::vector<std::string>& audiences);
        Verification::Ptr acceptLeeWay(long leeway);
        Verification::Ptr acceptExpiresAt(long leeway);
        Verification::Ptr acceptNotBefore(long leeway);
        Verification::Ptr acceptIssuedAt(long leeway);
        Verification::Ptr ignoreIssuedAt();
        Verification::Ptr withJwtId(const std::string& jwt_id);
        Verification::Ptr withClaimPresence(const std::string& name);

        template<typename T>
        Verification::Ptr withClaim(const std::string& name, const T& value) {
            assert(!name.empty());
            this->requireClaim(name, value);
            return shared_from_this();
        }

        template<typename T>
        Verification::Ptr withArrayClaim(const std::string& name, const std::vector<T>& value) {
            assert(!name.empty());
            Json::Value array = Json::arrayValue;
            for (auto& i : value) { array.append(i); }
            this->requireClaim(name, array);
            return shared_from_this();
        }

        void addLeewayToDateClaims();

        JWTVerifier::Ptr build();

        JWTVerifier::Ptr build(const Date& date);



        void requireClaim(const std::string& name, Json::Value value);


    private:
        Algorithm::Ptr m_algorithm;
        Json::Value m_claims;
        long m_defaultLeeway;
        bool m_ignoreIssuedAt;
    };

private:
    static bool isNullOrEmpty(Json::Value& value);


private:
    Algorithm::Ptr m_algorithm;
    Json::Value m_claims;
    Date m_date;

    const std::string AUDIENCE_EXACT = "AUDIENCE_EXACT";
    const std::string AUDIENCE_CONTAINS = "AUDIENCE_CONTAINS";
};

}   // namespace pico

#endif