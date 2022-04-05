#ifndef __PICO_UTIL_H__
#define __PICO_UTIL_H__

#include <pthread.h>
#include <string>

#include "fiber.h"

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

template<typename T, typename Tuple>
struct has_type;

template<typename T>
struct has_type<T, std::tuple<>> : std::false_type
{};

template<typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>>
{};

template<typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type
{};

// from http://stackoverflow.com/questions/2118541/check-if-c0x-parameter-pack-contains-a-type
template<typename Tp, typename... List>
struct contains : std::true_type
{};

template<typename Tp, typename Head, typename... Rest>
struct contains<Tp, Head, Rest...>
    : std::conditional<std::is_same<Tp, Head>::value, std::true_type, contains<Tp, Rest...>>::type
{};

template<typename Tp>
struct contains<Tp> : std::false_type
{};

template<class T, std::size_t N, class... Args>
struct get_index_of_element_from_tuple_by_type_impl
{ static constexpr auto value = N; };

template<class T, std::size_t N, class... Args>
struct get_index_of_element_from_tuple_by_type_impl<T, N, T, Args...>
{ static constexpr auto value = N; };

template<class T, std::size_t N, class U, class... Args>
struct get_index_of_element_from_tuple_by_type_impl<T, N, U, Args...>
{
    static constexpr auto value =
        get_index_of_element_from_tuple_by_type_impl<T, N + 1, Args...>::value;
};

template<class T, class... Args>
T& get_element_by_type(std::tuple<Args...>& t) {
    return std::get<get_index_of_element_from_tuple_by_type_impl<T, 0, Args...>::value>(t);
}


template<typename... T>
struct last_element_type
{ using type = typename std::tuple_element<sizeof...(T) - 1, std::tuple<T...>>::type; };


template<>
struct last_element_type<>
{};

// from
// http://stackoverflow.com/questions/13072359/c11-compile-time-array-with-logarithmic-evaluation-depth
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
struct pop_back   //: public pop_back_helper<typename gen_seq<sizeof...(T)-1>::type,
                  //: std::tuple<T...>>
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

}   // namespace pico



#endif