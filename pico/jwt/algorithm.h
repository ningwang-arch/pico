#ifndef __PICO_JWT_ALGORITHM_H__
#define __PICO_JWT_ALGORITHM_H__

#include <memory>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <string>


#include "../util.h"
#include "jwt_decoder.h"
#include "jwt_expect.h"

namespace pico {
class NoneAlgorithm;
class HMACAlgorithm;
class RSAAlgorithm;
class ECDSAAlgorithm;

class Algorithm
{
public:
    typedef std::shared_ptr<Algorithm> Ptr;

    Algorithm(const std::string& name, const std::string& description)
        : m_name(name)
        , m_description(description) {}

    static Algorithm::Ptr HMAC256(std::string secret) {
        return std::static_pointer_cast<Algorithm>(
            std::make_shared<HMACAlgorithm>(EVP_sha256, secret, "HS256", "HMAC using SHA-256"));
    }

    static Algorithm::Ptr HMAC384(std::string secret) {
        return std::static_pointer_cast<Algorithm>(
            std::make_shared<HMACAlgorithm>(EVP_sha384, secret, "HS384", "HMAC using SHA-384"));
    }

    static Algorithm::Ptr HMAC512(std::string secret) {
        return std::static_pointer_cast<Algorithm>(
            std::make_shared<HMACAlgorithm>(EVP_sha512, secret, "HS512", "HMAC using SHA-512"));
    }

    static Algorithm::Ptr RSA256(std::string public_key, std::string private_key,
                                 std::string public_pwd = "", std::string private_pwd = "") {
        return std::static_pointer_cast<Algorithm>(
            std::make_shared<RSAAlgorithm>(public_key,
                                           private_key,
                                           public_pwd,
                                           private_pwd,
                                           EVP_sha256,
                                           "RS256",
                                           "RSA using SHA-256"));
    }

    static Algorithm::Ptr RSA384(std::string public_key, std::string private_key,
                                 std::string public_pwd = "", std::string private_pwd = "") {
        return std::static_pointer_cast<Algorithm>(
            std::make_shared<RSAAlgorithm>(public_key,
                                           private_key,
                                           public_pwd,
                                           private_pwd,
                                           EVP_sha384,
                                           "RS384",
                                           "RSA using SHA-384"));
    }

    static Algorithm::Ptr RSA512(std::string public_key, std::string private_key,
                                 std::string public_pwd = "", std::string private_pwd = "") {
        return std::static_pointer_cast<Algorithm>(
            std::make_shared<RSAAlgorithm>(public_key,
                                           private_key,
                                           public_pwd,
                                           private_pwd,
                                           EVP_sha512,
                                           "RS512",
                                           "RSA using SHA-512"));
    }

    static Algorithm::Ptr ECDSA256(std::string public_key, std::string private_key,
                                   std::string public_pwd = "", std::string private_pwd = "") {
        return std::static_pointer_cast<Algorithm>(
            std::make_shared<ECDSAAlgorithm>(public_key,
                                             private_key,
                                             public_pwd,
                                             private_pwd,
                                             EVP_sha256,
                                             "ES256",
                                             "ECDSA using SHA-256",
                                             64));
    }

    static Algorithm::Ptr ECDSA384(std::string public_key, std::string private_key,
                                   std::string public_pwd = "", std::string private_pwd = "") {
        return std::static_pointer_cast<Algorithm>(
            std::make_shared<ECDSAAlgorithm>(public_key,
                                             private_key,
                                             public_pwd,
                                             private_pwd,
                                             EVP_sha384,
                                             "ES384",
                                             "ECDSA using SHA-384",
                                             96));
    }

    static Algorithm::Ptr ECDSA512(std::string public_key, std::string private_key,
                                   std::string public_pwd = "", std::string private_pwd = "") {
        return std::static_pointer_cast<Algorithm>(
            std::make_shared<ECDSAAlgorithm>(public_key,
                                             private_key,
                                             public_pwd,
                                             private_pwd,
                                             EVP_sha512,
                                             "ES512",
                                             "ECDSA using SHA-512",
                                             132));
    }

