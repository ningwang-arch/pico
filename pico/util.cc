#include "util.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdarg.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <vector>

#include "logging.h"

namespace pico {
pid_t getThreadId() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

uint32_t getFiberId() {
    return pico::Fiber::GetFiberId();
}

std::string getForamtedTime(const char* format) {
    std::stringstream ss;
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    ss << std::put_time(tm, format);
    return ss.str();
}

uint64_t getCurrentTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

std::string Time2Str(time_t ts, const std::string& format) {
    std::stringstream ss;
    struct tm* tm = localtime(&ts);
    ss << std::put_time(tm, format.c_str());
    return ss.str();
}
time_t Str2Time(const char* str, const char* format) {
    struct tm tm;
    strptime(str, format, &tm);
    return mktime(&tm);
}

std::string StringUtil::Format(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    auto v = Formatv(fmt, ap);
    va_end(ap);
    return v;
}

std::string StringUtil::Formatv(const char* fmt, va_list ap) {
    char* buf = nullptr;
    auto len = vasprintf(&buf, fmt, ap);
    if (len == -1) { return ""; }
    std::string ret(buf, len);
    if (buf) { free(buf); }
    return ret;
}

static const char uri_chars[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char xdigit_chars[256] = {
    0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15, 0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0,
};

#define CHAR_IS_UNRESERVED(c) (uri_chars[(unsigned char)(c)])

//-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~
std::string StringUtil::UrlEncode(const std::string& str, bool space_as_plus) {
    static const char* hexdigits = "0123456789ABCDEF";
    std::string* ss = nullptr;
    const char* end = str.c_str() + str.length();
    for (const char* c = str.c_str(); c < end; ++c) {
        if (!CHAR_IS_UNRESERVED(*c)) {
            if (!ss) {
                ss = new std::string;
                ss->reserve(str.size() * 1.2);
                ss->append(str.c_str(), c - str.c_str());
            }
            if (*c == ' ' && space_as_plus) { ss->append(1, '+'); }
            else {
                ss->append(1, '%');
                ss->append(1, hexdigits[(uint8_t)*c >> 4]);
                ss->append(1, hexdigits[*c & 0xf]);
            }
        }
        else if (ss) {
            ss->append(1, *c);
        }
    }
    if (!ss) { return str; }
    else {
        std::string rt = *ss;
        delete ss;
        return rt;
    }
}

std::string StringUtil::UrlDecode(const std::string& str, bool space_as_plus) {
    std::string* ss = nullptr;
    const char* end = str.c_str() + str.length();
    for (const char* c = str.c_str(); c < end; ++c) {
        if (*c == '+' && space_as_plus) {
            if (!ss) {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, ' ');
        }
        else if (*c == '%' && (c + 2) < end && isxdigit(*(c + 1)) && isxdigit(*(c + 2))) {
            if (!ss) {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, (char)(xdigit_chars[(int)*(c + 1)] << 4 | xdigit_chars[(int)*(c + 2)]));
            c += 2;
        }
        else if (ss) {
            ss->append(1, *c);
        }
    }
    if (!ss) { return str; }
    else {
        std::string rt = *ss;
        delete ss;
        return rt;
    }
}

std::string StringUtil::Trim(const std::string& str, const std::string& delimit) {
    auto begin = str.find_first_not_of(delimit);
    if (begin == std::string::npos) { return ""; }
    auto end = str.find_last_not_of(delimit);
    return str.substr(begin, end - begin + 1);
}

std::string StringUtil::TrimLeft(const std::string& str, const std::string& delimit) {
    auto begin = str.find_first_not_of(delimit);
    if (begin == std::string::npos) { return ""; }
    return str.substr(begin);
}

std::string StringUtil::TrimRight(const std::string& str, const std::string& delimit) {
    auto end = str.find_last_not_of(delimit);
    if (end == std::string::npos) { return ""; }
    return str.substr(0, end);
}

void listDir(const std::string& path, std::vector<std::string>& files, const std::string& suffix) {
    if (access(path.c_str(), 0) != 0) { return; }
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) { return; }
    struct dirent* dp = nullptr;
    while ((dp = readdir(dir)) != nullptr) {
        if (dp->d_type == DT_DIR) {
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) { continue; }
            listDir(path + dp->d_name, files, suffix);
        }
        else if (dp->d_type == DT_REG) {
            std::string filename(dp->d_name);
            if (suffix.empty()) {
                files.push_back(path + "/" + filename);
                // files.push_back(filename);
            }
            else {
                if (filename.size() < suffix.size()) { continue; }
                if (filename.substr(filename.length() - suffix.size()) == suffix) {
                    files.push_back(path + "/" + filename);
                    // files.push_back(filename);
                }
            }
        }
    }
    closedir(dir);
}

