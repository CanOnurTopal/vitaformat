// vita/detail/float_to_str.hpp
// float/double to string - simplified grisu-style
#ifndef VITA_DETAIL_FLOAT_TO_STR_HPP
#define VITA_DETAIL_FLOAT_TO_STR_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>

#include "int_to_str.hpp"

namespace Vita {
namespace detail {

struct DoubleComponents {
    std::uint64_t mantissa;
    int exponent;
    bool negative;
    bool is_zero;
    bool is_inf;
    bool is_nan;
};

inline DoubleComponents decompose_double(double value) {
    DoubleComponents c;
    std::uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));

    c.negative = (bits >> 63) != 0;
    std::uint64_t exp_bits = (bits >> 52) & 0x7FF;
    c.mantissa = bits & 0xFFFFFFFFFFFFFull;

    if (exp_bits == 0) {
        c.is_zero = (c.mantissa == 0);
        c.is_inf = false;
        c.is_nan = false;
        c.exponent = c.is_zero ? 0 : (1 - 1023 - 52);
    } else if (exp_bits == 0x7FF) {
        c.is_zero = false;
        c.is_inf = (c.mantissa == 0);
        c.is_nan = !c.is_inf;
        c.exponent = 0;
    } else {
        c.is_zero = c.is_inf = c.is_nan = false;
        c.mantissa |= (1ull << 52);
        c.exponent = static_cast<int>(exp_bits) - 1023 - 52;
    }
    return c;
}

// pow10 lookup
inline const double* pow10_pos() {
    static const double t[23] = {
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
        1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22
    };
    return t;
}

inline const double* pow10_neg() {
    static const double t[23] = {
        1e0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-10, 1e-11,
        1e-12, 1e-13, 1e-14, 1e-15, 1e-16, 1e-17, 1e-18, 1e-19, 1e-20, 1e-21, 1e-22
    };
    return t;
}

inline double pow10_fast(int n) {
    if (n >= 0 && n <= 22) return pow10_pos()[n];
    if (n < 0 && n >= -22) return pow10_neg()[-n];
    return std::pow(10.0, n);
}

inline int estimate_exp10(double v) {
    if (v == 0.0) return 0;
    int e2;
    std::frexp(v, &e2);
    return static_cast<int>((e2 - 1) * 0.30102999566398119);
}

inline std::size_t double_to_str_shortest(double value, char* buffer, int precision = -1) {
    DoubleComponents c = decompose_double(value);
    char* p = buffer;

    if (c.is_nan) {
        std::memcpy(p, "nan", 3);
        return 3;
    }
    if (c.negative) *p++ = '-';
    if (c.is_inf) {
        std::memcpy(p, "inf", 3);
        return static_cast<std::size_t>(p - buffer + 3);
    }
    if (c.is_zero) {
        *p++ = '0';
        return static_cast<std::size_t>(p - buffer);
    }

    double abs_val = c.negative ? -value : value;
    int exp10 = estimate_exp10(abs_val);

    double norm = abs_val;
    if (exp10 > 0) norm = abs_val / pow10_fast(exp10);
    else if (exp10 < 0) norm = abs_val * pow10_fast(-exp10);

    while (norm >= 10.0) { norm /= 10.0; exp10++; }
    while (norm < 1.0 && norm > 0.0) { norm *= 10.0; exp10--; }

    int sig = (precision >= 0) ? precision : 15;
    if (sig > 17) sig = 17;
    if (sig < 1) sig = 1;

    char digits[20];
    int ndig = 0;
    std::uint64_t int_part = static_cast<std::uint64_t>(norm * pow10_fast(sig - 1) + 0.5);

    char* dp = digits + 18;
    while (int_part > 0 && ndig < sig) {
        *--dp = static_cast<char>('0' + (int_part % 10));
        int_part /= 10;
        ndig++;
    }
    while (ndig < sig) { *--dp = '0'; ndig++; }

    // trim trailing zeros
    int trail = 0;
    for (int i = ndig - 1; i > 0 && dp[i] == '0'; i--) trail++;
    ndig -= trail;

    // pick notation
    if (exp10 >= -4 && exp10 < sig + 6) {
        if (exp10 >= 0) {
            int idig = exp10 + 1;
            if (idig >= ndig) {
                std::memcpy(p, dp, ndig);
                p += ndig;
                for (int i = ndig; i < idig; i++) *p++ = '0';
            } else {
                std::memcpy(p, dp, idig);
                p += idig;
                *p++ = '.';
                std::memcpy(p, dp + idig, ndig - idig);
                p += ndig - idig;
            }
        } else {
            *p++ = '0';
            *p++ = '.';
            for (int i = 0; i < -exp10 - 1; i++) *p++ = '0';
            std::memcpy(p, dp, ndig);
            p += ndig;
        }
    } else {
        // scientific
        *p++ = dp[0];
        if (ndig > 1) {
            *p++ = '.';
            std::memcpy(p, dp + 1, ndig - 1);
            p += ndig - 1;
        }
        *p++ = 'e';
        if (exp10 >= 0) *p++ = '+';
        else { *p++ = '-'; exp10 = -exp10; }
        if (exp10 >= 100) { *p++ = static_cast<char>('0' + exp10/100); exp10 %= 100; }
        *p++ = static_cast<char>('0' + exp10/10);
        *p++ = static_cast<char>('0' + exp10%10);
    }

    return static_cast<std::size_t>(p - buffer);
}

