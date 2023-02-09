#pragma once

#include <cstring>
#include <algorithm>
#include <utility>
#include <iterator>
#include <string_view>
#include <type_traits>

#include <cpputils/templateutils.h>

template<typename SEQUENCE, typename CHAR>
bool is_in(const SEQUENCE& trims, CHAR c)
{
    if constexpr (is_searchable_v<SEQUENCE>)
        return trims.find(c) != trims.end();
    return std::find(std::begin(trims), std::end(trims), c) != std::end(trims);
}
template<typename CHAR>
bool is_in(const char*trims, CHAR c)
{
    return std::strchr(trims, c) != NULL;
}
template<typename CHAR>
bool is_in(char trims, CHAR c)
{
    return trims == c;
}



// strip algorithms taking first, last pointer pairs.
//
//note: i used to have identical names for the predicate variants,
//and select the function by using:
// std::enable_if_t<is_callable_v<PRED>, P>
// std::enable_if_t<!std::is_void_v<typename S::value_type>, P> 
//
// where 'std::is_void_v<typename S::value_type>'  means: 'is_container'


template<typename P, typename PRED>
P rstrip_if(P first, P last, PRED pred)
{
    auto i = last;

    while (i > first)
    {
        i--;
        if (!pred(*i))
            return i+1;
    }
    return first;
}

template<typename P, typename S>
P rstrip(P first, P last, const S& trims)
{
    return rstrip_if(first, last, [&](auto c) { return is_in<S>(trims, c); });
}


template<typename P, typename PRED>
P lstrip_if(P first, P last, PRED pred)
{
    auto i = first;

    while (i < last)
    {
        if (!pred(*i))
            return i;
        i++;
    }
    return last;
}

template<typename P, typename S>
P lstrip(P first, P last, const S& trims)
{
    return lstrip_f(first, last, [&](auto c) { return is_in<S>(trims, c); });
}

template<typename P, typename PRED>
std::pair<P,P> strip_if(P first, P last, PRED pred)
{
    auto l = lstrip_if(first, last, pred);
    auto r = rstrip_if(first, last, pred);
    return {l, r};
}

template<typename P, typename S>
std::pair<P,P> strip(P first, P last, const S& trims)
{
    return strip_if(first, last, [&](auto c) { return is_in<S>(trims, c); });
}


template<typename T, typename P>
T makeresult(P first, P last)
{
    if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::wstring_view>)
        return {first, typename T::size_type(last-first)};
    else
        return {first, last};
}

// adaptors for stl types

template<typename T, typename PRED>
T rstrip_if(const T& str, PRED pred)
{
    return makeresult<T>(std::begin(str), rstrip_if(std::begin(str), std::end(str), pred));
}

template<typename T, typename S>
T rstrip(const T& str, const S& trims)
{
    return rstrip_if(str, [&](auto c) { return is_in<S>(trims, c); });
}


template<typename T, typename PRED>
T lstrip_if(const T& str, PRED pred)
{
    return makeresult<T>(lstrip_if(std::begin(str), std::end(str), pred), std::end(str));
}

template<typename T, typename S>
T lstrip(const T& str, const S& trims)
{
    return lstrip_if(str, [&](auto c) { return is_in<S>(trims, c); });
}

template<typename T, typename PRED>
T strip_if(const T& str, PRED pred)
{
    auto l = lstrip_if(std::begin(str), std::end(str), pred);
    auto r = rstrip_if(std::begin(str), std::end(str), pred);

    return makeresult<T>(l, r);
}

template<typename T, typename S>
T strip(const T& str, const S& trims)
{
    return strip_if(str, [&](auto c) { return is_in<S>(trims, c); });
}



