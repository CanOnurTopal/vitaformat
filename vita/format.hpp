// vita/format.hpp - portable C++11 string formatting
//
// Usage:
//   std::string s = Vita::format("Hello, {}!", name);
//   std::string s = Vita::format("{0} + {1} = {2}", a, b, a+b);
//
// Author: Can Onur Topal
// Originally written in 2022, current version released in 2025.
//
// MIT License - Copyright (c) 2022-2025 Can Onur Topal
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef VITA_FORMAT_HPP
#define VITA_FORMAT_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <limits>
#include <utility>

#ifndef VITA_FORMAT_ERROR_POLICY
#define VITA_FORMAT_ERROR_POLICY THROW
#endif

#ifndef VITA_FORMAT_MAX_ARGS
#define VITA_FORMAT_MAX_ARGS 32
#endif

#ifndef VITA_FORMAT_SBO_SIZE
#define VITA_FORMAT_SBO_SIZE 256
#endif

#include "detail/output.hpp"
#include "detail/int_to_str.hpp"
#include "detail/float_to_str.hpp"
#include "detail/parse.hpp"
#include "detail/compile_parse.hpp"

#if !defined(VITA_FORMAT_NO_EXCEPTIONS)
#include <stdexcept>
#endif

namespace Vita {

template <typename T, typename Enable = void>
struct Formatter;

namespace detail {

// type-erased argument storage
class FormatArg {
public:
    enum Type {
        NONE, BOOL, CHAR, INT, UINT, LLONG, ULLONG,
        DOUBLE, LDOUBLE, CSTRING, STRING, POINTER, CUSTOM
    };

    FormatArg() : type_(NONE) {}

    FormatArg(bool v) : type_(BOOL) { value_.bool_val = v; }
    FormatArg(char v) : type_(CHAR) { value_.char_val = v; }
    FormatArg(signed char v) : type_(INT) { value_.int_val = v; }
    FormatArg(unsigned char v) : type_(UINT) { value_.uint_val = v; }

    FormatArg(short v) : type_(INT) { value_.int_val = v; }
    FormatArg(unsigned short v) : type_(UINT) { value_.uint_val = v; }
    FormatArg(int v) : type_(INT) { value_.int_val = v; }
    FormatArg(unsigned int v) : type_(UINT) { value_.uint_val = v; }
    FormatArg(long v) : type_(LLONG) { value_.llong_val = v; }
    FormatArg(unsigned long v) : type_(ULLONG) { value_.ullong_val = v; }
    FormatArg(long long v) : type_(LLONG) { value_.llong_val = v; }
    FormatArg(unsigned long long v) : type_(ULLONG) { value_.ullong_val = v; }

    FormatArg(float v) : type_(DOUBLE) { value_.double_val = v; }
    FormatArg(double v) : type_(DOUBLE) { value_.double_val = v; }
    FormatArg(long double v) : type_(LDOUBLE) { value_.ldouble_val = v; }

    FormatArg(const char* v) : type_(CSTRING) { value_.cstring_val = v; }
    FormatArg(char* v) : type_(CSTRING) { value_.cstring_val = v; }
    FormatArg(const std::string& v) : type_(STRING) { value_.string_val = &v; }

    template <std::size_t N>
    FormatArg(const char (&v)[N]) : type_(CSTRING) { value_.cstring_val = v; }

    FormatArg(std::nullptr_t) : type_(POINTER) { value_.pointer_val = 0; }
    FormatArg(void* v) : type_(POINTER) { value_.pointer_val = v; }
    FormatArg(const void* v) : type_(POINTER) { value_.pointer_val = v; }

    template <typename T>
    FormatArg(T* v, typename std::enable_if<!std::is_same<T, char>::value &&
                                             !std::is_same<T, void>::value>::type* = 0)
        : type_(POINTER) { value_.pointer_val = static_cast<const void*>(v); }

    Type type() const { return type_; }

