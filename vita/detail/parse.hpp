// vita/detail/parse.hpp
// format string parser
#ifndef VITA_DETAIL_PARSE_HPP
#define VITA_DETAIL_PARSE_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace Vita {
namespace detail {

struct FormatSpec {
    char fill;
    char align;       // '<' '>' '^' '='
    char sign;        // '+' '-' ' '
    bool alt_form;
    bool zero_pad;
    int width;
    int precision;
    char type;        // d x X o b f e E g G s c p

    FormatSpec() noexcept
        : fill(' '), align('\0'), sign('-'), alt_form(false),
          zero_pad(false), width(0), precision(-1), type('\0') {}
};

// parse format spec after the ':'
inline const char* parse_format_spec(const char* begin, const char* end, FormatSpec& spec) {
    if (begin >= end) return begin;
    const char* p = begin;

    // fill + align?
    if (p + 1 < end) {
        char c2 = p[1];
        if (c2 == '<' || c2 == '>' || c2 == '^' || c2 == '=') {
            spec.fill = *p;
            spec.align = c2;
            p += 2;
        }
    }

    // standalone align
    if (spec.align == '\0' && p < end) {
        char c = *p;
        if (c == '<' || c == '>' || c == '^' || c == '=') {
            spec.align = c;
            p++;
        }
    }

    // sign
    if (p < end && (*p == '+' || *p == '-' || *p == ' ')) {
        spec.sign = *p++;
    }

    // alt form
    if (p < end && *p == '#') {
        spec.alt_form = true;
        p++;
    }

    // zero pad
    if (p < end && *p == '0') {
        spec.zero_pad = true;
        p++;
    }

    // width
    if (p < end && *p >= '1' && *p <= '9') {
        int w = 0;
        while (p < end && *p >= '0' && *p <= '9')
            w = w * 10 + (*p++ - '0');
        spec.width = w;
    }

    // precision
    if (p < end && *p == '.') {
        p++;
        int prec = 0;
        while (p < end && *p >= '0' && *p <= '9')
            prec = prec * 10 + (*p++ - '0');
        spec.precision = prec;
    }

    // type
    if (p < end) {
        char c = *p;
        if (c == 'd' || c == 'x' || c == 'X' || c == 'o' || c == 'b' ||
            c == 'f' || c == 'F' || c == 'e' || c == 'E' || c == 'g' || c == 'G' ||
            c == 's' || c == 'c' || c == 'p' || c == 'a' || c == 'A') {
            spec.type = c;
            p++;
        }
    }

    return p;
}

struct Placeholder {
    int arg_index;
    FormatSpec spec;
    const char* spec_begin;
    const char* spec_end;
};

struct ParseSegment {
    enum Type { LITERAL, PLACEHOLDER, ESCAPE_OPEN, ESCAPE_CLOSE, END, ERROR };
    Type type;
    const char* begin;
    const char* end;
    Placeholder placeholder;
};

class FormatParser {
public:
    FormatParser(const char* fmt, std::size_t len) noexcept
        : ptr_(fmt), end_(fmt + len), auto_idx_(0),
          has_explicit_(false), has_auto_(false) {}

    explicit FormatParser(const char* fmt) noexcept
        : FormatParser(fmt, std::strlen(fmt)) {}

    ParseSegment next() {
        ParseSegment seg;

        if (ptr_ >= end_) {
            seg.type = ParseSegment::END;
            seg.begin = seg.end = ptr_;
            return seg;
        }

        // {{
        if (*ptr_ == '{' && ptr_ + 1 < end_ && ptr_[1] == '{') {
            seg.type = ParseSegment::ESCAPE_OPEN;
            seg.begin = ptr_;
            ptr_ += 2;
            seg.end = ptr_;
            return seg;
        }

        // }}
        if (*ptr_ == '}' && ptr_ + 1 < end_ && ptr_[1] == '}') {
            seg.type = ParseSegment::ESCAPE_CLOSE;
            seg.begin = ptr_;
            ptr_ += 2;
            seg.end = ptr_;
            return seg;
        }

        // placeholder
        if (*ptr_ == '{') {
            return parse_placeholder();
        }

        // literal
        seg.type = ParseSegment::LITERAL;
        seg.begin = ptr_;

        while (ptr_ < end_ && *ptr_ != '{' && *ptr_ != '}')
            ptr_++;

        // lone } check
        if (ptr_ < end_ && *ptr_ == '}' && (ptr_ + 1 >= end_ || ptr_[1] != '}'))
            ptr_++;

        seg.end = ptr_;
        return seg;
    }

    bool has_mixing_error() const { return has_explicit_ && has_auto_; }

private:
    ParseSegment parse_placeholder() {
        ParseSegment seg;
        seg.type = ParseSegment::PLACEHOLDER;
        seg.begin = ptr_++;

        // arg index
        if (ptr_ < end_ && *ptr_ >= '0' && *ptr_ <= '9') {
            int idx = 0;
            while (ptr_ < end_ && *ptr_ >= '0' && *ptr_ <= '9')
                idx = idx * 10 + (*ptr_++ - '0');
            seg.placeholder.arg_index = idx;
            has_explicit_ = true;
        } else {
            seg.placeholder.arg_index = auto_idx_++;
            has_auto_ = true;
        }

        seg.placeholder.spec_begin = 0;
        seg.placeholder.spec_end = 0;

        // format spec
        if (ptr_ < end_ && *ptr_ == ':') {
            ptr_++;
            seg.placeholder.spec_begin = ptr_;
            while (ptr_ < end_ && *ptr_ != '}')
                ptr_++;
            seg.placeholder.spec_end = ptr_;
            parse_format_spec(seg.placeholder.spec_begin, seg.placeholder.spec_end,
                              seg.placeholder.spec);
        }

        if (ptr_ < end_ && *ptr_ == '}') {
            ptr_++;
            seg.end = ptr_;
        } else {
            seg.type = ParseSegment::ERROR;
            seg.end = ptr_;
        }

        return seg;
    }

    const char* ptr_;
    const char* end_;
    int auto_idx_;
    bool has_explicit_;
    bool has_auto_;
};

} // namespace detail
} // namespace Vita

#endif
