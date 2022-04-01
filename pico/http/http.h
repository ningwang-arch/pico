#ifndef __PICO_HTTP_HTTP_H__
#define __PICO_HTTP_HTTP_H__

#include <functional>
#include <map>
#include <memory>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

namespace pico {
/*Status Codes*/
#define HTTP_STATUS_MAP(XX)                                                   \
    XX(100, CONTINUE, Continue)                                               \
    XX(101, SWITCHING_PROTOCOLS, Switching Protocols)                         \
    XX(102, PROCESSING, Processing)                                           \
    XX(200, OK, OK)                                                           \
    XX(201, CREATED, Created)                                                 \
    XX(202, ACCEPTED, Accepted)                                               \
    XX(203, NON_AUTHORITATIVE_INFORMATION, Non - Authoritative Information)   \
    XX(204, NO_CONTENT, No Content)                                           \
    XX(205, RESET_CONTENT, Reset Content)                                     \
    XX(206, PARTIAL_CONTENT, Partial Content)                                 \
    XX(207, MULTI_STATUS, Multi - Status)                                     \
    XX(208, ALREADY_REPORTED, Already Reported)                               \
    XX(226, IM_USED, IM Used)                                                 \
    XX(300, MULTIPLE_CHOICES, Multiple Choices)                               \
    XX(301, MOVED_PERMANENTLY, Moved Permanently)                             \
    XX(302, FOUND, Found)                                                     \
    XX(303, SEE_OTHER, See Other)                                             \
    XX(304, NOT_MODIFIED, Not Modified)                                       \
    XX(305, USE_PROXY, Use Proxy)                                             \
    XX(307, TEMPORARY_REDIRECT, Temporary Redirect)                           \
    XX(308, PERMANENT_REDIRECT, Permanent Redirect)                           \
    XX(400, BAD_REQUEST, Bad Request)                                         \
    XX(401, UNAUTHORIZED, Unauthorized)                                       \
    XX(402, PAYMENT_REQUIRED, Payment Required)                               \
    XX(403, FORBIDDEN, Forbidden)                                             \
    XX(404, NOT_FOUND, Not Found)                                             \
    XX(405, METHOD_NOT_ALLOWED, Method Not Allowed)                           \
    XX(406, NOT_ACCEPTABLE, Not Acceptable)                                   \
    XX(407, PROXY_AUTHENTICATION_REQUIRED, Proxy Authentication Required)     \
    XX(408, REQUEST_TIMEOUT, Request Timeout)                                 \
    XX(409, CONFLICT, Conflict)                                               \
    XX(410, GONE, Gone)                                                       \
    XX(411, LENGTH_REQUIRED, Length Required)                                 \
    XX(412, PRECONDITION_FAILED, Precondition Failed)                         \
    XX(413, PAYLOAD_TOO_LARGE, Payload Too Large)                             \
    XX(414, URI_TOO_LONG, URI Too Long)                                       \
    XX(415, UNSUPPORTED_MEDIA_TYPE, Unsupported Media Type)                   \
    XX(416, RANGE_NOT_SATISFIABLE, Range Not Satisfiable)                     \
    XX(417, EXPECTATION_FAILED, Expectation Failed)                           \
    XX(421, MISDIRECTED_REQUEST, Misdirected Request)                         \
    XX(422, UNPROCESSABLE_ENTITY, Unprocessable Entity)                       \
    XX(423, LOCKED, Locked)                                                   \
    XX(424, FAILED_DEPENDENCY, Failed Dependency)                             \
    XX(426, UPGRADE_REQUIRED, Upgrade Required)                               \
    XX(428, PRECONDITION_REQUIRED, Precondition Required)                     \
    XX(429, TOO_MANY_REQUESTS, Too Many Requests)                             \
    XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
    XX(451, UNAVAILABLE_FOR_LEGAL_REASONS, Unavailable For Legal Reasons)     \
    XX(500, INTERNAL_SERVER_ERROR, Internal Server Error)                     \
    XX(501, NOT_IMPLEMENTED, Not Implemented)                                 \
    XX(502, BAD_GATEWAY, Bad Gateway)                                         \
    XX(503, SERVICE_UNAVAILABLE, Service Unavailable)                         \
    XX(504, GATEWAY_TIMEOUT, Gateway Timeout)                                 \
    XX(505, HTTP_VERSION_NOT_SUPPORTED, HTTP Version Not Supported)           \
    XX(506, VARIANT_ALSO_NEGOTIATES, Variant Also Negotiates)                 \
    XX(507, INSUFFICIENT_STORAGE, Insufficient Storage)                       \
    XX(508, LOOP_DETECTED, Loop Detected)                                     \
    XX(510, NOT_EXTENDED, Not Extended)                                       \
    XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required)

enum class HttpStatus
{
#define XX(num, name, string) name = num,
    HTTP_STATUS_MAP(XX)
#undef XX
};


