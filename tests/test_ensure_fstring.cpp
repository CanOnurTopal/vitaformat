#define VITA_COMPILE_TIME_DEBUG //WILL NOT COMPILE WITHOUT THIS
#include "vita/detail/ensure_fstring.hpp"

#include <gtest/gtest.h>
#include <cstddef>
#include <cstring>
#include <cstdio>

#define VALID(str)   EXPECT_STREQ(VITA_FORMAT_ENSURE_FSTRING(str), str)
#define INVALID(str) EXPECT_STREQ(VITA_FORMAT_ENSURE_FSTRING(str), "__VITA_SYNTAX_ERROR__")

constexpr bool streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return false;
        ++a; ++b;
    }
    return *a == *b;
}
constexpr const char* identity(const char* s) { return s; }
constexpr const char* pick_first(const char* a, const char*) { return a; }
constexpr const char* pick_second(const char*, const char* b) { return b; }
constexpr const char* pick_third(const char*, const char*, const char* c) { return c; }

template<typename T>
constexpr T tmpl_identity(T val) { return val; }

constexpr bool is_valid_fmt(const char* s) {
    return !streq(s, "__VITA_SYNTAX_ERROR__");
}

struct FmtPair {
    const char* label;
    const char* fmt;
};

struct FormatHolder {
    const char* fmt;
    constexpr FormatHolder(const char* f) : fmt(f) {}
    constexpr const char* get() const { return fmt; }
};

template<bool B>
struct BoolCheck { static constexpr bool value = B; };

struct ImplicitStr {
    const char* data;
    constexpr ImplicitStr(const char* s) : data(s) {}
};

constexpr const char* layer3(const char* s) { return s; }
constexpr const char* layer2(const char* s) { return layer3(s); }
constexpr const char* layer1(const char* s) { return layer2(s); }

constexpr const char* get_format() {
    return VITA_FORMAT_ENSURE_FSTRING("{:>10.2f}");
}

constexpr const char* get_bad_format() {
    return VITA_FORMAT_ENSURE_FSTRING("{:!}");
}

constexpr const char* with_default(
    const char* fmt = VITA_FORMAT_ENSURE_FSTRING("{}")) {
    return fmt;
}

using FormatCallback = const char* (*)(const char*);
const char* echo_format(const char* fmt) { return fmt; }

const char* get_static_fmt() {
    static const char* fmt = VITA_FORMAT_ENSURE_FSTRING("{:>20}");
    return fmt;
}

TEST(EnsureFstring, ValidBasicStrings) {
    VALID("");
    VALID("a");
    VALID("hello world");
    VALID("1234567890");
    VALID("!@#$%^&*()_+-=[]\\;',./~`");
    VALID("line1\nline2\ttab");
    VALID("    ");
    VALID("multi\n\n\nlines");
    VALID("path/to/file.txt");
    VALID("http://example.com?a=1&b=2");
    VALID("SELECT * FROM t WHERE x = 1;");
    VALID(":");
    VALID("a\0b");
}

TEST(EnsureFstring, ValidEscapedBraces) {
    VALID("{{");
    VALID("}}");
    VALID("{{}}");
    VALID("{{{{");
    VALID("}}}}");
    VALID("{{{{}}}}");
    VALID("a{{b");
    VALID("a}}b");
    VALID("a{{b}}c");
    VALID("{{hello}}");
    VALID("{{{}}}");
    VALID("{{{0}}}");
    VALID("{{{{}}}}{}");
    VALID("{}{{}}{}");
    VALID("{{}}{}{{}}");
    VALID("x{{y{{z");
    VALID("x}}y}}z");
    VALID("a{{b{{c}}d}}e");
    VALID("{{}}{{}}{{}}");
    VALID("{{{{{{}}}}}}");
}

TEST(EnsureFstring, ValidSingleClosingBraces) {
    VALID("}");
    VALID("}hello");
    VALID("hello}");
    VALID("a}b}c");
    VALID("}}}");
    VALID("}}}}");
    VALID("}}}}}");
    VALID("{}}");
    VALID("{}}}");
    VALID("}{}");
    VALID("}{{}");
}

TEST(EnsureFstring, ValidAutoPlaceholders) {
    VALID("{}");
    VALID("{} {}");
    VALID("{} {} {}");
    VALID("{}{}{}{}{}{}{}{}{}{}");
    VALID("a{}b");
    VALID("{}a");
    VALID("a{}");
    VALID("{}{}");
    VALID("{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}");
    VALID("prefix {} middle {} suffix");
    VALID("{}\n{}\t{}");
    VALID("{}{{}}{}{{}}{}");
}

TEST(EnsureFstring, ValidIndexedPlaceholders) {
    VALID("{0}");
    VALID("{1}");
    VALID("{2}");
    VALID("{9}");
    VALID("{10}");
    VALID("{99}");
    VALID("{100}");
    VALID("{999}");
    VALID("{12345}");
    VALID("{99999}");
    VALID("{00}");
    VALID("{01}");
    VALID("{007}");
    VALID("{0}{1}{2}");
    VALID("{2}{0}{1}");
    VALID("{0}{0}{0}");
    VALID("{0} and {0}");
    VALID("{1} before {0}");
    VALID("a{0}b{1}c{2}d");
}

