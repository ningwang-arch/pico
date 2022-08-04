#ifndef __PICO_COMMON_H__
#define __PICO_COMMON_H__

#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

#include "http/http.h"
#include "util.h"

namespace pico {
using request = HttpRequest::Ptr;
using response = HttpResponse::Ptr;

struct routing_params
{
    std::vector<int64_t> int_params;
    std::vector<uint64_t> uint_params;
    std::vector<double> double_params;
    std::vector<std::string> string_params;

    template<typename T>
    T get(unsigned) const;
};

template<>
inline int64_t routing_params::get<int64_t>(unsigned index) const {
    return int_params[index];
}

template<>
inline uint64_t routing_params::get<uint64_t>(unsigned index) const {
    return uint_params[index];
}

template<>
inline double routing_params::get<double>(unsigned index) const {
    return double_params[index];
}

template<>
inline std::string routing_params::get<std::string>(unsigned index) const {
    return string_params[index];
}


template<class T>
using Invoke = typename T::type;

template<unsigned...>
struct seq
{ using type = seq; };

template<class S1, class S2>
struct concat;

template<unsigned... I1, unsigned... I2>
struct concat<seq<I1...>, seq<I2...>> : seq<I1..., (sizeof...(I1) + I2)...>
{};

template<class S1, class S2>
using Concat = Invoke<concat<S1, S2>>;

template<unsigned N>
struct gen_seq;
template<unsigned N>
using GenSeq = Invoke<gen_seq<N>>;

template<unsigned N>
struct gen_seq : Concat<GenSeq<N / 2>, GenSeq<N - N / 2>>
{};

template<>
struct gen_seq<0> : seq<>
{};
template<>
struct gen_seq<1> : seq<0>
{};

template<typename Seq, typename Tuple>
struct pop_back_helper;

template<unsigned... N, typename Tuple>
struct pop_back_helper<seq<N...>, Tuple>
{
    template<template<typename... Args> class U>
    using rebind = U<typename std::tuple_element<N, Tuple>::type...>;
};

template<typename... T>
struct pop_back
{
    template<template<typename... Args> class U>
    using rebind = typename pop_back_helper<typename gen_seq<sizeof...(T) - 1>::type,
                                            std::tuple<T...>>::template rebind<U>;
};

template<>
struct pop_back<>
{
    template<template<typename... Args> class U>
    using rebind = U<>;
};



template<typename... T>
struct S
{
    template<typename U>
    using push = S<U, T...>;
    template<typename U>
    using push_back = S<T..., U>;
    template<template<typename... Args> class U>
    using rebind = U<T...>;
};

template<typename F, typename Set>
struct CallHelper;
template<typename F, typename... Args>
struct CallHelper<F, S<Args...>>
{
    template<typename F1, typename... Args1,
             typename = decltype(std::declval<F1>()(std::declval<Args1>()...))>
    static char __test(int);

    template<typename...>
    static int __test(...);

