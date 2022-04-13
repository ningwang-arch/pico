#ifndef __PICO_HTTP_HTTP_PARSER_H__
#define __PICO_HTTP_HTTP_PARSER_H__

#include "http.h"
#include "http11_common.h"
#include "http11_parser.h"
#include "httpclient_parser.h"
#include <string>
#include <vector>

namespace pico {
class HttpRequestParser
{
public:
    typedef std::shared_ptr<HttpRequestParser> Ptr;
    HttpRequestParser();
    virtual ~HttpRequestParser();

    virtual int parse(char* data, size_t len);

    virtual bool isFinished();
    virtual int hasError();

    virtual HttpRequest::Ptr getRequest() { return m_request; }
    virtual http_parser& getParser() { return m_parser; }

    virtual void reset();

    uint64_t getContentLength();


public:
    static uint64_t getHttpRequestBufferSize();
    static uint64_t getHttpRequestMaxBodySize();

private:
    http_parser m_parser;
    HttpRequest::Ptr m_request;
};

class HttpResponseParser
{
public:
    typedef std::shared_ptr<HttpResponseParser> Ptr;
    HttpResponseParser();

    virtual int parse(char* data, size_t len, bool chunk);

    virtual bool isFinished();
    virtual int hasError();

    virtual HttpResponse::Ptr getResponse() { return m_response; }
    virtual httpclient_parser& getParser() { return m_parser; }

    virtual void reset();

    uint64_t getContentLength();

public:
    static uint64_t getHttpResponseBufferSize();
    static uint64_t getHttpResponseMaxBodySize();

private:
    httpclient_parser m_parser;
    HttpResponse::Ptr m_response;
};

}   // namespace pico

#endif