TEST(EnsureFstring, ValidEmptyFormatSpec) {
    VALID("{:}");
    VALID("{0:}");
    VALID("{1:}");
    VALID("{99:}");
    VALID("{:}{:}");
    VALID("{0:}{1:}");
}

TEST(EnsureFstring, ValidAlignmentSpec) {
    VALID("{:<}");
    VALID("{:>}");
    VALID("{:^}");
    VALID("{:=}");
    VALID("{0:<}");
    VALID("{0:>}");
    VALID("{0:^}");
    VALID("{0:=}");

    VALID("{:*>}");
    VALID("{:*<}");
    VALID("{:*^}");
    VALID("{:*=}");
    VALID("{:->}");
    VALID("{:_>}");
    VALID("{: >}");
    VALID("{:.>}");
    VALID("{:#>}");
    VALID("{:+>}");
    VALID("{:0>}");
    VALID("{:0<}");
    VALID("{:0^}");
    VALID("{:0=}");

    VALID("{:>>}");
    VALID("{:<<}");
    VALID("{:^^}");
    VALID("{:==}");
    VALID("{:><}");
    VALID("{:<>}");
    VALID("{:^<}");
    VALID("{:=<}");
    VALID("{:=>}");

    VALID("{:1<}");
    VALID("{:2>}");
    VALID("{:9^}");
    VALID("{:1<10}");
    VALID("{:0<5}");
    VALID("{:5>10}");
}

TEST(EnsureFstring, ValidFillCharacters) {
    VALID("{:@>}");
    VALID("{:!>}");
    VALID("{:$>}");
    VALID("{:%>}");
    VALID("{:&>}");
    VALID("{:(>}");
    VALID("{:)>}");
    VALID("{:[>}");
    VALID("{:]>}");
    VALID("{:|>}");
    VALID("{:\\>}");
    VALID("{:/>}");
    VALID("{:?>}");
    VALID("{:~>}");
    VALID("{:,>}");
    VALID("{:;>}");
    VALID("{:'>}");

    VALID("{:{>}");
    VALID("{:{<}");
    VALID("{:{^}");
    VALID("{:{=}");
    VALID("{:{>10}");
    VALID("{:{>10.5f}");
}

TEST(EnsureFstring, ValidFillAndWidth) {
    VALID("{:*>10}");
    VALID("{:.<20}");
    VALID("{:-^30}");
    VALID("{:0=40}");
    VALID("{: >100}");

    VALID("{:*>+}");
    VALID("{:*>+#}");
    VALID("{:*>+#0}");
    VALID("{:*>+#010}");
    VALID("{:*>+#010.5}");
    VALID("{:*>+#010.5f}");
}

TEST(EnsureFstring, ValidSignSpec) {
    VALID("{:+}");
    VALID("{:-}");
    VALID("{: }");

    VALID("{:+d}");
    VALID("{:-d}");
    VALID("{: d}");
    VALID("{:+10}");
    VALID("{:-10}");
    VALID("{: 10}");
    VALID("{:+.5}");
    VALID("{:+10.5}");
    VALID("{:+10.5f}");
    VALID("{:>+}");
    VALID("{:>+10.5f}");
}

TEST(EnsureFstring, ValidHashSpec) {
    VALID("{:#}");
    VALID("{:#x}");
    VALID("{:#X}");
    VALID("{:#o}");
    VALID("{:#b}");
    VALID("{:#d}");
    VALID("{:+#}");
    VALID("{:+#x}");
    VALID("{:>#x}");
    VALID("{:*>#x}");
    VALID("{:#10}");
}

TEST(EnsureFstring, ValidZeroPadding) {
    VALID("{:0}");
    VALID("{:05}");
    VALID("{:010}");
    VALID("{:08x}");
    VALID("{:+05}");
    VALID("{:+08x}");
    VALID("{:#010x}");
    VALID("{:+#010x}");
    VALID("{:<05}");
    VALID("{:00}");
    VALID("{:00x}");
}

TEST(EnsureFstring, ValidWidthSpec) {
    VALID("{:1}");
    VALID("{:5}");
    VALID("{:10}");
    VALID("{:100}");
    VALID("{:999}");
    VALID("{:10d}");
    VALID("{:10x}");
    VALID("{:10s}");
    VALID("{:10f}");
    VALID("{:10.5}");
    VALID("{:10.5f}");
    VALID("{:20.10e}");
}

