// vita/detail/compile_parse.hpp
// compile-time helpers (limited in C++11, mostly runtime fallbacks)
#ifndef VITA_DETAIL_COMPILE_PARSE_HPP
#define VITA_DETAIL_COMPILE_PARSE_HPP

#include <cstddef>

#ifndef VITA_FORMAT_MAX_FORMAT_LENGTH
#define VITA_FORMAT_MAX_FORMAT_LENGTH 256
#endif

namespace Vita {
namespace detail {
namespace ct {

// C++11 constexpr is too limited for real compile-time parsing,
// so we just do runtime with template instantiation benefits

inline int count_placeholders_rt(const char* fmt, std::size_t len) {
    int count = 0;
    std::size_t i = 0;
    while (i < len) {
        if (fmt[i] == '{') {
            if (i + 1 < len && fmt[i + 1] == '{') {
                i += 2;
            } else {
                count++;
                while (i < len && fmt[i] != '}') i++;
                if (i < len) i++;
            }
        } else if (fmt[i] == '}' && i + 1 < len && fmt[i + 1] == '}') {
            i += 2;
        } else {
            i++;
        }
    }
    return count;
}

template <std::size_t N>
inline int count_placeholders(const char (&fmt)[N]) {
    return count_placeholders_rt(fmt, N - 1);
}

struct SegmentDesc {
    enum Type : unsigned char { LITERAL, PLACEHOLDER, ESCAPE_OPEN, ESCAPE_CLOSE };
    Type type;
    unsigned short start;
    unsigned short length;
    unsigned char arg_index;
};

template <std::size_t MaxSeg>
struct ParsedFormat {
    SegmentDesc segments[MaxSeg > 0 ? MaxSeg : 1];
    std::size_t num_segments;
    int num_placeholders;
    const char* fmt;
    std::size_t fmt_len;

    ParsedFormat() : num_segments(0), num_placeholders(0), fmt(0), fmt_len(0) {}
};

template <std::size_t MaxSeg>
inline void parse_format_string(const char* fmt, std::size_t len, ParsedFormat<MaxSeg>& out) {
    out.fmt = fmt;
    out.fmt_len = len;
    out.num_segments = 0;
    out.num_placeholders = 0;

    std::size_t i = 0;
    int auto_idx = 0;
    std::size_t lit_start = 0;
    bool in_lit = false;

    while (i < len && out.num_segments < MaxSeg) {
        if (fmt[i] == '{') {
            if (i + 1 < len && fmt[i + 1] == '{') {
                if (in_lit && i > lit_start) {
                    out.segments[out.num_segments].type = SegmentDesc::LITERAL;
                    out.segments[out.num_segments].start = static_cast<unsigned short>(lit_start);
                    out.segments[out.num_segments].length = static_cast<unsigned short>(i - lit_start);
                    out.num_segments++;
                }
                out.segments[out.num_segments].type = SegmentDesc::ESCAPE_OPEN;
                out.segments[out.num_segments].start = static_cast<unsigned short>(i);
                out.segments[out.num_segments].length = 2;
                out.num_segments++;
                i += 2;
                lit_start = i;
                in_lit = false;
            } else {
                if (in_lit && i > lit_start) {
                    out.segments[out.num_segments].type = SegmentDesc::LITERAL;
                    out.segments[out.num_segments].start = static_cast<unsigned short>(lit_start);
                    out.segments[out.num_segments].length = static_cast<unsigned short>(i - lit_start);
                    out.num_segments++;
                }
                std::size_t ph_start = i++;

                int idx = -1;
                if (i < len && fmt[i] >= '0' && fmt[i] <= '9') {
                    idx = 0;
                    while (i < len && fmt[i] >= '0' && fmt[i] <= '9')
                        idx = idx * 10 + (fmt[i++] - '0');
                } else {
                    idx = auto_idx++;
                }

                while (i < len && fmt[i] != '}') i++;
                if (i < len) i++;

                out.segments[out.num_segments].type = SegmentDesc::PLACEHOLDER;
                out.segments[out.num_segments].start = static_cast<unsigned short>(ph_start);
                out.segments[out.num_segments].length = static_cast<unsigned short>(i - ph_start);
                out.segments[out.num_segments].arg_index = static_cast<unsigned char>(idx);
                out.num_segments++;
                out.num_placeholders++;

                lit_start = i;
                in_lit = false;
            }
        } else if (fmt[i] == '}' && i + 1 < len && fmt[i + 1] == '}') {
            if (in_lit && i > lit_start) {
                out.segments[out.num_segments].type = SegmentDesc::LITERAL;
                out.segments[out.num_segments].start = static_cast<unsigned short>(lit_start);
                out.segments[out.num_segments].length = static_cast<unsigned short>(i - lit_start);
                out.num_segments++;
            }
            out.segments[out.num_segments].type = SegmentDesc::ESCAPE_CLOSE;
            out.segments[out.num_segments].start = static_cast<unsigned short>(i);
            out.segments[out.num_segments].length = 2;
            out.num_segments++;
            i += 2;
            lit_start = i;
            in_lit = false;
        } else {
            if (!in_lit) { lit_start = i; in_lit = true; }
            i++;
        }
    }

    if (in_lit && i > lit_start && out.num_segments < MaxSeg) {
        out.segments[out.num_segments].type = SegmentDesc::LITERAL;
        out.segments[out.num_segments].start = static_cast<unsigned short>(lit_start);
        out.segments[out.num_segments].length = static_cast<unsigned short>(i - lit_start);
        out.num_segments++;
    }
}

} // namespace ct
} // namespace detail
} // namespace Vita

#endif