std::string base64_encode(const char* data, size_t length, bool strict) {
    auto input = reinterpret_cast<const unsigned char*>(data);
    const auto pl = 4 * ((length + 2) / 3);
    auto output = reinterpret_cast<char*>(
        calloc(pl + 1, 1));   //+1 for the terminating null that EVP_EncodeBlock adds on
    const auto ol = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(output), input, length);
    if ((int)pl != ol) {
        std::cerr << "Whoops, encode predicted " << pl << " but we got " << ol << "\n";
    }
    std::string str = reinterpret_cast<char*>(output);
    // delete '=' padding
    if (strict) {
        str.erase(std::remove(str.begin(), str.end(), '='), str.end());
        // replace '/' with '_', '+' with '-'
        std::replace(str.begin(), str.end(), '/', '_');
        std::replace(str.begin(), str.end(), '+', '-');
    }
    free(output);
    return str;
}

std::string base64_encode(const std::string& str, bool strict) {
    return base64_encode(str.c_str(), str.length(), strict);
}

std::string base64_decode(const char* data, size_t length, bool strict) {
    std::string input = data;
    if (strict) {
        // add '=' padding
        if (length % 4 != 0) {
            size_t pad = 4 - length % 4;
            for (size_t i = 0; i < pad; i++) { input += '='; }
        }
        // replace '-' with '+', '_' with '/'
        std::replace(input.begin(), input.end(), '-', '+');
        std::replace(input.begin(), input.end(), '_', '/');
    }
    const auto pl = 3 * input.size() / 4;
    auto output = reinterpret_cast<unsigned char*>(calloc(pl + 1, 1));
    const auto ol =
        EVP_DecodeBlock(output, reinterpret_cast<const unsigned char*>(input.data()), input.size());
    if ((int)pl != ol) {
        std::cerr << "Whoops, decode predicted " << pl << " but we got " << ol << "\n";
        return std::string();
    }
    // std::string str = reinterpret_cast<char*>(output);
    // free(output);
    return reinterpret_cast<char*>(output);
}

std::string base64_decode(const std::string& str, bool strict) {
    return base64_decode(str.c_str(), str.length(), strict);
}

std::string Json2Str(const Json::Value& json) {
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    // change accuracy of double to 6 digits
    builder.settings_["precision"] = 6;
    std::string str = Json::writeString(builder, json);
    return str;
}

bool Str2Json(const std::string& str, Json::Value& json) {
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();

    std::string errors;

    bool parsingSuccessful = reader->parse(str.c_str(), str.c_str() + str.size(), &json, &errors);
    delete reader;

    if (!parsingSuccessful) {
        LOG_ERROR("Failed to parse json: %s", errors.c_str());
        return false;
    }
    return true;
}

void split(const std::string& str, std::vector<std::string>& tokens, const std::string delim) {
    tokens.clear();

    char* buffer = new char[str.size() + 1];
    std::strcpy(buffer, str.c_str());

    char* tmp;
    char* p = strtok_r(buffer, delim.c_str(), &tmp);   // 第一次分割
    do {
        tokens.push_back(p);   // 如果 p 为 nullptr，则将整个字符串作为结果
    } while ((p = strtok_r(nullptr, delim.c_str(), &tmp)) != nullptr);
    // strtok_r 为 strtok 的线程安全版本。
}