    bool as_bool() const { return value_.bool_val; }
    char as_char() const { return value_.char_val; }
    int as_int() const { return value_.int_val; }
    unsigned int as_uint() const { return value_.uint_val; }
    long long as_llong() const { return value_.llong_val; }
    unsigned long long as_ullong() const { return value_.ullong_val; }
    double as_double() const { return value_.double_val; }
    long double as_ldouble() const { return value_.ldouble_val; }
    const char* as_cstring() const { return value_.cstring_val; }
    const std::string* as_string() const { return value_.string_val; }
    const void* as_pointer() const { return value_.pointer_val; }

private:
    Type type_;
    union Value {
        bool bool_val;
        char char_val;
        int int_val;
        unsigned int uint_val;
        long long llong_val;
        unsigned long long ullong_val;
        double double_val;
        long double ldouble_val;
        const char* cstring_val;
        const std::string* string_val;
        const void* pointer_val;
        Value() : pointer_val(0) {}
    } value_;
};

inline void apply_format_spec(FormatOutput& out, const char* content, std::size_t len, const FormatSpec& spec) {
    if (spec.width <= 0 || len >= static_cast<std::size_t>(spec.width)) {
        out.append(content, len);
        return;
    }

    std::size_t padding = static_cast<std::size_t>(spec.width) - len;
    char fill = spec.fill;
    char align = spec.align ? spec.align : '<';

    switch (align) {
    case '<':
        out.append(content, len);
        out.append_fill(fill, padding);
        break;
    case '>':
        out.append_fill(fill, padding);
        out.append(content, len);
        break;
    case '^': {
        std::size_t left_pad = padding / 2;
        out.append_fill(fill, left_pad);
        out.append(content, len);
        out.append_fill(fill, padding - left_pad);
        break;
    }
    case '=':
        // sign-aware padding
        if (len > 0 && (content[0] == '-' || content[0] == '+' || content[0] == ' ')) {
            out.append(content[0]);
            out.append_fill(fill, padding);
            out.append(content + 1, len - 1);
        } else {
            out.append_fill(fill, padding);
            out.append(content, len);
        }
        break;
    default:
        out.append(content, len);
    }
}

inline void format_arg(FormatOutput& out, const FormatArg& arg, const FormatSpec& spec) {
    char buffer[128];
    std::size_t len = 0;

    switch (arg.type()) {
    case FormatArg::NONE:
        out.append("{?}", 3);
        return;

    case FormatArg::BOOL:
        if (spec.type == 'd') {
            buffer[0] = arg.as_bool() ? '1' : '0';
            len = 1;
        } else {
            const char* str = arg.as_bool() ? "true" : "false";
            apply_format_spec(out, str, arg.as_bool() ? 4 : 5, spec);
            return;
        }
        break;

    case FormatArg::CHAR:
        if (spec.type == 'd' || spec.type == 'x' || spec.type == 'X' ||
            spec.type == 'o' || spec.type == 'b') {
            int val = static_cast<unsigned char>(arg.as_char());
            if (spec.type == 'x')
                len = uint_to_hex(static_cast<unsigned>(val), buffer, false);
            else if (spec.type == 'X')
                len = uint_to_hex(static_cast<unsigned>(val), buffer, true);
            else if (spec.type == 'o')
                len = uint_to_oct(static_cast<unsigned>(val), buffer);
            else if (spec.type == 'b')
                len = uint_to_bin(static_cast<unsigned>(val), buffer);
            else
                len = int_to_str(val, buffer);
        } else {
            buffer[0] = arg.as_char();
            len = 1;
        }
        break;

    case FormatArg::INT: {
        int val = arg.as_int();
        if (spec.type == 'x') {
            if (val < 0) {
                buffer[0] = '-';
                len = 1 + uint_to_hex(static_cast<unsigned>(-val), buffer + 1, false);
            } else {
                len = uint_to_hex(static_cast<unsigned>(val), buffer, false);
            }
        } else if (spec.type == 'X') {
            if (val < 0) {
                buffer[0] = '-';
                len = 1 + uint_to_hex(static_cast<unsigned>(-val), buffer + 1, true);
            } else {
                len = uint_to_hex(static_cast<unsigned>(val), buffer, true);
            }
        } else if (spec.type == 'o') {
            len = uint_to_oct(static_cast<unsigned>(val < 0 ? -val : val), buffer);
        } else if (spec.type == 'b') {
            len = uint_to_bin(static_cast<unsigned>(val < 0 ? -val : val), buffer);
        } else {
            len = int_to_str(val, buffer);
            if (spec.sign == '+' && val >= 0) {
                std::memmove(buffer + 1, buffer, len);
                buffer[0] = '+';
                len++;
            } else if (spec.sign == ' ' && val >= 0) {
                std::memmove(buffer + 1, buffer, len);
                buffer[0] = ' ';
                len++;
            }
        }
        break;
    }

    case FormatArg::UINT: {
        unsigned val = arg.as_uint();
        if (spec.type == 'x')
            len = uint_to_hex(val, buffer, false);
        else if (spec.type == 'X')
            len = uint_to_hex(val, buffer, true);
        else if (spec.type == 'o')
            len = uint_to_oct(val, buffer);
        else if (spec.type == 'b')
            len = uint_to_bin(val, buffer);
        else {
            len = uint_to_str(val, buffer);
            if (spec.sign == '+') {
                std::memmove(buffer + 1, buffer, len);
                buffer[0] = '+';
                len++;
            } else if (spec.sign == ' ') {
                std::memmove(buffer + 1, buffer, len);
                buffer[0] = ' ';
                len++;
            }
        }
        break;
    }

    case FormatArg::LLONG: {
        long long val = arg.as_llong();
        if (spec.type == 'x') {
            if (val < 0) {
                buffer[0] = '-';
                len = 1 + uint_to_hex(static_cast<unsigned long long>(-val), buffer + 1, false);
            } else {
                len = uint_to_hex(static_cast<unsigned long long>(val), buffer, false);
            }
        } else if (spec.type == 'X') {
            if (val < 0) {
                buffer[0] = '-';
                len = 1 + uint_to_hex(static_cast<unsigned long long>(-val), buffer + 1, true);
            } else {
                len = uint_to_hex(static_cast<unsigned long long>(val), buffer, true);
            }
        } else if (spec.type == 'o') {
            len = uint_to_oct(static_cast<unsigned long long>(val < 0 ? -val : val), buffer);
        } else if (spec.type == 'b') {
            len = uint_to_bin(static_cast<unsigned long long>(val < 0 ? -val : val), buffer);
        } else {
            len = int_to_str(val, buffer);
            if (spec.sign == '+' && val >= 0) {
                std::memmove(buffer + 1, buffer, len);
                buffer[0] = '+';
                len++;
            } else if (spec.sign == ' ' && val >= 0) {
                std::memmove(buffer + 1, buffer, len);
                buffer[0] = ' ';
                len++;
            }
        }
        break;
    }

    case FormatArg::ULLONG: {
        unsigned long long val = arg.as_ullong();
        if (spec.type == 'x')
            len = uint_to_hex(val, buffer, false);
        else if (spec.type == 'X')
            len = uint_to_hex(val, buffer, true);
        else if (spec.type == 'o')
            len = uint_to_oct(val, buffer);
        else if (spec.type == 'b')
            len = uint_to_bin(val, buffer);
        else {
            len = uint_to_str(val, buffer);
            if (spec.sign == '+') {
                std::memmove(buffer + 1, buffer, len);
                buffer[0] = '+';
                len++;
            } else if (spec.sign == ' ') {
                std::memmove(buffer + 1, buffer, len);
                buffer[0] = ' ';
                len++;
            }
        }
        break;
    }

    case FormatArg::DOUBLE: {
        double val = arg.as_double();
        int prec = spec.precision >= 0 ? spec.precision : 6;
        if (spec.type == 'f' || spec.type == 'F')
            len = double_to_str_fixed(val, buffer, prec);
        else if (spec.type == 'e')
            len = double_to_str_scientific(val, buffer, prec, false);
        else if (spec.type == 'E')
            len = double_to_str_scientific(val, buffer, prec, true);
        else
            len = double_to_str_shortest(val, buffer, spec.precision);
        break;
    }

    case FormatArg::LDOUBLE: {
        long double val = arg.as_ldouble();
        int prec = spec.precision >= 0 ? spec.precision : 6;
        if (spec.type == 'f' || spec.type == 'F')
            len = ldouble_to_str_fixed(val, buffer, prec);
        else if (spec.type == 'e')
            len = ldouble_to_str_scientific(val, buffer, prec, false);
        else if (spec.type == 'E')
            len = ldouble_to_str_scientific(val, buffer, prec, true);
        else
            len = ldouble_to_str_shortest(val, buffer, spec.precision);
        break;
    }

    case FormatArg::CSTRING: {
        const char* str = arg.as_cstring();
        if (!str) {
            apply_format_spec(out, "(null)", 6, spec);
            return;
        }
        std::size_t str_len = std::strlen(str);
        if (spec.precision >= 0 && static_cast<std::size_t>(spec.precision) < str_len)
            str_len = static_cast<std::size_t>(spec.precision);
        apply_format_spec(out, str, str_len, spec);
        return;
    }

    case FormatArg::STRING: {
        const std::string* str = arg.as_string();
        std::size_t str_len = str->size();
        if (spec.precision >= 0 && static_cast<std::size_t>(spec.precision) < str_len)
            str_len = static_cast<std::size_t>(spec.precision);
        apply_format_spec(out, str->data(), str_len, spec);
        return;
    }

    case FormatArg::POINTER:
        len = ptr_to_str(arg.as_pointer(), buffer);
        break;

    case FormatArg::CUSTOM:
        out.append("{custom}", 8);
        return;
    }

    // zero padding for numbers
    FormatSpec adjusted_spec = spec;
    if (spec.zero_pad && spec.width > 0 && adjusted_spec.align == '\0') {
        adjusted_spec.fill = '0';
        adjusted_spec.align = '=';
    }

    apply_format_spec(out, buffer, len, adjusted_spec);
}

inline std::string format_impl(const char* fmt, std::size_t fmt_len,
                               const FormatArg* args, std::size_t num_args) {
    FormatOutput out;
    out.reserve(fmt_len + num_args * 16);

    FormatParser parser(fmt, fmt_len);

    for (;;) {
        ParseSegment seg = parser.next();

        switch (seg.type) {
        case ParseSegment::LITERAL:
            out.append(seg.begin, static_cast<std::size_t>(seg.end - seg.begin));
            break;

        case ParseSegment::PLACEHOLDER: {
            int idx = seg.placeholder.arg_index;
            if (idx >= 0 && static_cast<std::size_t>(idx) < num_args)
                format_arg(out, args[idx], seg.placeholder.spec);
            else
                out.append("{?}", 3);
            break;
        }

        case ParseSegment::ESCAPE_OPEN:
            out.append('{');
            break;

        case ParseSegment::ESCAPE_CLOSE:
            out.append('}');
            break;

        case ParseSegment::END:
            return out.finish();

        case ParseSegment::ERROR:
#if !defined(VITA_FORMAT_NO_EXCEPTIONS)
            throw std::runtime_error("Vita::format: invalid format string");
#else
            out.append("{error}", 7);
#endif
            break;
        }
    }

    return out.finish();
}

inline void pack_args(FormatArg*) {}

template <typename T, typename... Rest>
inline void pack_args(FormatArg* dest, T&& arg, Rest&&... rest) {
    *dest = FormatArg(std::forward<T>(arg));
    pack_args(dest + 1, std::forward<Rest>(rest)...);
}

} // namespace detail

//
// Public API
//

template <typename... Args>
std::string format(const char* fmt, Args&&... args) {
    detail::FormatArg arg_array[sizeof...(Args) > 0 ? sizeof...(Args) : 1];
    detail::pack_args(arg_array, std::forward<Args>(args)...);
    return detail::format_impl(fmt, std::strlen(fmt), arg_array, sizeof...(Args));
}

template <typename... Args>
std::string format(const std::string& fmt, Args&&... args) {
    detail::FormatArg arg_array[sizeof...(Args) > 0 ? sizeof...(Args) : 1];
    detail::pack_args(arg_array, std::forward<Args>(args)...);
    return detail::format_impl(fmt.data(), fmt.size(), arg_array, sizeof...(Args));
}

inline std::string format(const char* fmt) {
    return detail::format_impl(fmt, std::strlen(fmt), 0, 0);
}

inline std::string format(const std::string& fmt) {
    return detail::format_impl(fmt.data(), fmt.size(), 0, 0);
}

// formatc - compile-time optimized version
// template instantiation per format string allows better inlining
template <std::size_t N, typename... Args>
std::string formatc(const char (&fmt)[N], Args&&... args) {
    detail::FormatArg arg_array[sizeof...(Args) > 0 ? sizeof...(Args) : 1];
    detail::pack_args(arg_array, std::forward<Args>(args)...);
    return detail::format_impl(fmt, N - 1, arg_array, sizeof...(Args));
}

template <std::size_t N>
std::string formatc(const char (&fmt)[N]) {
    return detail::format_impl(fmt, N - 1, 0, 0);
}

#define VITA_FORMAT(fmt, ...) ::Vita::formatc(fmt, ##__VA_ARGS__)

// Formatter extension point
template <typename T, typename Enable>
struct Formatter {
    static_assert(sizeof(T) == 0,
        "Vita::format - no Formatter specialization for this type");
};

template <typename T>
struct Formatter<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    static void format(detail::FormatOutput& out, const T& value, const detail::FormatSpec& spec) {
        typedef typename std::underlying_type<T>::type underlying;
        detail::FormatArg arg(static_cast<underlying>(value));
        detail::format_arg(out, arg, spec);
    }
};

} // namespace Vita

#endif // VITA_FORMAT_HPP
