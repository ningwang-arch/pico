#include <iostream>

#include "pico/date.h"
#include "pico/jwt/jwt.h"
#include <memory>

std::string token_gen() {
    const std::string TOKEN_SECRET = "privateKey";

    pico::Algorithm::Ptr alg = pico::Algorithm::HMAC256(TOKEN_SECRET);


    pico::JWTCreator::Builder::Ptr builder = pico::JWTCreator::init();

    builder->withHeader(Json::Value(Json::objectValue))
        ->withKeyId("keyId")
        ->withIssuer("issuer")
        ->withSubject("subject")
        ->withAudience(std::vector<std::string>{"audience1", "audience2"})
        ->withExpiresAt(pico::Date() + 3)
        ->withNotBefore(pico::Date() /* + 30*/)
        ->withIssuedAt(pico::Date())
        ->withJWTId("jwtId")
        ->withClaim("name", "value")
        ->withClaim("name", std::vector<std::string>{"value1", "value2"});

    // std::cout << builder->to_string() << std::endl;

    std::string token = builder->sign(alg);
    // std::cout << token << std::endl;
    return token;
}


void verify(std::string token) {
    try {
        pico::Algorithm::Ptr alg = pico::Algorithm::HMAC256("privateKey");
        auto verifier = pico::JWT::require(alg);
        std::vector<std::string> audience = {"audience1", "audience2"};
        verifier->withAudience(audience);
        auto decoder = verifier->build()->verify(token);
        // auto decoder = std::make_shared<pico::JWTDecoder>(token);
        // auto decoder = verifier->verify(token);

        // std::cout << decoder->to_string() << std::endl;
        std::cout << decoder->getExpiration().to_string() << std::endl;
        std::cout << decoder->getJwtId() << std::endl;
        sleep(5);
        //
        std::cout << verifier->build()->verify(token)->getJwtId();
        // std::cout << decoder->getExpiration().to_string() << std::endl;
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}


int main(int argc, char const* argv[]) {
    std::string token = token_gen();
    verify(token);
    return 0;
}
