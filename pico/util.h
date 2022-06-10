#ifndef __PICO_UTIL_H__
#define __PICO_UTIL_H__

#include <boost/lexical_cast.hpp>
#include <byteswap.h>
#include <dirent.h>
#include <json/json.h>
#include <list>
#include <map>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/platform.h>
#include <mbedtls/sha1.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <pthread.h>
#include <set>
#include <string.h>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <mysql/mysql.h>

#include "fiber.h"
#include "singleton.h"

namespace pico {
pid_t getThreadId();

uint32_t getFiberId();

std::string getForamtedTime(const char* format);

uint64_t getCurrentTime();

std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");

class StringUtil
{
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);

    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

    static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");
};



void listDir(const std::string& path, std::vector<std::string>& files,
             const std::string& suffix = "");

template<class T>
T getValueFromMap(const std::map<std::string, std::string>& map, const std::string& key,
                  const T& default_value = T()) {
    auto it = map.find(key);
    if (it != map.end()) { return boost::lexical_cast<T>(it->second); }
    return default_value;
}


std::string base64_encode(const char* data, size_t data_len, bool strict = true);
std::string base64_encode(const std::string& data, bool strict = true);

std::string base64_decode(const char* encoded_data, size_t encoded_data_len, bool strict = true);
std::string base64_decode(const std::string& encoded_data, bool strict = true);

std::string Json2Str(const Json::Value& json);

bool Str2Json(const std::string& str, Json::Value& json);

void split(const std::string& str, std::vector<std::string>& tokens, const std::string delim);


std::string extract_pubkey_from_cert(const std::string& certstr, const std::string& pw = "");


std::shared_ptr<EVP_PKEY> load_public_key_from_string(const std::string& key,
                                                      const std::string& password = "");


std::shared_ptr<EVP_PKEY> load_private_key_from_string(const std::string& key,
                                                       const std::string& password = "");

std::shared_ptr<EVP_PKEY> load_public_ec_key_from_string(const std::string& key,
                                                         const std::string& password = "");

std::shared_ptr<EVP_PKEY> load_private_ec_key_from_string(const std::string& key,
                                                          const std::string& password = "");


std::string bn2raw(const BIGNUM* bn);

std::unique_ptr<BIGNUM, decltype(&BN_free)> raw2bn(const std::string& raw);

MYSQL_TIME time_t2mysql_time(std::time_t ts);

std::time_t mysql_time2time_t(const MYSQL_TIME& time);

int genRandom(int min, int max);

std::string genRandomString(
    int len, std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

std::string camel2underline(const std::string& camel);

template<typename T>
static bool isContainer(const std::vector<T>& value) {
    return true;
}

template<typename T>
static bool isContainer(const std::set<T>& value) {
    return true;
}

template<typename T>
static bool isContainer(const std::list<T>& value) {
    return true;
}

template<typename T>
static bool isContainer(const T& t) {
    return false;
}

class RowBounds
{
public:
    RowBounds() { limit_ = INT32_MAX; }
    RowBounds(int offset, int limit)
        : offset_(offset)
        , limit_(limit) {}
    int offset() const { return offset_; }
    int limit() const { return limit_; }

private:
    int offset_;
    int limit_;
};

std::string sha1sum(const std::string& str);

std::string sha1sum(const void* data, size_t len);

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}


template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#    define PICO_BYTE_ORDER PICO_BIG_ENDIAN
#else
#    define PICO_BYTE_ORDER PICO_LITTLE_ENDIAN
#endif

#if PICO_BYTE_ORDER == PICO_BIG_ENDIAN
template<class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

template<class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}
#else

template<class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

template<class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif

std::size_t find(const std::string& str, const std::string& substr, bool is_case_sensitive = false);

}   // namespace pico

#endif
