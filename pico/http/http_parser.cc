#include "http_parser.h"
#include "../logging.h"
#include <boost/lexical_cast.hpp>
#include <iostream>


namespace pico {
const static uint64_t kHttpRequestBufferSize = 1024 * 1024;
const static uint64_t kHttpRequestMaxBodySize = 1024 * 1024;
const static uint64_t kHttpResponseBufferSize = 1024 * 1024;
const static uint64_t kHttpResponseMaxBodySize = 1024 * 1024;

uint64_t HttpRequestParser::getHttpRequestBufferSize() {
    return kHttpRequestBufferSize;
}

uint64_t HttpRequestParser::getHttpRequestMaxBodySize() {
    return kHttpRequestMaxBodySize;
}

uint64_t HttpResponseParser::getHttpResponseBufferSize() {
    return kHttpResponseBufferSize;
}

uint64_t HttpResponseParser::getHttpResponseMaxBodySize() {
    return kHttpResponseMaxBodySize;
}

/*
typedef void (*element_cb)(void* data, const char* at, size_t length);
typedef void (*field_cb)(void* data, const char* field, size_t flen, const char* value,
                         size_t vlen);
    field_cb http_field;
    element_cb request_method;
    element_cb request_uri;
    element_cb fragment;
    element_cb request_path;
    element_cb query_string;
    element_cb http_version;
    element_cb header_done;
*/

void on_request_method(void* data, const char* at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    HttpMethod method = http_method_from_string(std::string(at, length));
    parser->getRequest()->set_method(method);
}

void on_request_uri(void* data, const char* at, size_t length) {}

void on_request_fragment(void* data, const char* at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getRequest()->set_fragment(std::string(at, length));
}

void on_request_path(void* data, const char* at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getRequest()->set_path(std::string(at, length));
}

void on_request_query_string(void* data, const char* at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getRequest()->set_query(std::string(at, length));
}

void on_request_http_version(void* data, const char* at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getRequest()->set_version(std::string(at, length));
}

void on_request_header_done(void* data, const char* at, size_t length) {}

void on_request_http_field(void* data, const char* field, size_t flen, const char* value,
                           size_t vlen) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getRequest()->set_header(std::string(field, flen), std::string(value, vlen));
}

HttpRequestParser::HttpRequestParser() {
    m_request.reset(new HttpRequest());
    http_parser_init(&m_parser);
    m_parser.data = this;
    m_parser.http_field = on_request_http_field;
    m_parser.request_method = on_request_method;
    m_parser.request_uri = on_request_uri;
    m_parser.fragment = on_request_fragment;
    m_parser.request_path = on_request_path;
    m_parser.query_string = on_request_query_string;
    m_parser.http_version = on_request_http_version;
    m_parser.header_done = on_request_header_done;
}

HttpRequestParser::~HttpRequestParser() {}

int HttpRequestParser::parse(char* data, size_t len) {
    int offset = http_parser_execute(&m_parser, data, len, 0);
    if (http_parser_has_error(&m_parser)) {
        LOG_ERROR("http_parser_execute error");
        return -1;
    }
    memmove(data, data + offset, len - offset);
    return offset;
}

bool HttpRequestParser::isFinished() {
    return http_parser_is_finished(&m_parser);
}

int HttpRequestParser::hasError() {
    return http_parser_has_error(&m_parser);
}

void HttpRequestParser::reset() {
    http_parser_init(&m_parser);
}

uint64_t HttpRequestParser::getContentLength() {
    std::string ret = m_request->get_header("Content-Length", "0");
    return boost::lexical_cast<uint64_t>(ret);
}

/*
    typedef void (*element_cb)(void* data, const char* at, size_t length);
    typedef void (*field_cb)(void* data, const char* field, size_t flen, const char* value,
                         size_t vlen);
    field_cb http_field;
    element_cb reason_phrase;
    element_cb status_code;
    element_cb chunk_size;
    element_cb http_version;
    element_cb header_done;
    element_cb last_chunk;
*/

void on_response_reason_phrase(void* data, const char* at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    parser->getResponse()->set_reason(std::string(at, length));
}

void on_response_status_code(void* data, const char* at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    std::string status = std::string(at, length);
    parser->getResponse()->set_status((HttpStatus)std::atoi(status.c_str()));
}

void on_response_chunk_size(void* data, const char* at, size_t length) {}

void on_response_http_version(void* data, const char* at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    parser->getResponse()->set_version(std::string(at, length));
}

void on_response_header_done(void* data, const char* at, size_t length) {}

void on_response_last_chunk(void* data, const char* at, size_t length) {}

void on_response_http_field(void* data, const char* field, size_t flen, const char* value,
                            size_t vlen) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    parser->getResponse()->set_header(std::string(field, flen), std::string(value, vlen));
}

HttpResponseParser::HttpResponseParser() {
    m_response.reset(new HttpResponse());
    httpclient_parser_init(&m_parser);
    m_parser.data = this;
    m_parser.reason_phrase = on_response_reason_phrase;
    m_parser.status_code = on_response_status_code;
    m_parser.chunk_size = on_response_chunk_size;
    m_parser.http_version = on_response_http_version;
    m_parser.header_done = on_response_header_done;
    m_parser.last_chunk = on_response_last_chunk;
    m_parser.http_field = on_response_http_field;
}

int HttpResponseParser::parse(char* data, size_t len, bool chunk) {
    if (chunk) { httpclient_parser_init(&m_parser); }
    int offset = httpclient_parser_execute(&m_parser, data, len, 0);
    if (httpclient_parser_has_error(&m_parser)) {
        LOG_ERROR("http_parser_execute error");
        return -1;
    }
    memmove(data, data + offset, len - offset);
    return offset;
}

bool HttpResponseParser::isFinished() {
    return httpclient_parser_is_finished(&m_parser);
}

int HttpResponseParser::hasError() {
    return httpclient_parser_has_error(&m_parser);
}

void HttpResponseParser::reset() {
    httpclient_parser_init(&m_parser);
}

uint64_t HttpResponseParser::getContentLength() {
    std::string ret = m_response->get_header("Content-Length", "0");
    return boost::lexical_cast<uint64_t>(ret);
}

}   // namespace pico