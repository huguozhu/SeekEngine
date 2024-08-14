#pragma once

#include <string>
#include <vector>
#include <assert.h>
#include "error.h"
#include "safe_c.h"
#include "seek.config.h"

USING_SSB_NAMESPACE

SEEK_NAMESPACE_BEGIN

// Assert
#define SEEK_ASSERT assert

#define seek_alignup(x, a)   ( ((x) + (a) - 1) & (~((a)-1)) )
#define seek_aligndown(x, a) ( (x) & (~((a)-1)) )
#define seek_alignaddr(x, a) ( ((intptr_t)(x) + ((intptr_t)(a) - (intptr_t)1)) & ((intptr_t)-(a)) )

// String function
std::string                 string_trim     (std::string const& str);
std::vector<std::string>    string_split    (std::string const& str, char delimiter = ',');
void                        string_replace  (std::string & str, std::string const& from, std::string const& to);

// Type Convert
template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

template<typename T>
struct default_array_deleter
{
    void operator ()(T const* p)
    {
        delete[] p;
    }
};

#if defined(SEEK_PLATFORM_WINDOWS)
std::wstring utf8_to_wchar(const char* src);
#endif

// Export
#ifndef _WIN32
    #define SEEK_EXPORT __attribute__((visibility("default")))
#else
    #define SEEK_EXPORT
#endif

SEEK_NAMESPACE_END
