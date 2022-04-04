#ifndef __PICO_HTTP_HTTP_SERVER_H__
#define __PICO_HTTP_HTTP_SERVER_H__

#include "../logging.h"
#include "../tcp_server.h"
#include "http.h"
#include "http_connection.h"
#include "pico/util.h"
#include "request_handler.h"
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace pico {
template<typename... Middlewares>
class HttpServer : public TcpServer
{
private:
    template<typename MW>
    struct check_global_call_false
    {
        template<typename T,
                 typename std::enable_if<T::call_global::value == false, bool>::type = true>
        struct get
        {};
    };

    template<typename MW>
    struct middleware_call_criteria_only_global
    {
        template<typename C>
        static std::false_type f(typename check_global_call_false<MW>::template get<C>*);

        template<typename C>
        static std::true_type f(...);

        static const bool value = decltype(f<MW>(nullptr))::value;
    };

public:
    typedef std::shared_ptr<HttpServer> Ptr;

    HttpServer(bool keepalive = false, IOManager* worker = IOManager::getThis(),
               IOManager* acceptor = IOManager::getThis(), std::tuple<Middlewares...>* mw = nullptr)
        : TcpServer(worker, acceptor)
        , m_request_handler(new RequestHandler())
        , m_is_KeepAlive(keepalive)
        , m_middlewares(mw) {
        if (m_middlewares == nullptr) m_middlewares = new std::tuple<Middlewares...>();
    };

    RequestHandler::Ptr getRequestHandler() const { return m_request_handler; }

    void setName(const std::string& name) override { TcpServer::setName(name); }

    void setRequestHandler(RequestHandler::Ptr& handler) { m_request_handler = handler; }

    template<typename T>
    typename T::context getContext() {
        static_assert(contains<T, Middlewares...>::value,
                      "App doesn't have the specified middleware type.");

        return T::ctx;
    }

    template<typename T>
    T& get_middleware() {
        return get_element_by_type<T, Middlewares...>(*m_middlewares);
    }


protected:
    void handleClient(Socket::Ptr& sock) override {
        LOG_INFO("handleClient");
        HttpConnection::Ptr conn(new HttpConnection(sock));
        do {
            auto req = conn->recvRequest();
            if (!req) {
                LOG_ERROR("recvRequest failed, close connection, sock: %s, errno=%d, %s",
                          sock->to_string().c_str(),
                          errno,
                          strerror(errno));
                break;
            }

            HttpResponse::Ptr resp(
                new HttpResponse(req->get_version(), req->is_close() || !m_is_KeepAlive));
            resp->set_header("Server", getName());
            middleware_call_helper<middleware_call_criteria_only_global,
                                   0,
                                   decltype(*m_middlewares)>(*m_middlewares, req, resp);

            m_request_handler->handle(req, resp);

            after_handlers_call_helper<middleware_call_criteria_only_global,
                                       (static_cast<int>(sizeof...(Middlewares)) - 1),
                                       decltype(*m_middlewares)>(*m_middlewares, req, resp);

            conn->sendResponse(resp);
            if (!m_is_KeepAlive || req->is_close()) { break; }
        } while (true);
        conn->close();
    }

private:
    template<typename MW>
    void before_handler_call(MW& mw, HttpRequest::Ptr& req, HttpResponse::Ptr& resp) {
        mw.before_handle(req, resp, mw.ctx);
    }

    template<typename MW>
    void after_handler_call(MW& mw, HttpRequest::Ptr& req, HttpResponse::Ptr& resp) {
        mw.after_handle(req, resp, mw.ctx);
    }

    template<template<typename QueryMW> class CallCriteria, int N, typename Container>
    typename std::enable_if<
        (N < std::tuple_size<typename std::remove_reference<Container>::type>::value), bool>::type
    middleware_call_helper(Container& middlewares, HttpRequest::Ptr& req, HttpResponse::Ptr& res) {

        using CurrentMW =
            typename std::tuple_element<N, typename std::remove_reference<Container>::type>::type;

        if (!CallCriteria<CurrentMW>::value) {

            return middleware_call_helper<CallCriteria, N + 1, Container>(middlewares, req, res);
        }

        before_handler_call<CurrentMW>(std::get<N>(middlewares), req, res);

        if (middleware_call_helper<CallCriteria, N + 1, Container>(middlewares, req, res)) {
            after_handler_call<CurrentMW>(std::get<N>(middlewares), req, res);
            return true;
        }

        return false;
    }

    template<template<typename QueryMW> class CallCriteria, int N, typename Container>
    typename std::enable_if<
        (N >= std::tuple_size<typename std::remove_reference<Container>::type>::value), bool>::type
    middleware_call_helper(Container& middlewares, HttpRequest::Ptr& req, HttpResponse::Ptr& res) {
        return false;
    }

    template<template<typename QueryMW> class CallCriteria, int N, typename Container>
    typename std::enable_if<(N < 0)>::type after_handlers_call_helper(Container& /*middlewares*/,
                                                                      HttpRequest::Ptr& /*req*/,
                                                                      HttpResponse::Ptr& /*res*/) {}

    template<template<typename QueryMW> class CallCriteria, int N, typename Container>
    typename std::enable_if<(N == 0)>::type after_handlers_call_helper(Container& middlewares,
                                                                       HttpRequest::Ptr& req,
                                                                       HttpResponse::Ptr& res) {
        using CurrentMW =
            typename std::tuple_element<N, typename std::remove_reference<Container>::type>::type;
        if (CallCriteria<CurrentMW>::value) {
            after_handler_call<CurrentMW>(std::get<N>(middlewares), req, res);
        }
    }

    template<template<typename QueryMW> class CallCriteria, int N, typename Container>
    typename std::enable_if<(N > 0)>::type after_handlers_call_helper(Container& middlewares,
                                                                      HttpRequest::Ptr& req,
                                                                      HttpResponse::Ptr& res) {
        using CurrentMW =
            typename std::tuple_element<N, typename std::remove_reference<Container>::type>::type;
        if (CallCriteria<CurrentMW>::value) {
            after_handler_call<CurrentMW>(std::get<N>(middlewares), req, res);
        }
        after_handlers_call_helper<CallCriteria, N - 1, Container>(middlewares, req, res);
    }

private:
    RequestHandler::Ptr m_request_handler;
    bool m_is_KeepAlive;
    std::tuple<Middlewares...>* m_middlewares;
};
}   // namespace pico

#endif