TEST(EnsureFstring, ValidPrecisionSpec) {
    VALID("{:.0}");
    VALID("{:.1}");
    VALID("{:.5}");
    VALID("{:.10}");
    VALID("{:.100}");
    VALID("{:.999}");
    VALID("{:.}");
    VALID("{:.5f}");
    VALID("{:.2f}");
    VALID("{:.0f}");
    VALID("{:.5e}");
    VALID("{:.5E}");
    VALID("{:.5g}");
    VALID("{:.5s}");
    VALID("{:10.5f}");
    VALID("{:20.10e}");
    VALID("{:8.2f}");
}

TEST(EnsureFstring, ValidTypeSpec) {
    VALID("{:d}");
    VALID("{:x}");
    VALID("{:X}");
    VALID("{:o}");
    VALID("{:b}");

    VALID("{:f}");
    VALID("{:F}");
    VALID("{:e}");
    VALID("{:E}");
    VALID("{:g}");
    VALID("{:G}");
    VALID("{:a}");
    VALID("{:A}");

    VALID("{:s}");
    VALID("{:c}");
    VALID("{:p}");

    VALID("{0:d}");
    VALID("{1:x}");
    VALID("{2:f}");
    VALID("{99:s}");
}

TEST(EnsureFstring, ValidComplexSpec) {
    VALID("{:*<+#010.5f}");
    VALID("{:0=+#010.5f}");
    VALID("{0:*^+#010.5e}");
    VALID("{99:_>-010.100g}");
    VALID("{: >+#08.3f}");
    VALID("{:0=+10d}");
    VALID("{:+>10.5f}");

    VALID("Hello, {}!");
    VALID("{} + {} = {}");
    VALID("{0} and {0}");
    VALID("{1} before {0}");
    VALID("{:>10}");
    VALID("{:*^20}");
    VALID("{:+}");
    VALID("{:05}");
    VALID("{:08x}");
    VALID("{:b}");
    VALID("{:.2f}");
    VALID("{:.5}");
    VALID("{{}}");
    VALID("${:.2f}");
    VALID("{:02X} {:02X}");
    VALID("{:<10}");
    VALID("{:^10}");
    VALID("{:*>10}");
    VALID("{:0=+10}");
    VALID("{:.0f}");
    VALID("{:.4f}");
    VALID("{:.2e}");
    VALID("{:.2F}");
    VALID("{:.2E}");
}

TEST(EnsureFstring, ValidRealWorldFormats) {
    VALID("Error at line {}: {}");
    VALID("[{:>8}] {:.<40} {:.2f}%");
    VALID("{:04d}-{:02d}-{:02d}");
    VALID("{:02d}:{:02d}:{:02d}");
    VALID("0x{:08X}");
    VALID("{} ({:.1f}%)");
    VALID("{0}: {1} (was {2})");
    VALID("{{{0}}}");
    VALID("{{key}}: {}");
    VALID("Total: ${:.2f}");
    VALID("{:>20s} | {:>10d} | {:>10.2f}");
    VALID("{}{}{}{}");
    VALID("{0}{0}{0}");
    VALID("{:*^40}");
}

TEST(EnsureFstring, ValidAmbiguousFillChars) {
    VALID("{:+<}");
    VALID("{:+>}");
    VALID("{:+}");
    VALID("{:+<10}");
    VALID("{:+10}");

    VALID("{:-<}");
    VALID("{:->}");
    VALID("{:-}");
    VALID("{:-<10}");
    VALID("{:-10}");

    VALID("{: <}");
    VALID("{: >}");
    VALID("{: }");
    VALID("{: <10}");
    VALID("{: 10}");

    VALID("{:#>}");
    VALID("{:#<}");
    VALID("{:#}");
    VALID("{:#>10}");
    VALID("{:#10}");

    VALID("{:0<}");
    VALID("{:0>}");
    VALID("{:0}");
    VALID("{:0<5}");
    VALID("{:05}");

    VALID("{:1>}");
    VALID("{:1<10}");
    VALID("{:5^20}");
    VALID("{:1}");

    VALID("{:=>}");
    VALID("{:=<}");
    VALID("{:=}");
    VALID("{:=10}");
    VALID("{:=>10}");
}

TEST(EnsureFstring, ValidShortStrings) {
    VALID("a");
    VALID("}");
    VALID("{}");
    VALID("{{");
    VALID("}}");
    VALID("ab");
    VALID("a}");
    VALID("}a");
    VALID("{:}");
    VALID("{0}");
    VALID("abc");
    VALID("a{}");
    VALID("{}a");
    VALID("{{}");
    VALID("{{}}");
    VALID("{{{}");
    VALID("{}{}");
    VALID("{:d}");
    VALID("{0:}");
}