std::string extract_pubkey_from_cert(const std::string& certstr, const std::string& pw) {
    std::unique_ptr<BIO, decltype(&BIO_free_all)> certbio(
        BIO_new_mem_buf(certstr.data(), static_cast<int>(certstr.size())), BIO_free_all);
    std::unique_ptr<BIO, decltype(&BIO_free_all)> keybio(BIO_new(BIO_s_mem()), BIO_free_all);
    if (!certbio || !keybio) { return {}; }

    std::unique_ptr<X509, decltype(&X509_free)> cert(
        PEM_read_bio_X509(certbio.get(), nullptr, nullptr, const_cast<char*>(pw.c_str())),
        X509_free);
    if (!cert) { return {}; }
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key(X509_get_pubkey(cert.get()),
                                                            EVP_PKEY_free);
    if (!key) { return {}; }
    if (PEM_write_bio_PUBKEY(keybio.get(), key.get()) == 0) { return {}; }
    char* ptr = nullptr;
    auto len = BIO_get_mem_data(keybio.get(), &ptr);
    if (len <= 0 || ptr == nullptr) { return {}; }
    return {ptr, static_cast<size_t>(len)};
}


std::shared_ptr<EVP_PKEY> load_public_key_from_string(const std::string& key,
                                                      const std::string& password) {
    std::unique_ptr<BIO, decltype(&BIO_free_all)> pubkey_bio(BIO_new(BIO_s_mem()), BIO_free_all);
    if (!pubkey_bio) { return nullptr; }
    if (key.substr(0, 27) == "-----BEGIN CERTIFICATE-----") {
        auto epkey = extract_pubkey_from_cert(key, password);
        if (epkey.empty()) { return nullptr; }
        const int len = static_cast<int>(epkey.size());
        if (BIO_write(pubkey_bio.get(), epkey.data(), len) != len) { return nullptr; }
    }
    else {
        const int len = static_cast<int>(key.size());
        if (BIO_write(pubkey_bio.get(), key.data(), len) != len) { return nullptr; }
    }

    std::shared_ptr<EVP_PKEY> pkey(
        PEM_read_bio_PUBKEY(
            pubkey_bio.get(),
            nullptr,
            nullptr,
            (void*)password.data()),   // NOLINT(google-readability-casting) requires `const_cast`
        EVP_PKEY_free);
    if (!pkey) { return nullptr; }
    return pkey;
}



std::shared_ptr<EVP_PKEY> load_private_key_from_string(const std::string& key,
                                                       const std::string& password) {
    std::unique_ptr<BIO, decltype(&BIO_free_all)> privkey_bio(BIO_new(BIO_s_mem()), BIO_free_all);
    if (!privkey_bio) { return nullptr; }
    const int len = static_cast<int>(key.size());
    if (BIO_write(privkey_bio.get(), key.data(), len) != len) { return nullptr; }
    std::shared_ptr<EVP_PKEY> pkey(
        PEM_read_bio_PrivateKey(
            privkey_bio.get(), nullptr, nullptr, const_cast<char*>(password.c_str())),
        EVP_PKEY_free);
    if (!pkey) { return nullptr; }
    return pkey;
}



std::shared_ptr<EVP_PKEY> load_public_ec_key_from_string(const std::string& key,
                                                         const std::string& password) {
    std::unique_ptr<BIO, decltype(&BIO_free_all)> pubkey_bio(BIO_new(BIO_s_mem()), BIO_free_all);
    if (!pubkey_bio) { return nullptr; }
    if (key.substr(0, 27) == "-----BEGIN CERTIFICATE-----") {
        auto epkey = extract_pubkey_from_cert(key, password);
        if (epkey.empty()) return nullptr;
        const int len = static_cast<int>(epkey.size());
        if (BIO_write(pubkey_bio.get(), epkey.data(), len) != len) { return nullptr; }
    }
    else {
        const int len = static_cast<int>(key.size());
        if (BIO_write(pubkey_bio.get(), key.data(), len) != len) { return nullptr; }
    }

    std::shared_ptr<EVP_PKEY> pkey(
        PEM_read_bio_PUBKEY(
            pubkey_bio.get(),
            nullptr,
            nullptr,
            (void*)password.data()),   // NOLINT(google-readability-casting) requires `const_cast`
        EVP_PKEY_free);
    if (!pkey) { return nullptr; }
    return pkey;
}

