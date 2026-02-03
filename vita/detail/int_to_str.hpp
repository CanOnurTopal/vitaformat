// vita/detail/int_to_str.hpp
// fast integer conversion using digit pair tables
#ifndef VITA_DETAIL_INT_TO_STR_HPP
#define VITA_DETAIL_INT_TO_STR_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace Vita {
namespace detail {

// digit pair table - avoids division per digit
inline const char* digit_pairs() {
    static const char table[201] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";
    return table;
}

inline const char* hex_digits_lower() {
    static const char table[17] = "0123456789abcdef";
    return table;
}

inline const char* hex_digits_upper() {
    static const char table[17] = "0123456789ABCDEF";
    return table;
}

template <typename T>
inline unsigned count_digits(T n) {
    unsigned count = 1;
    for (;;) {
        if (n < 10) return count;
        if (n < 100) return count + 1;
        if (n < 1000) return count + 2;
        if (n < 10000) return count + 3;
        n /= 10000u;
        count += 4;
    }
}

// write digits backward from end, return start ptr
template <typename T>
inline char* uint_to_str_backward(T value, char* end) {
    const char* digits = digit_pairs();
    char* ptr = end;

    while (value >= 100) {
        unsigned idx = static_cast<unsigned>((value % 100) * 2);
        value /= 100;
        *--ptr = digits[idx + 1];
        *--ptr = digits[idx];
    }

    if (value >= 10) {
        unsigned idx = static_cast<unsigned>(value * 2);
        *--ptr = digits[idx + 1];
        *--ptr = digits[idx];
    } else {
        *--ptr = static_cast<char>('0' + value);
    }

    return ptr;
}

template <typename T>
inline std::size_t uint_to_str(T value, char* buffer) {
    char temp[24]; // enough for uint64
    char* end = temp + sizeof(temp);
    char* start = uint_to_str_backward(value, end);
    std::size_t len = static_cast<std::size_t>(end - start);
    std::memcpy(buffer, start, len);
    return len;
}

template <typename T>
inline std::size_t int_to_str(T value, char* buffer) {
    typedef typename std::make_unsigned<T>::type UnsignedT;

    if (value < 0) {
        *buffer = '-';
        // cast handles INT_MIN correctly (-(INT_MIN) overflows but cast works)
        UnsignedT uval = static_cast<UnsignedT>(~value) + 1u;
        return 1 + uint_to_str(uval, buffer + 1);
    }
    return uint_to_str(static_cast<UnsignedT>(value), buffer);
}

template <typename T>
inline std::size_t uint_to_hex(T value, char* buffer, bool uppercase) {
    const char* hex = uppercase ? hex_digits_upper() : hex_digits_lower();

    if (value == 0) {
        buffer[0] = '0';
        return 1;
    }

    char temp[20];
    char* ptr = temp + sizeof(temp);

    while (value > 0) {
        *--ptr = hex[value & 0xF];
        value >>= 4;
    }

    std::size_t len = static_cast<std::size_t>((temp + sizeof(temp)) - ptr);
    std::memcpy(buffer, ptr, len);
    return len;
}

template <typename T>
inline std::size_t uint_to_oct(T value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        return 1;
    }

    char temp[24];
    char* ptr = temp + sizeof(temp);

    while (value > 0) {
        *--ptr = static_cast<char>('0' + (value & 7));
        value >>= 3;
    }

    std::size_t len = static_cast<std::size_t>((temp + sizeof(temp)) - ptr);
    std::memcpy(buffer, ptr, len);
    return len;
}

template <typename T>
inline std::size_t uint_to_bin(T value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        return 1;
    }

    char temp[68];
    char* ptr = temp + sizeof(temp);

    while (value > 0) {
        *--ptr = static_cast<char>('0' + (value & 1));
        value >>= 1;
    }

    std::size_t len = static_cast<std::size_t>((temp + sizeof(temp)) - ptr);
    std::memcpy(buffer, ptr, len);
    return len;
}

inline std::size_t ptr_to_str(const void* ptr, char* buffer) {
    if (ptr == 0) {
        std::memcpy(buffer, "(nil)", 5);
        return 5;
    }

    buffer[0] = '0';
    buffer[1] = 'x';
    return 2 + uint_to_hex(reinterpret_cast<std::uintptr_t>(ptr), buffer + 2, false);
}

} // namespace detail
} // namespace Vita

#endif