TEST(EnsureFstring, ValidMixedAndLongStrings) {
    VALID("{}");
    VALID("{}x");
    VALID("x{}");
    VALID("x{}x");

    VALID("{:d} {:x} {:o} {:b}");
    VALID("{0:>10s} {1:08x} {2:.2f}");
    VALID("{:*<10} {:->20} {:_^30}");

    VALID("{{}");
    VALID("{{}}");
    VALID("{}{}");
    VALID("{{}}}");
    VALID("{{}{}");
    VALID("{{}{}}");
    VALID("{}{}{}");
    VALID("{}{{}}{}{{}}{}");

    VALID("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    VALID("{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}");
    VALID("{0}{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}{11}{12}{13}{14}{15}");
    VALID("{:*>10}{:.<20}{:-^30}{:_=40}");
    VALID("{}{{}}{}{{}}{}{{}}{}{{}}{}");
}

TEST(EnsureFstring, InvalidUnclosedBrace) {
    INVALID("{");
    INVALID("a{");
    INVALID("{b");
    INVALID("abc{");
    INVALID("abc{def");
    INVALID("hello { world");
    INVALID("{}{");
    INVALID("{} {");
    INVALID("{0}{");
    INVALID("{:d}{");

    INVALID("{0");
    INVALID("{12");
    INVALID("{:");
    INVALID("{:d");
    INVALID("{:>10");
    INVALID("{:>10.5");
    INVALID("{:>10.5f");
    INVALID("{:*>+#010.5f");
    INVALID("{0:");
    INVALID("{0:d");

    INVALID("valid {0} then {");
    INVALID("{{}} {");
}

TEST(EnsureFstring, InvalidNamedAndBadArgs) {
    INVALID("{a}");
    INVALID("{z}");
    INVALID("{A}");
    INVALID("{Z}");
    INVALID("{abc}");
    INVALID("{hello}");
    INVALID("{name}");
    INVALID("{key}");
    INVALID("{-1}");
    INVALID("{+1}");
    INVALID("{.1}");
    INVALID("{1.0}");
    INVALID("{ }");
    INVALID("{ 0}");
    INVALID("{0 }");
    INVALID("{_}");
    INVALID("{0a}");
    INVALID("{1b2}");
    INVALID("{!}");
    INVALID("{?}");
    INVALID("{*}");
    INVALID("{@}");
    INVALID("{$}");
    INVALID("{0,1}");
    INVALID("{0.1}");
    INVALID("{ 0 }");
}

TEST(EnsureFstring, InvalidBadTypeSpecifier) {
    INVALID("{:!}");
    INVALID("{:@}");
    INVALID("{:$}");
    INVALID("{:%}");
    INVALID("{:&}");
    INVALID("{:*}");
    INVALID("{:(}");
    INVALID("{:)}");
    INVALID("{:_}");
    INVALID("{:[}");
    INVALID("{:]}");
    INVALID("{:|}");
    INVALID("{:\\}");
    INVALID("{:/}");
    INVALID("{:?}");
    INVALID("{:~}");
    INVALID("{:,}");
    INVALID("{:;}");
    INVALID("{:'}");
    INVALID("{:\"}");
    INVALID("{:{}");

    INVALID("{:\t}");
    INVALID("{:\n}");

    INVALID("{:h}");
    INVALID("{:i}");
    INVALID("{:j}");
    INVALID("{:k}");
    INVALID("{:l}");
    INVALID("{:m}");
    INVALID("{:n}");
    INVALID("{:q}");
    INVALID("{:r}");
    INVALID("{:t}");
    INVALID("{:u}");
    INVALID("{:v}");
    INVALID("{:w}");
    INVALID("{:y}");
    INVALID("{:z}");

    INVALID("{:B}");
    INVALID("{:C}");
    INVALID("{:D}");
    INVALID("{:H}");
    INVALID("{:I}");
    INVALID("{:J}");
    INVALID("{:K}");
    INVALID("{:L}");
    INVALID("{:M}");
    INVALID("{:N}");
    INVALID("{:O}");
    INVALID("{:P}");
    INVALID("{:Q}");
    INVALID("{:R}");
    INVALID("{:S}");
    INVALID("{:T}");
    INVALID("{:U}");
    INVALID("{:V}");
    INVALID("{:W}");
    INVALID("{:Y}");
    INVALID("{:Z}");
}

TEST(EnsureFstring, InvalidMultipleTypes) {
    INVALID("{:df}");
    INVALID("{:dx}");
    INVALID("{:fd}");
    INVALID("{:xd}");
    INVALID("{:ff}");
    INVALID("{:dd}");
    INVALID("{:xx}");
    INVALID("{:ef}");
    INVALID("{:gs}");
    INVALID("{:sp}");
}

TEST(EnsureFstring, InvalidMisorderedSpec) {
    INVALID("{:d5}");
    INVALID("{:d10}");
    INVALID("{:f5}");

    INVALID("{:f.5}");
    INVALID("{:d#}");
    INVALID("{:d+}");
    INVALID("{:d!}");
    INVALID("{:da}");

    INVALID("{:10.5fx}");
    INVALID("{:10.5f5}");
    INVALID("{:10.5f.2}");
    INVALID("{:+>10.5f!}");

    INVALID("{:10>}");
    INVALID("{:10<}");
    INVALID("{:10^}");
    INVALID("{:.5>10}");
    INVALID("{:5+}");
    INVALID("{:d10}");
    INVALID("{:f.2}");
    INVALID("{:5#}");
}

TEST(EnsureFstring, InvalidDuplicateAndMalformedSyntax) {
    INVALID("{::}");
    INVALID("{:..5}");
    INVALID("{:##}");
    INVALID("{:++}");
    INVALID("{:+-}");
    INVALID("{:-+}");
    INVALID("{:+ }");
    INVALID("{:.5.5}");

    INVALID("{{{");
    INVALID("}{");
    INVALID("}}{");
    INVALID("{}{}{}{}{}{}{}{}{}{");
    INVALID("{{}{");
    INVALID("{}{");
}

TEST(EnsureFstring, InvalidMiscPatterns) {
    INVALID("{:abc}");
    INVALID("{:hello}");
    INVALID("{: hello}");
    INVALID("{:10 }");
    INVALID("{:10,}");
    INVALID("{:10;}");
    INVALID("{:d d}");
    INVALID("{:10f5}");
    INVALID("{:>10>}");
    INVALID("{:aaaaaaaaaa}");

    INVALID("{:*}");
    INVALID("{:_}");
    INVALID("{:@}");
    INVALID("{:!}");

    INVALID("{:{}");

    INVALID("{:**}");
    INVALID("{:__}");
    INVALID("{:!!}");

    INVALID("{:  }");
    INVALID("{: \t}");
    INVALID("{:\t\t}");

    INVALID("{} {");
    INVALID("{:d} {");
    INVALID("{0} {abc}");
    INVALID("text {} text { text");
    INVALID("{} {:!} {}");
    INVALID("{} {abc} {}");
    INVALID("{:d} {:!} {:f}");
    INVALID("{} {} {} {} {} {} {} {} {} {");

    INVALID("{0:d:f}");
    INVALID("{:>10.5f extra}");
    INVALID("{: - }");

    INVALID("{:>10.5f\t}");

    INVALID("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa{");
    INVALID("{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{ ");
}

TEST(EnsureFstring, ConstexprVariables) {
    constexpr const char* valid_str   = VITA_FORMAT_ENSURE_FSTRING("Hello, {}!");
    constexpr const char* invalid_str = VITA_FORMAT_ENSURE_FSTRING("{");

    EXPECT_STREQ(valid_str,   "Hello, {}!");
    EXPECT_STREQ(invalid_str, "__VITA_SYNTAX_ERROR__");

    constexpr const char* ternary_test =
        true ? VITA_FORMAT_ENSURE_FSTRING("{:d}") : "";
    EXPECT_STREQ(ternary_test, "{:d}");

    EXPECT_EQ(sizeof("hello"), 6u);
}

TEST(EnsureFstring, FunctionArgument) {
    EXPECT_STREQ(identity(VITA_FORMAT_ENSURE_FSTRING("{}")), "{}");
    EXPECT_STREQ(identity(VITA_FORMAT_ENSURE_FSTRING("{:d}")), "{:d}");
    EXPECT_STREQ(identity(VITA_FORMAT_ENSURE_FSTRING("{")), "__VITA_SYNTAX_ERROR__");
}

TEST(EnsureFstring, MultiArgFunction) {
    EXPECT_STREQ(
        pick_first(
            VITA_FORMAT_ENSURE_FSTRING("{}"),
            VITA_FORMAT_ENSURE_FSTRING("{:d}")),
        "{}");

    EXPECT_STREQ(
        pick_second(
            VITA_FORMAT_ENSURE_FSTRING("{}"),
            VITA_FORMAT_ENSURE_FSTRING("{:d}")),
        "{:d}");

    EXPECT_STREQ(
        pick_first(
            VITA_FORMAT_ENSURE_FSTRING("{:f}"),
            VITA_FORMAT_ENSURE_FSTRING("{")),
        "{:f}");

    EXPECT_STREQ(
        pick_second(
            VITA_FORMAT_ENSURE_FSTRING("{:f}"),
            VITA_FORMAT_ENSURE_FSTRING("{")),
        "__VITA_SYNTAX_ERROR__");

    EXPECT_STREQ(
        pick_third(
            VITA_FORMAT_ENSURE_FSTRING("{}"),
            VITA_FORMAT_ENSURE_FSTRING("{:d}"),
            VITA_FORMAT_ENSURE_FSTRING("{0:.2f}")),
        "{0:.2f}");
}

TEST(EnsureFstring, TemplateFunction) {
    EXPECT_STREQ(tmpl_identity(VITA_FORMAT_ENSURE_FSTRING("{}")), "{}");
    EXPECT_STREQ(
        tmpl_identity(VITA_FORMAT_ENSURE_FSTRING("{")),
        "__VITA_SYNTAX_ERROR__");
}

TEST(EnsureFstring, ReturnValue) {
    EXPECT_STREQ(get_format(), "{:>10.2f}");
    EXPECT_STREQ(get_bad_format(), "__VITA_SYNTAX_ERROR__");
}

TEST(EnsureFstring, ArrayInitialization) {
    constexpr const char* fmt_array[] = {
        VITA_FORMAT_ENSURE_FSTRING("{}"),
        VITA_FORMAT_ENSURE_FSTRING("{0}"),
        VITA_FORMAT_ENSURE_FSTRING("{:d}"),
        VITA_FORMAT_ENSURE_FSTRING("{:.2f}"),
        VITA_FORMAT_ENSURE_FSTRING("{:*>10}"),
    };

    EXPECT_STREQ(fmt_array[0], "{}");
    EXPECT_STREQ(fmt_array[1], "{0}");
    EXPECT_STREQ(fmt_array[2], "{:d}");
    EXPECT_STREQ(fmt_array[3], "{:.2f}");
    EXPECT_STREQ(fmt_array[4], "{:*>10}");

    constexpr const char* mixed_array[] = {
        VITA_FORMAT_ENSURE_FSTRING("{}"),
        VITA_FORMAT_ENSURE_FSTRING("{"),
        VITA_FORMAT_ENSURE_FSTRING("{:d}"),
        VITA_FORMAT_ENSURE_FSTRING("{abc}"),
    };

    EXPECT_STREQ(mixed_array[0], "{}");
    EXPECT_STREQ(mixed_array[1], "__VITA_SYNTAX_ERROR__");
    EXPECT_STREQ(mixed_array[2], "{:d}");
    EXPECT_STREQ(mixed_array[3], "__VITA_SYNTAX_ERROR__");
}

TEST(EnsureFstring, StructInitialization) {
    constexpr FmtPair pair1 = { "int",   VITA_FORMAT_ENSURE_FSTRING("{:d}") };
    constexpr FmtPair pair2 = { "float", VITA_FORMAT_ENSURE_FSTRING("{:.2f}") };
    constexpr FmtPair pair3 = { "bad",   VITA_FORMAT_ENSURE_FSTRING("{abc}") };

    EXPECT_STREQ(pair1.fmt, "{:d}");
    EXPECT_STREQ(pair2.fmt, "{:.2f}");
    EXPECT_STREQ(pair3.fmt, "__VITA_SYNTAX_ERROR__");

    constexpr FmtPair pair_table[] = {
        { "a", VITA_FORMAT_ENSURE_FSTRING("{}") },
        { "b", VITA_FORMAT_ENSURE_FSTRING("{:x}") },
        { "c", VITA_FORMAT_ENSURE_FSTRING("{name}") },
    };

    EXPECT_STREQ(pair_table[0].fmt, "{}");
    EXPECT_STREQ(pair_table[1].fmt, "{:x}");
    EXPECT_STREQ(pair_table[2].fmt, "__VITA_SYNTAX_ERROR__");
}

TEST(EnsureFstring, TernaryExpression) {
    constexpr const char* nested_ternary =
        true
            ? (false
                ? VITA_FORMAT_ENSURE_FSTRING("{:d}")
                : VITA_FORMAT_ENSURE_FSTRING("{:.5f}"))
            : VITA_FORMAT_ENSURE_FSTRING("{}");

    EXPECT_STREQ(nested_ternary, "{:.5f}");

    constexpr const char* ternary_invalid =
        true ? VITA_FORMAT_ENSURE_FSTRING("{") : "fallback";

    EXPECT_STREQ(ternary_invalid, "__VITA_SYNTAX_ERROR__");
}

TEST(EnsureFstring, SizeofAndPointer) {
    EXPECT_EQ(sizeof("{}"), 3u);

    constexpr const char* sz_ptr = VITA_FORMAT_ENSURE_FSTRING("{}");
    EXPECT_EQ(sizeof(sz_ptr), sizeof(const char*));
}

TEST(EnsureFstring, CommaOperator) {
    constexpr const char* comma_test =
        (42, VITA_FORMAT_ENSURE_FSTRING("{:d}"));
    EXPECT_STREQ(comma_test, "{:d}");

    constexpr const char* comma_chain =
        (VITA_FORMAT_ENSURE_FSTRING("{}"),
         VITA_FORMAT_ENSURE_FSTRING("{:d}"));
    EXPECT_STREQ(comma_chain, "{:d}");
}

TEST(EnsureFstring, AliasMacro) {
    EXPECT_STREQ(VITA_CFSTRING("{}"), "{}");
    EXPECT_STREQ(VITA_CFSTRING("{"), "__VITA_SYNTAX_ERROR__");
    EXPECT_STREQ(VITA_CFSTRING("{:d}"), "{:d}");
    EXPECT_STREQ(identity(VITA_CFSTRING("{:.2f}")), "{:.2f}");
}

TEST(EnsureFstring, ValidityDispatch) {
    EXPECT_TRUE( is_valid_fmt(VITA_FORMAT_ENSURE_FSTRING("{}")));
    EXPECT_TRUE( is_valid_fmt(VITA_FORMAT_ENSURE_FSTRING("{:d}")));
    EXPECT_FALSE(is_valid_fmt(VITA_FORMAT_ENSURE_FSTRING("{")));
    EXPECT_FALSE(is_valid_fmt(VITA_FORMAT_ENSURE_FSTRING("{abc}")));
}

TEST(EnsureFstring, ClassConstructor) {
    constexpr FormatHolder h1(VITA_FORMAT_ENSURE_FSTRING("{:>10}"));
    constexpr FormatHolder h2(VITA_FORMAT_ENSURE_FSTRING("{abc}"));

    EXPECT_STREQ(h1.get(), "{:>10}");
    EXPECT_STREQ(h2.get(), "__VITA_SYNTAX_ERROR__");

    constexpr const char* via_method =
        FormatHolder(VITA_FORMAT_ENSURE_FSTRING("{:.3e}")).get();
    EXPECT_STREQ(via_method, "{:.3e}");
}

TEST(EnsureFstring, DefaultArgument) {
    EXPECT_STREQ(with_default(), "{}");
    EXPECT_STREQ(with_default(VITA_FORMAT_ENSURE_FSTRING("{:x}")), "{:x}");
}

TEST(EnsureFstring, NestedMacro) {
#define MY_FMT(s) VITA_FORMAT_ENSURE_FSTRING(s)

    EXPECT_STREQ(MY_FMT("{}"), "{}");
    EXPECT_STREQ(MY_FMT("{"), "__VITA_SYNTAX_ERROR__");
    EXPECT_STREQ(MY_FMT("{:d}"), "{:d}");

#undef MY_FMT

#define LOG_FMT(s) identity(VITA_FORMAT_ENSURE_FSTRING(s))

    EXPECT_STREQ(LOG_FMT("{}"), "{}");
    EXPECT_STREQ(LOG_FMT("{:d}"), "{:d}");
    EXPECT_STREQ(LOG_FMT("{"), "__VITA_SYNTAX_ERROR__");

#undef LOG_FMT
}

TEST(EnsureFstring, PointerArithmetic) {
    constexpr const char* arith_base =
        VITA_FORMAT_ENSURE_FSTRING("Hello, {}!");
    EXPECT_EQ(arith_base[0], 'H');
    EXPECT_EQ(arith_base[7], '{');
    EXPECT_EQ(arith_base[8], '}');
}

TEST(EnsureFstring, SequentialDeclarations) {
    constexpr const char* seq1 = VITA_FORMAT_ENSURE_FSTRING("{}");
    constexpr const char* seq2 = VITA_FORMAT_ENSURE_FSTRING("{:d}");
    constexpr const char* seq3 = VITA_FORMAT_ENSURE_FSTRING("{0:.2f}");
    constexpr const char* seq4 = VITA_FORMAT_ENSURE_FSTRING("{:*^20}");

    EXPECT_STREQ(seq1, "{}");
    EXPECT_STREQ(seq2, "{:d}");
    EXPECT_STREQ(seq3, "{0:.2f}");
    EXPECT_STREQ(seq4, "{:*^20}");
}

TEST(EnsureFstring, TemplateBoolArg) {
    EXPECT_TRUE(
        (BoolCheck<streq(VITA_FORMAT_ENSURE_FSTRING("{}"), "{}")>::value));

    EXPECT_TRUE(
        (BoolCheck<streq(VITA_FORMAT_ENSURE_FSTRING("{"),
            "__VITA_SYNTAX_ERROR__")>::value));
}

TEST(EnsureFstring, ComplexConditionalExpr) {
    constexpr const char* complex_expr =
        is_valid_fmt(VITA_FORMAT_ENSURE_FSTRING("{:d}"))
            ? VITA_FORMAT_ENSURE_FSTRING("{:.2f}")
            : VITA_FORMAT_ENSURE_FSTRING("{}");

    EXPECT_STREQ(complex_expr, "{:.2f}");

    constexpr const char* complex_expr2 =
        is_valid_fmt(VITA_FORMAT_ENSURE_FSTRING("{abc}"))
            ? VITA_FORMAT_ENSURE_FSTRING("{:d}")
            : VITA_FORMAT_ENSURE_FSTRING("{:x}");

    EXPECT_STREQ(complex_expr2, "{:x}");
}

TEST(EnsureFstring, NullCheckAndIndexing) {
    EXPECT_NE(VITA_FORMAT_ENSURE_FSTRING("{}"), nullptr);
    EXPECT_NE(VITA_FORMAT_ENSURE_FSTRING("{"), nullptr);
    EXPECT_EQ(VITA_FORMAT_ENSURE_FSTRING("{}")[0], '{');
    EXPECT_EQ(VITA_FORMAT_ENSURE_FSTRING("{}")[1], '}');
    EXPECT_EQ(VITA_FORMAT_ENSURE_FSTRING("{}")[2], '\0');
}

TEST(EnsureFstring, ChainedLogicalOperators) {
    EXPECT_TRUE(
        streq(VITA_FORMAT_ENSURE_FSTRING("{}"), "{}") &&
        streq(VITA_FORMAT_ENSURE_FSTRING("{:d}"), "{:d}") &&
        streq(VITA_FORMAT_ENSURE_FSTRING("{:.2f}"), "{:.2f}"));

    EXPECT_TRUE(
        streq(VITA_FORMAT_ENSURE_FSTRING("{"), "__VITA_SYNTAX_ERROR__") ||
        streq(VITA_FORMAT_ENSURE_FSTRING("{}"), "this won't match"));
}

TEST(EnsureFstring, MultiLayerPassThrough) {
    EXPECT_STREQ(
        layer1(VITA_FORMAT_ENSURE_FSTRING("{:*^20.5f}")),
        "{:*^20.5f}");
}

TEST(EnsureFstring, ImplicitConversion) {
    constexpr ImplicitStr imp(VITA_FORMAT_ENSURE_FSTRING("{:d}"));
    EXPECT_STREQ(imp.data, "{:d}");
}

TEST(EnsureFstringRuntime, FunctionArg) {
    EXPECT_EQ(std::strlen(VITA_FORMAT_ENSURE_FSTRING("{}")), 2u);
    EXPECT_EQ(std::strlen(VITA_FORMAT_ENSURE_FSTRING("{:d}")), 4u);
    EXPECT_GT(std::strlen(VITA_FORMAT_ENSURE_FSTRING("{")), 0u);

    EXPECT_STREQ(VITA_FORMAT_ENSURE_FSTRING("{}"), "{}");
    EXPECT_STREQ(VITA_FORMAT_ENSURE_FSTRING("{:d}"), "{:d}");
    EXPECT_STREQ(VITA_FORMAT_ENSURE_FSTRING("{"), "__VITA_SYNTAX_ERROR__");
}

TEST(EnsureFstringRuntime, LocalVariable) {
    const char* local_valid = VITA_FORMAT_ENSURE_FSTRING("{:.3f}");
    EXPECT_STREQ(local_valid, "{:.3f}");

    const char* local_invalid = VITA_FORMAT_ENSURE_FSTRING("{abc}");
    EXPECT_STREQ(local_invalid, "__VITA_SYNTAX_ERROR__");

    auto auto_fmt = VITA_FORMAT_ENSURE_FSTRING("{:08x}");
    EXPECT_STREQ(auto_fmt, "{:08x}");
}

TEST(EnsureFstringRuntime, Loop) {
    const char* formats[] = {
        VITA_FORMAT_ENSURE_FSTRING("{}"),
        VITA_FORMAT_ENSURE_FSTRING("{:d}"),
        VITA_FORMAT_ENSURE_FSTRING("{:.2f}"),
    };
    const char* expected[] = { "{}", "{:d}", "{:.2f}" };

    for (int i = 0; i < 3; ++i) {
        EXPECT_STREQ(formats[i], expected[i]);
    }
}

TEST(EnsureFstringRuntime, FunctionPointer) {
    FormatCallback cb = echo_format;
    const char* result = cb(VITA_FORMAT_ENSURE_FSTRING("{:>10s}"));
    EXPECT_STREQ(result, "{:>10s}");
}

TEST(EnsureFstringRuntime, Lambda) {
    auto make_fmt = []() {
        return VITA_FORMAT_ENSURE_FSTRING("{:+.5e}");
    };
    EXPECT_STREQ(make_fmt(), "{:+.5e}");

    auto use_fmt = [](const char* f) { return f; };
    EXPECT_STREQ(
        use_fmt(VITA_FORMAT_ENSURE_FSTRING("{:x}")), "{:x}");

    const char* captured = VITA_FORMAT_ENSURE_FSTRING("{:04d}");
    auto with_capture = [captured]() { return captured; };
    EXPECT_STREQ(with_capture(), "{:04d}");

    auto generic = [](auto fmt) { return fmt; };
    EXPECT_STREQ(
        generic(VITA_FORMAT_ENSURE_FSTRING("{}")), "{}");
}

TEST(EnsureFstringRuntime, VariadicCFunction) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%s",
        VITA_FORMAT_ENSURE_FSTRING("{:.2f}"));
    EXPECT_STREQ(buf, "{:.2f}");
}

TEST(EnsureFstringRuntime, StaticLocal) {
    EXPECT_STREQ(get_static_fmt(), "{:>20}");
    EXPECT_EQ(get_static_fmt(), get_static_fmt());
}

TEST(EnsureFstringRuntime, SwitchLike) {
    const char* fmt = VITA_FORMAT_ENSURE_FSTRING("{:d}");
    ASSERT_STRNE(fmt, "__VITA_SYNTAX_ERROR__");
    EXPECT_STREQ(fmt, "{:d}");
}

TEST(EnsureFstringRuntime, StructMemberAssign) {
    FmtPair p;
    p.label = "runtime";
    p.fmt = VITA_FORMAT_ENSURE_FSTRING("{:08X}");
    EXPECT_STREQ(p.fmt, "{:08X}");
}