    static Algorithm::Ptr none() {
        return std::static_pointer_cast<Algorithm>(std::make_shared<NoneAlgorithm>());
    }

    std::string getName() const { return m_name; }
    std::string getDescription() const { return m_description; }

    std::string to_string() const { return m_description; }

    virtual void verify(JWTDecoder::Ptr decoder) {
        std::string signature = decoder->getSignature();
        if (signature != sign(decoder->getHeader(), decoder->getPayload())) {
            throw VerificationException("Signature verification failed");
        }
    }

    virtual std::string sign(const std::string& header, const std::string& payload) {
        std::string header_base64 = base64_encode(header.c_str(), header.size());
        std::string payload_base64 = base64_encode(payload.c_str(), payload.size());
        return sign(header_base64 + "." + payload_base64);
    }

    virtual std::string sign(const std::string& str) = 0;



private:
    std::string m_name;
    std::string m_description;
};



class NoneAlgorithm : public Algorithm
{
public:
    typedef std::shared_ptr<NoneAlgorithm> Ptr;
    NoneAlgorithm()
        : Algorithm("none", "No signature or encryption") {}

    std::string sign(const std::string& str) override { return std::string(); }
};

class HMACAlgorithm : public Algorithm
{
public:
    typedef std::shared_ptr<HMACAlgorithm> Ptr;

    HMACAlgorithm(const EVP_MD* (*md)(), const std::string& key, std::string name,
                  std::string description)
        : Algorithm(name, description)
        , m_key(key)
        , m_md(md) {}


    std::string sign(const std::string& str) override {
        unsigned char* digest = HMAC(m_md(),
                                     m_key.c_str(),
                                     m_key.size(),
                                     (unsigned char*)str.c_str(),
                                     str.size(),
                                     NULL,
                                     NULL);
        // OPENSSL_free(digest);
        return reinterpret_cast<char*>(digest);
    }

private:
    std::string m_key;
    const EVP_MD* (*m_md)();
};

class RSAAlgorithm : public Algorithm
{
public:
    typedef std::shared_ptr<RSAAlgorithm> Ptr;

    RSAAlgorithm(const std::string& public_key, const std::string& private_key,
                 const std::string& public_key_password, const std::string& private_key_password,
                 const EVP_MD* (*md)(), std::string name, std::string description)
        : Algorithm(name, description)
        , m_md(md) {
        if (!private_key.empty()) {
            m_key = load_private_key_from_string(private_key, private_key_password);
        }
        else if (!public_key.empty()) {
            m_key = load_public_key_from_string(public_key, public_key_password);
        }
        else {
            throw std::runtime_error("No key provided");
        }
    }

    std::string sign(const std::string& str) override {
        std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_create(),
                                                                    EVP_MD_CTX_free);
        if (!ctx) { return {}; }
        if (!EVP_SignInit(ctx.get(), m_md())) { return {}; }

        std::string res(EVP_PKEY_size(m_key.get()), '\0');
        unsigned int len = 0;

        if (!EVP_SignUpdate(ctx.get(), str.data(), str.size())) { return {}; }
        if (EVP_SignFinal(ctx.get(), (unsigned char*)res.data(), &len, m_key.get()) == 0) {
            return {};
        }

        res.resize(len);
        return res;
    }


private:
    const EVP_MD* (*m_md)();
    std::shared_ptr<EVP_PKEY> m_key;
};

class ECDSAAlgorithm : public Algorithm
{
public:
    typedef std::shared_ptr<ECDSAAlgorithm> Ptr;

