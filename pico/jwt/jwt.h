#ifndef __PICO_JWT_H__
#define __PICO_JWT_H__

#include "jwt_decoder.h"
#include <json/json.h>
#include <memory>
#include <string>
#include <vector>

#include "jwt_creator.h"
#include "jwt_verifier.h"

namespace pico {

class JWT
{
public:
    typedef std::shared_ptr<JWT> Ptr;

    JWT() {}

    static JWTDecoder::Ptr decode(const std::string& jwt) {
        return std::make_shared<JWTDecoder>(jwt);
    }

    static JWTCreator::Builder::Ptr create() { return JWTCreator::init(); }

    static JWTVerifier::Verification::Ptr require(Algorithm::Ptr algorithm) {
        return JWTVerifier::init(algorithm);
    }

private:
    Json::Value payload_;
    Json::Value header_;
    std::string signature_;
    std::string key_;
};


}   // namespace pico


#endif