/* Request Methods */
#define HTTP_METHOD_MAP(XX)          \
    XX(0, DELETE, DELETE)            \
    XX(1, GET, GET)                  \
    XX(2, HEAD, HEAD)                \
    XX(3, POST, POST)                \
    XX(4, PUT, PUT)                  \
    /* pathological */               \
    XX(5, CONNECT, CONNECT)          \
    XX(6, OPTIONS, OPTIONS)          \
    XX(7, TRACE, TRACE)              \
    /* WebDAV */                     \
    XX(8, COPY, COPY)                \
    XX(9, LOCK, LOCK)                \
    XX(10, MKCOL, MKCOL)             \
    XX(11, MOVE, MOVE)               \
    XX(12, PROPFIND, PROPFIND)       \
    XX(13, PROPPATCH, PROPPATCH)     \
    XX(14, SEARCH, SEARCH)           \
    XX(15, UNLOCK, UNLOCK)           \
    XX(16, BIND, BIND)               \
    XX(17, REBIND, REBIND)           \
    XX(18, UNBIND, UNBIND)           \
    XX(19, ACL, ACL)                 \
    /* subversion */                 \
    XX(20, REPORT, REPORT)           \
    XX(21, MKACTIVITY, MKACTIVITY)   \
    XX(22, CHECKOUT, CHECKOUT)       \
    XX(23, MERGE, MERGE)             \
    /* upnp */                       \
    XX(24, MSEARCH, M - SEARCH)      \
    XX(25, NOTIFY, NOTIFY)           \
    XX(26, SUBSCRIBE, SUBSCRIBE)     \
    XX(27, UNSUBSCRIBE, UNSUBSCRIBE) \
    /* RFC-5789 */                   \
    XX(28, PATCH, PATCH)             \
    XX(29, PURGE, PURGE)             \
    /* CalDAV */                     \
    XX(30, MKCALENDAR, MKCALENDAR)   \
    /* RFC-2068, section 19.6.1.2 */ \
    XX(31, LINK, LINK)               \
    XX(32, UNLINK, UNLINK)           \
    /* icecast */                    \
    XX(33, SOURCE, SOURCE)

enum class HttpMethod
{
#define XX(num, name, string) name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
        INVALID_METHOD = -1
};

HttpMethod http_method_from_string(const char* method);
const char* http_method_to_string(HttpMethod method);
const char* http_status_to_string(HttpStatus status);

struct CaseInsensitiveLess
{
    bool operator()(const std::string& a, const std::string& b) const {
        return strcasecmp(a.c_str(), b.c_str()) < 0;
    }
};

enum http_parser_type
{
    HTTP_REQUEST,
    HTTP_RESPONSE,
    HTTP_BOTH
};
class HttpRequest
{
public:
    typedef std::shared_ptr<HttpRequest> Ptr;
    typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;

    HttpRequest(std::string version = "HTTP/1.1", bool is_close = false);

    // getter
    std::string get_version() const { return m_version; }
    HttpMethod get_method() const { return m_method; }
    std::string get_path() const { return m_path; }
    std::string get_query() const { return m_query; }
    std::string get_body() const { return m_body; }
    std::string get_header(const std::string& key, const std::string& def = "");
    std::string get_param(const std::string& key, const std::string& def = "");
    std::string get_cookie(const std::string& key, const std::string& def = "");

    // setter
    void set_version(const std::string& version) { m_version = version; }
    void set_method(HttpMethod method) { m_method = method; }
    void set_path(const std::string& path) { m_path = path; }
    void set_query(const std::string& query) { m_query = query; }
    void set_body(const std::string& body) { m_body = body; }
    void set_header(const std::string& key, const std::string& value);
    void set_param(const std::string& key, const std::string& value);
    void set_cookie(const std::string& key, const std::string& value);

    // delete
    void del_header(const std::string& key);
    void del_param(const std::string& key);
    void del_cookie(const std::string& key);

    // has
    /**
     * 若存在对应键值，则返回true,同时设置value,否则返回false,不设置value
     */
    bool has_header(const std::string& key, std::string* value = nullptr);
    bool has_param(const std::string& key, std::string* value = nullptr);
    bool has_cookie(const std::string& key, std::string* value = nullptr);

    bool is_close() const { return m_is_close; }
    void set_close(bool is_close) { m_is_close = is_close; }

    std::string to_string() const;

    void init();
    void initParam();
    void initQueryParam();
    void initBodyParam();
    void initCookies();

private:
    bool m_is_close;
    HttpMethod m_method;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    std::string m_version;
    MapType m_headers;
    MapType m_params;
    MapType m_cookies;
    std::string m_body;
    uint8_t m_parserParamFlag;
};

class HttpResponse
{
public:
    typedef std::shared_ptr<HttpResponse> Ptr;
    typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;

    HttpResponse(std::string version = "HTTP/1.1", bool is_close = false);

    // getter
    std::string get_version() const { return m_version; }
    HttpStatus get_status() const { return m_status; }
    std::string get_body() const { return m_body; }
    std::string get_header(const std::string& key, const std::string& def = "");
    std::string get_reason() const { return m_reason; }

    // setter
    void set_version(const std::string& version) { m_version = version; }
    void set_status(HttpStatus status) { m_status = status; }
    void set_body(const std::string& body) { m_body = body; }
    void set_header(const std::string& key, const std::string& value);
    void set_reason(const std::string& reason) { m_reason = reason; }

    // delete
    void del_header(const std::string& key);

    bool is_close() const { return m_is_close; }
    void set_close(bool is_close) { m_is_close = is_close; }

    void set_redirect(const std::string& url);

    void set_cookie(const std::string& key, const std::string& val, time_t expired = 0,
                    const std::string& path = "", const std::string& domain = "",
                    bool secure = false);

    std::string to_string() const;



private:
    std::string m_version;
    HttpStatus m_status;
    MapType m_headers;
    std::string m_body;
    bool m_is_close;
    std::string m_reason;

    std::vector<std::string> m_cookies;
};

}   // namespace pico

#endif