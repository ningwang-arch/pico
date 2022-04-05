#ifndef __PICO_HTTP_MIDDLEWARE_CONTEXT_H__
#define __PICO_HTTP_MIDDLEWARE_CONTEXT_H__

#include "../util.h"
#include "http.h"

namespace pico {
template<typename... Middlewares>
struct partial_context : public pop_back<Middlewares...>::template rebind<partial_context>,
                         public last_element_type<Middlewares...>::type::context
{
    using parent_context =
        typename pop_back<Middlewares...>::template rebind<pico::partial_context>;
    template<int N>
    using partial = typename std::conditional<N == sizeof...(Middlewares) - 1, partial_context,
                                              typename parent_context::template partial<N>>::type;

    template<typename T>
    typename T::context& get() {
        return static_cast<typename T::context&>(*this);
    }
};



template<>
struct partial_context<>
{
    template<int>
    using partial = partial_context;
};


template<typename... Middlewares>
struct context : public partial_context<Middlewares...>
{
    template<template<typename QueryMW> class CallCriteria, int N, typename Context,
             typename Container>
    friend typename std::enable_if<(N == 0)>::type after_handlers_call_helper(
        Container& middlewares, Context& ctx, HttpRequest::Ptr& req, HttpResponse::Ptr& res);
    template<template<typename QueryMW> class CallCriteria, int N, typename Context,
             typename Container>
    friend typename std::enable_if<(N > 0)>::type after_handlers_call_helper(
        Container& middlewares, Context& ctx, HttpRequest::Ptr& req, HttpResponse::Ptr& res);

    template<template<typename QueryMW> class CallCriteria, int N, typename Context,
             typename Container>
    friend typename std::enable_if<
        (N < std::tuple_size<typename std::remove_reference<Container>::type>::value), bool>::type
    middleware_call_helper(Container& middlewares, HttpRequest::Ptr& req, HttpResponse::Ptr& res,
                           Context& ctx);

    template<typename T>
    typename T::context& get() {
        return static_cast<typename T::context&>(*this);
    }

    template<int N>
    using partial = typename partial_context<Middlewares...>::template partial<N>;
};
}   // namespace pico

#endif