std::shared_ptr<EVP_PKEY> load_private_ec_key_from_string(const std::string& key,
                                                          const std::string& password) {
    std::unique_ptr<BIO, decltype(&BIO_free_all)> privkey_bio(BIO_new(BIO_s_mem()), BIO_free_all);
    if (!privkey_bio) { return nullptr; }
    const int len = static_cast<int>(key.size());
    if (BIO_write(privkey_bio.get(), key.data(), len) != len) { return nullptr; }
    std::shared_ptr<EVP_PKEY> pkey(
        PEM_read_bio_PrivateKey(
            privkey_bio.get(), nullptr, nullptr, const_cast<char*>(password.c_str())),
        EVP_PKEY_free);
    if (!pkey) { return nullptr; }
    return pkey;
}

std::string bn2raw(const BIGNUM* bn) {
    std::string res(BN_num_bytes(bn), '\0');
    BN_bn2bin(
        bn,
        (unsigned char*)res.data());   // NOLINT(google-readability-casting) requires `const_cast`
    return res;
}

std::unique_ptr<BIGNUM, decltype(&BN_free)> raw2bn(const std::string& raw) {
    return std::unique_ptr<BIGNUM, decltype(&BN_free)>(
        BN_bin2bn(reinterpret_cast<const unsigned char*>(raw.data()),
                  static_cast<int>(raw.size()),
                  nullptr),
        BN_free);
}

MYSQL_TIME time_t2mysql_time(std::time_t ts) {
    struct tm* tm_time = localtime(&ts);
    MYSQL_TIME mysql_time;
    mysql_time.year = tm_time->tm_year + 1900;
    mysql_time.month = tm_time->tm_mon + 1;
    mysql_time.day = tm_time->tm_mday;
    mysql_time.hour = tm_time->tm_hour;
    mysql_time.minute = tm_time->tm_min;
    mysql_time.second = tm_time->tm_sec;
    mysql_time.second_part = 0;
    mysql_time.time_type = MYSQL_TIMESTAMP_DATETIME;
    return mysql_time;
}

std::time_t mysql_time2time_t(const MYSQL_TIME& time) {
    struct tm tm_time = {0};
    tm_time.tm_year = time.year - 1900;
    tm_time.tm_mon = time.month - 1;
    tm_time.tm_mday = time.day;
    tm_time.tm_hour = time.hour;
    tm_time.tm_min = time.minute;
    tm_time.tm_sec = time.second;
    tm_time.tm_isdst = -1;
    return mktime(&tm_time);
}

int genRandom(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

std::string genRandomString(int len, std::string chars) {
    std::string str;
    for (int i = 0; i < len; i++) { str += chars[genRandom(0, chars.size() - 1)]; }
    return str;
}

std::string camel2underline(const std::string& str) {
    std::string res;
    for (auto c : str) {
        if (isupper(c)) {
            res += '_';
            res += tolower(c);
        }
        else {
            res += c;
        }
    }

    return res.at(0) == '_' ? res.substr(1) : res;
}

std::string sha1sum(const std::string& str) {
    return sha1sum(str.data(), str.size());
}

std::string sha1sum(const void* data, size_t len) {
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, data, len);
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1_Final(digest, &ctx);
    std::string res(SHA_DIGEST_LENGTH, '\0');
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) { res[i] = digest[i]; }
    return res;
}

std::size_t find(const std::string& str, const std::string& substr, bool is_case_sensitive) {
    if (is_case_sensitive) { return str.find(substr); }
    else {
        return std::find_if(
                   str.begin(),
                   str.end(),
                   [&substr](char c) { return std::tolower(c) == std::tolower(substr[0]); }) -
               str.begin();
    }
}

}   // namespace pico
