#ifndef VITA_FORMAT_ENSURE_FSTRING_HPP_
#define VITA_FORMAT_ENSURE_FSTRING_HPP_

#include <cstddef>

#if __cplusplus >= 201402L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)
#define VITA_FORMAT_ENSURE_FSTRING_AVAILABLE_ 1
#else
#define VITA_FORMAT_ENSURE_FSTRING_AVAILABLE_ 0
#endif

#if VITA_FORMAT_ENSURE_FSTRING_AVAILABLE_

namespace Vita { namespace detail { namespace strsyn {

constexpr bool is_align(char c) {
    return c == '<' || c == '>' || c == '^' || c == '=';
}

constexpr bool is_sign(char c) {
    return c == '+' || c == '-' || c == ' ';
}

constexpr bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

constexpr bool is_type(char c) {
    return c == 'd' || c == 'x' || c == 'X' || c == 'o' || c == 'b'
        || c == 'f' || c == 'F' || c == 'e' || c == 'E'
        || c == 's' || c == 'c' || c == 'p'
        || c == 'g' || c == 'G' || c == 'a' || c == 'A';
}

constexpr int parse_format_spec(const char* s, std::size_t n, std::size_t& i) {
    if (i >= n || s[i] == '}') {
        return 0;
    }
    if (i + 1 < n && is_align(s[i + 1])) {
        i += 2;
    } else if (is_align(s[i])) {
        i += 1;
    }
    if (i < n && is_sign(s[i])) ++i;
    if (i < n && s[i] == '#') ++i;
    if (i < n && s[i] == '0') ++i;
    while (i < n && is_digit(s[i])) ++i;
    if (i < n && s[i] == '.') {
        ++i;
        while (i < n && is_digit(s[i])) ++i;
    }
    if (i < n && is_type(s[i])) ++i;
    return 0;
}

constexpr int validate(const char* s, std::size_t n) {
    std::size_t i = 0;
    while (i < n) {
        if (s[i] == '{') {
            if (i + 1 < n && s[i + 1] == '{') {
                i += 2;
                continue;
            }
            ++i;
            while (i < n && is_digit(s[i])) ++i;
            if (i >= n)            return 1;
            if (s[i] == '}') { ++i; continue; }

            if (s[i] != ':')       return 2;
            ++i;
            int err = parse_format_spec(s, n, i);
            if (err) return err;
            if (i >= n || s[i] != '}') return 3;
            ++i;
        } else if (s[i] == '}') {
            if (i + 1 < n && s[i + 1] == '}') {
                i += 2;
            } else {
                ++i;
            }
        } else {
            ++i;
        }
    }
    return 0;
}

#ifndef VITA_COMPILE_TIME_DEBUG

template<int ErrCode>
struct Check {
    static_assert(ErrCode == 0,
        "VitaFormatter: Invalid format string syntax — "
        "check for unclosed '{', invalid format specifier, "
        "or unexpected characters in a replacement field.");
};

#endif

}}}

#ifdef VITA_COMPILE_TIME_DEBUG

#define VITA_FORMAT_ENSURE_FSTRING(str)                                          \
    (::Vita::detail::strsyn::validate((str), sizeof(str) - 1) == 0           \
        ? (str)                                                              \
        : "__VITA_SYNTAX_ERROR__")

#else

#define VITA_FORMAT_ENSURE_FSTRING(str)                                          \
    ((void)sizeof(::Vita::detail::strsyn::Check<                             \
        ::Vita::detail::strsyn::validate((str), sizeof(str) - 1)>),          \
    (str))

#endif

#else

#define VITA_FORMAT_ENSURE_FSTRING(str) (str)

#endif

//Provides the user with the ability to verify that a format string is correct at compile time.
#define VITA_CFSTRING(str) VITA_FORMAT_ENSURE_FSTRING(str)

#undef VITA_FORMAT_ENSURE_FSTRING_AVAILABLE_

#endif