    static constexpr bool value = sizeof(__test<F, Args...>(0)) == sizeof(char);
};

template<typename T>
struct promote
{ using type = T; };

#define PICO_INTERNAL_PROMOTE_TYPE(t1, t2) \
    template<>                             \
    struct promote<t1>                     \
    { using type = t2; };

PICO_INTERNAL_PROMOTE_TYPE(char, int64_t);
PICO_INTERNAL_PROMOTE_TYPE(short, int64_t);
PICO_INTERNAL_PROMOTE_TYPE(int, int64_t);
PICO_INTERNAL_PROMOTE_TYPE(long, int64_t);
PICO_INTERNAL_PROMOTE_TYPE(long long, int64_t);
PICO_INTERNAL_PROMOTE_TYPE(unsigned char, uint64_t);
PICO_INTERNAL_PROMOTE_TYPE(unsigned short, uint64_t);
PICO_INTERNAL_PROMOTE_TYPE(unsigned int, uint64_t);
PICO_INTERNAL_PROMOTE_TYPE(unsigned long, uint64_t);
PICO_INTERNAL_PROMOTE_TYPE(unsigned long long, uint64_t);
PICO_INTERNAL_PROMOTE_TYPE(float, double);
#undef PICO_INTERNAL_PROMOTE_TYPE

template<typename T>
using promote_t = typename pico::promote<T>::type;


routing_params find_routing_params(const request& req, const std::string& pattern) {
    std::string url = req->get_path();

    routing_params res;

    std::vector<std::string> str_parts;
    split(url, str_parts, "/");
    std::vector<std::string> pattern_parts;
    split(pattern, pattern_parts, "/");

    if (str_parts.size() != pattern_parts.size()) {
        return res;
    }

    for (int i = 0; i < (int)str_parts.size(); i++) {
        if (pattern_parts[i].at(0) == '<' &&
            pattern_parts[i].at(pattern_parts[i].size() - 1) == '>') {
            std::string type = pattern_parts[i].substr(1, pattern_parts[i].size() - 2);
            if (type == "int") {
                res.int_params.push_back(atoll(str_parts[i].c_str()));
            }
            else if (type == "uint") {
                res.uint_params.push_back(atoll(str_parts[i].c_str()));
            }
            else if (type == "string") {
                res.string_params.push_back(str_parts[i]);
            }
            else if (type == "double") {
                res.double_params.push_back(atof(str_parts[i].c_str()));
            }
            else {}
        }
    }
    return res;
}



namespace routing_handler_call_helper {

template<typename T, int Pos>
struct call_pair
{
    using type = T;
    static const int pos = Pos;
};

template<typename H1>
struct call_params
{
    H1& handler;
    const routing_params& params;
    const request& req;
    response& res;
};

template<typename F, int NInt, int NUint, int NDouble, int NString, typename S1, typename S2>
struct call
{};

template<typename F, int NInt, int NUint, int NDouble, int NString, typename... Args1,
         typename... Args2>
struct call<F, NInt, NUint, NDouble, NString, pico::S<int64_t, Args1...>, pico::S<Args2...>>
{
    void operator()(F cparams) {
        using pushed = typename pico::S<Args2...>::template push_back<call_pair<int64_t, NInt>>;
        call<F, NInt + 1, NUint, NDouble, NString, pico::S<Args1...>, pushed>()(cparams);
    }
};

template<typename F, int NInt, int NUint, int NDouble, int NString, typename... Args1,
         typename... Args2>
struct call<F, NInt, NUint, NDouble, NString, pico::S<uint64_t, Args1...>, pico::S<Args2...>>
{
    void operator()(F cparams) {
        using pushed = typename pico::S<Args2...>::template push_back<call_pair<uint64_t, NUint>>;
        call<F, NInt, NUint + 1, NDouble, NString, pico::S<Args1...>, pushed>()(cparams);
    }
};

template<typename F, int NInt, int NUint, int NDouble, int NString, typename... Args1,
         typename... Args2>
struct call<F, NInt, NUint, NDouble, NString, pico::S<double, Args1...>, pico::S<Args2...>>
{
    void operator()(F cparams) {
        using pushed = typename pico::S<Args2...>::template push_back<call_pair<double, NDouble>>;
        call<F, NInt, NUint, NDouble + 1, NString, pico::S<Args1...>, pushed>()(cparams);
    }
};

template<typename F, int NInt, int NUint, int NDouble, int NString, typename... Args1,
         typename... Args2>
struct call<F, NInt, NUint, NDouble, NString, pico::S<std::string, Args1...>, pico::S<Args2...>>
{
    void operator()(F cparams) {
        using pushed =
            typename pico::S<Args2...>::template push_back<call_pair<std::string, NString>>;
        call<F, NInt, NUint, NDouble, NString + 1, pico::S<Args1...>, pushed>()(cparams);
    }
};

template<typename F, int NInt, int NUint, int NDouble, int NString, typename... Args1>
struct call<F, NInt, NUint, NDouble, NString, pico::S<>, pico::S<Args1...>>
{
    void operator()(F cparams) {
        cparams.handler(cparams.req,
                        cparams.res,
                        cparams.params.template get<typename Args1::type>(Args1::pos)...);
    }
};



template<typename Func, typename... ArgsWrapped>
struct Wrapped
{

    // int string
    template<typename... Args>
    void set_(Func f,
              typename std::enable_if<
                  !std::is_same<typename std::tuple_element<0, std::tuple<Args..., void>>::type,
                                const request&>::value &&
                      !std::is_same<typename std::tuple_element<0, std::tuple<Args..., void>>::type,
                                    response&>::value,
                  int>::type = 0) {
        using return_t = typename function_traits<Func>::result_type;

        static_assert(!std::is_same<return_t, void>::value,
                      "Handler function cannot have void return type; valid return types: string");

        handler_ =
            ([f](const request&, response& res, Args... args) { res->set_body(f(args...)); });
    }


    template<typename Res, typename... Args>
    struct res_handler_wrapper
    {
        res_handler_wrapper(Func f)
            : f(std::move(f)) {}