    ECDSAAlgorithm(const std::string& public_key, const std::string& private_key,
                   const std::string& public_key_password, const std::string& private_key_password,
                   const EVP_MD* (*md)(), std::string name, std::string description, size_t siglen)
        : Algorithm(name, description)
        , m_md(md)
        , m_signature_length(siglen) {
        if (!private_key.empty()) {
            m_key = load_private_key_from_string(private_key, private_key_password);
            if (!check_key(m_key.get())) { throw std::runtime_error("Invalid private key"); }
        }
        else if (!public_key.empty()) {
            m_key = load_public_key_from_string(public_key, public_key_password);
            if (!check_key(m_key.get())) { throw std::runtime_error("Invalid public key"); }
        }
        else {
            throw std::runtime_error("No key provided");
        }
    }

    std::string sign(const std::string& str) override {
        std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_create(),
                                                                    EVP_MD_CTX_free);
        if (!ctx) { return {}; }
        if (!EVP_DigestSignInit(ctx.get(), nullptr, m_md(), nullptr, m_key.get())) { return {}; }
        if (!EVP_DigestUpdate(ctx.get(), str.data(), str.size())) { return {}; }

        size_t len = 0;
        if (!EVP_DigestSignFinal(ctx.get(), nullptr, &len)) { return {}; }
        std::string res(len, '\0');
        if (!EVP_DigestSignFinal(ctx.get(), (unsigned char*)res.data(), &len)) { return {}; }

        res.resize(len);
        return der_to_p1363_signature(res);
    }

private:
    static bool check_key(EVP_PKEY* pkey) {
        std::unique_ptr<EC_KEY, decltype(&EC_KEY_free)> eckey(EVP_PKEY_get1_EC_KEY(pkey),
                                                              EC_KEY_free);
        if (!eckey) { return false; }
        if (EC_KEY_check_key(eckey.get()) == 0) return false;
        return true;
    }

    std::string der_to_p1363_signature(const std::string& der_signature) const {
        const unsigned char* possl_signature =
            reinterpret_cast<const unsigned char*>(der_signature.data());
        std::unique_ptr<ECDSA_SIG, decltype(&ECDSA_SIG_free)> sig(
            d2i_ECDSA_SIG(nullptr, &possl_signature, der_signature.length()), ECDSA_SIG_free);
        if (!sig) { return {}; }

        const BIGNUM* r;
        const BIGNUM* s;
        ECDSA_SIG_get0(sig.get(), &r, &s);
        auto rr = bn2raw(r);
        auto rs = bn2raw(s);

        if (rr.size() > m_signature_length / 2 || rs.size() > m_signature_length / 2)
            throw std::logic_error("bignum size exceeded expected length");
        rr.insert(0, m_signature_length / 2 - rr.size(), '\0');
        rs.insert(0, m_signature_length / 2 - rs.size(), '\0');
        return rr + rs;
    }

    std::string p1363_to_der_signature(const std::string& signature, std::error_code& ec) const {
        ec.clear();
        auto r = raw2bn(signature.substr(0, signature.size() / 2));
        auto s = raw2bn(signature.substr(signature.size() / 2));

        ECDSA_SIG* psig;

        std::unique_ptr<ECDSA_SIG, decltype(&ECDSA_SIG_free)> sig(ECDSA_SIG_new(), ECDSA_SIG_free);
        if (!sig) { return {}; }
        ECDSA_SIG_set0(sig.get(), r.release(), s.release());
        psig = sig.get();

        int length = i2d_ECDSA_SIG(psig, nullptr);
        if (length < 0) { return {}; }
        std::string der_signature(length, '\0');
        unsigned char* psbuffer = (unsigned char*)der_signature.data();
        length = i2d_ECDSA_SIG(psig, &psbuffer);
        if (length < 0) { return {}; }
        der_signature.resize(length);
        return der_signature;
    }



private:
    std::shared_ptr<EVP_PKEY> m_key;
    const EVP_MD* (*m_md)();
    const size_t m_signature_length;
};


}   // namespace pico

#endif