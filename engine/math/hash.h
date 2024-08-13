#pragma once

#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

#define PRIME_NUM 0x9e3779b9

constexpr size_t _Hash(char const * str, size_t seed)
{
    return 0 == *str ? seed : _Hash(str + 1, seed ^ (*str + PRIME_NUM + (seed << 6) + (seed >> 2)));
}
#define CT_HASH(x) (_Hash(x, 0))

template <typename T>
inline void HashCombineImpl(T& seed, T value)
{
    seed ^= value + PRIME_NUM + (seed << 6) + (seed >> 2);
}

inline size_t RT_HASH(char const* str)
{
    size_t seed = 0;
    while (*str != 0)
    {
        HashCombineImpl(seed, (size_t)*str);
        str++;
    }
    return seed;
}

#undef PRIME_NUM

template <typename T>
inline size_t HashValue(T v)
{
    return static_cast<size_t>(v);
}

template <typename T>
inline size_t HashValue(T* v)
{
    return static_cast<size_t>(reinterpret_cast<int>(v));
}

template <typename T>
inline void HashCombine(size_t& seed, T const & v)
{
    return HashCombineImpl(seed, HashValue(v));
}

template <typename T>
inline void HashRange(size_t& seed, T first, T last)
{
    for (; first != last; ++first)
    {
        HashCombine(seed, *first);
    }
}

template <typename T>
inline size_t HashRange(T first, T last)
{
    size_t seed = 0;
    HashRange(seed, first, last);
    return seed;
}

#define STRUCT_HASH() \
    char* begin = (char*)(this); \
    char* end = begin + sizeof(*this); \
    return HashRange(begin, end); \


SEEK_NAMESPACE_END