inline std::size_t double_to_str_fixed(double value, char* buffer, int prec = 6) {
    DoubleComponents c = decompose_double(value);
    char* p = buffer;

    if (c.is_nan) { std::memcpy(p, "nan", 3); return 3; }
    if (c.negative) *p++ = '-';
    if (c.is_inf) { std::memcpy(p, "inf", 3); return static_cast<std::size_t>(p - buffer + 3); }

    double abs_val = c.negative ? -value : value;

    double mult = pow10_fast(prec);
    double rounded = std::floor(abs_val * mult + 0.5) / mult;

    double int_d;
    double frac = std::modf(rounded, &int_d);

    if (int_d == 0.0) *p++ = '0';
    else if (int_d < 1e15) p += uint_to_str(static_cast<std::uint64_t>(int_d), p);
    else return double_to_str_shortest(value, buffer, prec);

    if (prec > 0) {
        *p++ = '.';
        double scaled = frac * mult;
        std::uint64_t frac_int = static_cast<std::uint64_t>(scaled + 0.5);

        // leading zeros
        char fbuf[20];
        int flen = static_cast<int>(uint_to_str(frac_int, fbuf));
        for (int i = flen; i < prec; i++) *p++ = '0';
        std::memcpy(p, fbuf, flen);
        p += flen;
    }

    return static_cast<std::size_t>(p - buffer);
}

inline std::size_t double_to_str_scientific(double value, char* buffer, int prec = 6, bool upper = false) {
    DoubleComponents c = decompose_double(value);
    char* p = buffer;

    if (c.is_nan) { std::memcpy(p, upper ? "NAN" : "nan", 3); return 3; }
    if (c.negative) *p++ = '-';
    if (c.is_inf) { std::memcpy(p, upper ? "INF" : "inf", 3); return static_cast<std::size_t>(p - buffer + 3); }

    if (c.is_zero) {
        *p++ = '0';
        if (prec > 0) { *p++ = '.'; for (int i = 0; i < prec; i++) *p++ = '0'; }
        *p++ = upper ? 'E' : 'e';
        *p++ = '+'; *p++ = '0'; *p++ = '0';
        return static_cast<std::size_t>(p - buffer);
    }

    double abs_val = c.negative ? -value : value;
    int exp10 = estimate_exp10(abs_val);
    double norm = abs_val / pow10_fast(exp10);

    while (norm >= 10.0) { norm /= 10.0; exp10++; }
    while (norm < 1.0) { norm *= 10.0; exp10--; }

    double scaled = norm * pow10_fast(prec);
    std::uint64_t mant = static_cast<std::uint64_t>(scaled + 0.5);

    std::uint64_t thresh = 1;
    for (int i = 0; i <= prec; i++) thresh *= 10;
    if (mant >= thresh) { mant /= 10; exp10++; }

    char mbuf[20];
    int mlen = static_cast<int>(uint_to_str(mant, mbuf));

    *p++ = mbuf[0];
    if (prec > 0) {
        *p++ = '.';
        int need = prec + 1 - mlen;
        while (need-- > 0) { *p++ = '0'; prec--; }
        int clen = (mlen - 1 < prec) ? mlen - 1 : prec;
        if (clen > 0) { std::memcpy(p, mbuf + 1, clen); p += clen; }
        for (int i = clen; i < prec; i++) *p++ = '0';
    }

    *p++ = upper ? 'E' : 'e';
    if (exp10 >= 0) *p++ = '+';
    else { *p++ = '-'; exp10 = -exp10; }
    if (exp10 >= 100) { *p++ = static_cast<char>('0' + exp10/100); exp10 %= 100; }
    *p++ = static_cast<char>('0' + exp10/10);
    *p++ = static_cast<char>('0' + exp10%10);

    return static_cast<std::size_t>(p - buffer);
}

// float wrappers
inline std::size_t float_to_str_shortest(float v, char* buf, int prec = -1) {
    return double_to_str_shortest(static_cast<double>(v), buf, prec >= 0 ? prec : 6);
}
inline std::size_t float_to_str_fixed(float v, char* buf, int prec = 6) {
    return double_to_str_fixed(static_cast<double>(v), buf, prec);
}
inline std::size_t float_to_str_scientific(float v, char* buf, int prec = 6, bool up = false) {
    return double_to_str_scientific(static_cast<double>(v), buf, prec, up);
}

// long double - just cast down for now
// TODO: proper 80-bit support if needed
inline std::size_t ldouble_to_str_shortest(long double v, char* buf, int prec = -1) {
    return double_to_str_shortest(static_cast<double>(v), buf, prec);
}
inline std::size_t ldouble_to_str_fixed(long double v, char* buf, int prec = 6) {
    return double_to_str_fixed(static_cast<double>(v), buf, prec);
}
inline std::size_t ldouble_to_str_scientific(long double v, char* buf, int prec = 6, bool up = false) {
    return double_to_str_scientific(static_cast<double>(v), buf, prec, up);
}

} // namespace detail
} // namespace Vita

#endif
