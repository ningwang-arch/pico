#ifndef __PICO_UTIL_H__
#define __PICO_UTIL_H__

#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
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
#include <regex>
#include <set>
#include <string.h>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <mysql/mysql.h>


#include "fiber.h"
#include "singleton.h"
#include <iostream>

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
    if (it != map.end()) {
        return boost::lexical_cast<T>(it->second);
    }
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


/**
 * @brief fuzzy_match
 * @param str ---> /user/123/abc
 * @param pattern ---> /user/<int>/<string>
 * @return true if match
 */

bool fuzzy_match(const std::string& str, const std::string& pattern);


unsigned find_closing_tag_runtime(const char* s, unsigned p);

uint64_t get_parameter_tag_runtime(const char* s, unsigned p = 0);

bool is_parameter_tag_compatible(uint64_t a, uint64_t b);

template<typename T>
struct parameter_tag
{ static const int value = 0; };
#define PICO_INTERNAL_PARAMETER_TAG(t, i) \
    template<>                            \
    struct parameter_tag<t>               \
    { static const int value = i; }
PICO_INTERNAL_PARAMETER_TAG(int, 1);
PICO_INTERNAL_PARAMETER_TAG(char, 1);
PICO_INTERNAL_PARAMETER_TAG(short, 1);
PICO_INTERNAL_PARAMETER_TAG(long, 1);
PICO_INTERNAL_PARAMETER_TAG(long long, 1);
PICO_INTERNAL_PARAMETER_TAG(unsigned int, 2);
PICO_INTERNAL_PARAMETER_TAG(unsigned char, 2);
PICO_INTERNAL_PARAMETER_TAG(unsigned short, 2);
PICO_INTERNAL_PARAMETER_TAG(unsigned long, 2);
PICO_INTERNAL_PARAMETER_TAG(unsigned long long, 2);
PICO_INTERNAL_PARAMETER_TAG(double, 3);
PICO_INTERNAL_PARAMETER_TAG(std::string, 4);
#undef PICO_INTERNAL_PARAMETER_TAG
template<typename... Args>
struct compute_parameter_tag_from_args_list;

template<>
struct compute_parameter_tag_from_args_list<>
{ static const int value = 0; };

template<typename Arg, typename... Args>
struct compute_parameter_tag_from_args_list<Arg, Args...>
{

    static const int sub_value = compute_parameter_tag_from_args_list<Args...>::value;
    static const int value =
        parameter_tag<typename std::decay<Arg>::type>::value
            ? sub_value * 6 + parameter_tag<typename std::decay<Arg>::type>::value
            : sub_value;
};


template<typename T>
struct function_traits;

template<typename T>
struct function_traits : public function_traits<decltype(&T::operator())>
{
    using parent_t = function_traits<decltype(&T::operator())>;
    static const size_t arity = parent_t::arity;
    using result_type = typename parent_t::result_type;
    template<size_t i>
    using arg = typename parent_t::template arg<i>;
};

template<typename ClassType, typename R, typename... Args>
struct function_traits<R (ClassType::*)(Args...) const>
{
    static const size_t arity = sizeof...(Args);

    typedef R result_type;

    template<size_t i>
    using arg = typename std::tuple_element<i, std::tuple<Args...>>::type;
};

template<typename ClassType, typename R, typename... Args>
struct function_traits<R (ClassType::*)(Args...)>
{
    static const size_t arity = sizeof...(Args);

    typedef R result_type;

    template<size_t i>
    using arg = typename std::tuple_element<i, std::tuple<Args...>>::type;
};

template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>>
{
    static const size_t arity = sizeof...(Args);

    typedef R result_type;

    template<size_t i>
    using arg = typename std::tuple_element<i, std::tuple<Args...>>::type;
};

}   // namespace pico

#endif