        void operator()(const request& req, response& res, Args... args) { f(res, args...); }

        Func f;
    };


    template<typename Req, typename... Args>
    struct req_handler_wrapper
    {
        req_handler_wrapper(Func f)
            : f(std::move(f)) {}

        void operator()(const request& req, response& res, Args... args) {
            res->set_body(f(req, args...));
        }

        Func f;
    };

    // res int string
    template<typename... Args>
    void set_(Func f,
              typename std::enable_if<
                  std::is_same<typename std::tuple_element<0, std::tuple<Args..., void>>::type,
                               response&>::value,
                  int>::type = 0) {
        using return_t = typename function_traits<Func>::result_type;

        static_assert(std::is_same<return_t, void>::value,
                      "Handler function with response argument should have void return type");

        handler_ = res_handler_wrapper<Args...>(std::move(f));
    }

    // req int string
    template<typename... Args>
    void set_(
        Func f,
        typename std::enable_if<
            std::is_same<typename std::tuple_element<0, std::tuple<Args..., void>>::type,
                         const request&>::value &&
                !std::is_same<typename std::tuple_element<1, std::tuple<Args..., void, void>>::type,
                              response&>::value,
            int>::type = 0) {
        using return_t = typename function_traits<Func>::result_type;
        static_assert(!std::is_same<return_t, void>::value,
                      "Handler function cannot have void return type; valid return types: string");
        handler_ = req_handler_wrapper<Args...>(std::move(f));
    }

    // req res int string
    template<typename... Args>
    void
    set_(Func f,
         typename std::enable_if<
             std::is_same<typename std::tuple_element<0, std::tuple<Args..., void>>::type,
                          const request&>::value &&
                 std::is_same<typename std::tuple_element<1, std::tuple<Args..., void, void>>::type,
                              response&>::value,
             int>::type = 0) {
        handler_ = std::move(f);
    }

    template<typename... Args>
    struct handler_type_helper
    {
        using type = std::function<void(const request&, response&, Args...)>;

        using args_type = pico::S<typename pico::promote_t<Args>...>;
    };

    template<typename... Args>
    struct handler_type_helper<response&, Args...>
    {
        using type = std::function<void(const request&, response&, Args...)>;
        using args_type = pico::S<typename pico::promote_t<Args>...>;
    };

    template<typename... Args>
    struct handler_type_helper<const request&, Args...>
    {
        using type = std::function<void(const request&, response&, Args...)>;
        using args_type = pico::S<typename pico::promote_t<Args>...>;
    };

    template<typename... Args>
    struct handler_type_helper<const request&, response&, Args...>
    {
        using type = std::function<void(const request&, response&, Args...)>;
        using args_type = pico::S<typename pico::promote_t<Args>...>;
    };

    typename handler_type_helper<ArgsWrapped...>::type handler_;


    void operator()(const request& req, response& res, const routing_params& params) {
        routing_handler_call_helper::call<
            routing_handler_call_helper::call_params<decltype(handler_)>,
            0,
            0,
            0,
            0,
            typename handler_type_helper<ArgsWrapped...>::args_type,
            pico::S<>>()(routing_handler_call_helper::call_params<decltype(handler_)>{
            handler_, params, req, res});
    }
};

}   // namespace routing_handler_call_helper



template<typename Func, unsigned... Indices>
auto wrap(const std::string& pattern, Func f, seq<Indices...>)
    -> decltype(typename routing_handler_call_helper::Wrapped<
                Func, typename function_traits<Func>::template arg<Indices>...>()) {


    using function_t = function_traits<Func>;


    if (!is_parameter_tag_compatible(get_parameter_tag_runtime(pattern.c_str()),
                                     compute_parameter_tag_from_args_list<
                                         typename function_t::template arg<Indices>...>::value)) {
        throw std::runtime_error("Handler type is mismatched with URL parameters: " + pattern);
    }



    auto ret =
        routing_handler_call_helper::Wrapped<Func, typename function_t::template arg<Indices>...>();
    ret.template set_<typename function_t::template arg<Indices>...>(std::move(f));
    return ret;
}



template<typename Func>
void handle(const HttpRequest::Ptr& req, HttpResponse::Ptr& res, const std::string& pattern,
            Func func) {
    using function_t = function_traits<Func>;


    auto ret = wrap(pattern, std::move(func), gen_seq<function_t::arity>());

    auto rp = find_routing_params(req, pattern);

    ret(req, res, rp);
}

}   // namespace pico

#endif