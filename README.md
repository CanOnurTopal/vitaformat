# VitaFormat User Guide

A fast, portable C++11 string formatting library.

## Table of Contents

- [Quick Start](#quick-start)
- [Installation](#installation)
- [Basic Usage](#basic-usage)
- [Format Syntax](#format-syntax)
  - [Format Syntax Examples](#format-syntax-examples)
  - [Combining Format Specifiers](#combining-format-specifiers)
  - [Real-World Combined Examples](#real-world-combined-examples)
- [Supported Types](#supported-types)
- [Format Specifications](#format-specifications)
- [Advanced Usage](#advanced-usage)
- [Configuration](#configuration)
- [Building from Source](#building-from-source)

---

## Quick Start

```cpp
#include "vita/format.hpp"

int main() {
    // Simple string formatting
    std::string greeting = Vita::format("Hello, {}!", "World");
    // Result: "Hello, World!"

    // Multiple arguments
    std::string info = Vita::format("{} is {} years old", "Alice", 30);
    // Result: "Alice is 30 years old"

    // Formatted numbers
    std::string price = Vita::format("Total: ${:.2f}", 19.99);
    // Result: "Total: $19.99"

    return 0;
}
```

---

## Installation

### Header-Only (Recommended)

Simply copy the `vita/` folder into your project and include the header:

```cpp
#include "vita/format.hpp"
```

### CMake Integration

**Option 1: Subdirectory**
```cmake
add_subdirectory(path/to/vitaformat)
target_link_libraries(your_target Vita::format)
```

**Option 2: find_package**
```bash
# Install first
cd vitaformat && mkdir build && cd build
cmake .. && cmake --install . --prefix /usr/local
```

```cmake
find_package(VitaFormat REQUIRED)
target_link_libraries(your_target Vita::format)
```

---

## Basic Usage

### The `format()` Function

The primary function is `Vita::format()` which takes a format string and arguments:

```cpp
std::string result = Vita::format("pattern", arg1, arg2, ...);
```

### Placeholder Syntax

Use `{}` as placeholders for arguments:

```cpp
Vita::format("Name: {}", "Bob");           // "Name: Bob"
Vita::format("{} + {} = {}", 1, 2, 3);     // "1 + 2 = 3"
```

### Explicit Argument Indices

Reference arguments by position using `{0}`, `{1}`, etc.:

```cpp
Vita::format("{0} vs {1}: {0} wins!", "A", "B");  // "A vs B: A wins!"
Vita::format("{1}, {0}", "world", "Hello");       // "Hello, world"
```

### Escaping Braces

Use double braces to output literal braces:

```cpp
Vita::format("Use {{}} for placeholders");  // "Use {} for placeholders"
Vita::format("JSON: {{\"key\": {}}}", 42);  // "JSON: {\"key\": 42}"
```

---

## Format Syntax

The full format specification syntax:

```
{[index]:[fill][align][sign][#][0][width][.precision][type]}
```

| Component    | Description                              | Example        |
|--------------|------------------------------------------|----------------|
| `index`      | Argument position (0-based)              | `{0}`, `{1}`   |
| `fill`       | Padding character                        | `{:*>10}`      |
| `align`      | Alignment: `<` `>` `^` `=`               | `{:<10}`       |
| `sign`       | Sign display: `+` `-` ` `                | `{:+}`         |
| `#`          | Alternate form (prefix for hex/oct/bin)  | `{:#x}`        |
| `0`          | Zero-padding                             | `{:05}`        |
| `width`      | Minimum field width                      | `{:10}`        |
| `.precision` | Decimal places or string max length      | `{:.2f}`       |
| `type`       | Format type specifier                    | `{:x}`, `{:f}` |

### Format Syntax Examples

**Basic examples for each component:**

```cpp
// Index only
Vita::format("{0} {1} {0}", "A", "B");        // "A B A"

// Fill + Align
Vita::format("{:_<10}", "test");              // "test______"
Vita::format("{:_>10}", "test");              // "______test"
Vita::format("{:_^10}", "test");              // "___test___"

// Sign
Vita::format("{:+d}", 42);                    // "+42"
Vita::format("{: d}", 42);                    // " 42"

// Alternate form (#)
Vita::format("{:#x}", 255);                   // "0xff"
Vita::format("{:#o}", 64);                    // "0o100"
Vita::format("{:#b}", 5);                     // "0b101"

// Zero-padding (0)
Vita::format("{:08d}", 42);                   // "00000042"
Vita::format("{:08x}", 255);                  // "000000ff"

// Width
Vita::format("{:10d}", 42);                   // "        42"
Vita::format("{:10s}", "hi");                 // "hi        "

// Precision
Vita::format("{:.3f}", 3.14159);              // "3.142"
Vita::format("{:.5s}", "Hello World");        // "Hello"

// Type
Vita::format("{:x}", 255);                    // "ff"
Vita::format("{:e}", 1234.5);                 // "1.234500e+03"
```

### Combining Format Specifiers

The real power comes from combining multiple specifiers together.

**Fill + Align + Width:**
```cpp
Vita::format("[{:*<15}]", "left");            // "[left***********]"
Vita::format("[{:*>15}]", "right");           // "[**********right]"
Vita::format("[{:*^15}]", "center");          // "[*****center****]"
Vita::format("[{:=>10}]", 42);                // "[========42]"
```

**Sign + Zero-padding + Width:**
```cpp
Vita::format("{:+010d}", 42);                 // "+000000042"
Vita::format("{:+010d}", -42);                // "-000000042"
Vita::format("{: 010d}", 42);                 // " 000000042"
```

**Alternate form + Zero-padding + Width:**
```cpp
Vita::format("{:#010x}", 255);                // "0x000000ff"
Vita::format("{:#010b}", 5);                  // "0b00000101"
Vita::format("{:#010o}", 64);                 // "0o00000100"
```

**Width + Precision (floats):**
```cpp
Vita::format("{:10.2f}", 3.14159);            // "      3.14"
Vita::format("{:10.4f}", 3.14159);            // "    3.1416"
Vita::format("{:010.2f}", 3.14159);           // "0000003.14"
```

**Fill + Align + Width + Precision:**
```cpp
Vita::format("{:_>10.2f}", 3.14159);          // "______3.14"
Vita::format("{:_<10.2f}", 3.14159);          // "3.14______"
Vita::format("{:_^10.2f}", 3.14159);          // "___3.14___"
```

**Sign + Width + Precision + Type:**
```cpp
Vita::format("{:+12.4e}", 1234.5);            // " +1.2345e+03"
Vita::format("{:+12.4E}", 1234.5);            // " +1.2345E+03"
Vita::format("{:+12.2f}", -99.9);             // "      -99.90"
```

**Index + Full spec:**
```cpp
Vita::format("{0:+08.2f} | {1:_^10}", 3.14, "test");
// "+0003.14 | ___test___"

Vita::format("{1:*>10} = {0:#06x}", 255, "hex");
// "*******hex = 0x00ff"
```

**Sign-aware padding (=) with sign:**
```cpp
Vita::format("{:=+10d}", 42);                 // "+       42"
Vita::format("{:=+10d}", -42);                // "-       42"
Vita::format("{:0=+10d}", 42);                // "+000000042"
```

### Real-World Combined Examples

**Financial formatting:**
```cpp
// Right-aligned currency with 2 decimals
Vita::format("${:>12.2f}", 1234.56);          // "$     1234.56"
Vita::format("${:>12.2f}", -99.99);           // "$      -99.99"

// Accounting style (sign-aware)
Vita::format("{:=+12.2f}", 1234.56);          // "+     1234.56"
Vita::format("{:=+12.2f}", -1234.56);         // "-     1234.56"
```

**ID/Code formatting:**
```cpp
// Order ID: zero-padded 8 digits
Vita::format("ORD-{:08d}", 1234);             // "ORD-00001234"

// Product code: uppercase hex, 4 digits
Vita::format("SKU-{:04X}", 171);              // "SKU-00AB"

// Serial number with prefix
Vita::format("SN:{:#010x}", 0xABCD);          // "SN:0x0000abcd"
```

**Data display:**
```cpp
// Memory address
Vita::format("0x{:016X}", 0x7FFE12345678);    // "0x00007FFE12345678"

// Percentage with sign
Vita::format("{:+.1f}%", 12.5);               // "+12.5%"
Vita::format("{:+.1f}%", -3.2);               // "-3.2%"

// Binary flags
Vita::format("Flags: {:08b}", 0b10110011);    // "Flags: 10110011"
```

**Table columns:**
```cpp
// Mixed column types
Vita::format("{:<20} {:>8} {:>10.2f}", "Product Name", 100, 29.99);
// "Product Name               100      29.99"

// Centered headers with fill
Vita::format("{:=^30}", " REPORT ");          // "=========== REPORT ==========="
```

**Debug output:**
```cpp
// Variable inspection
int val = 255;
Vita::format("[{0}] dec:{0:d} hex:{0:#x} oct:{0:#o} bin:{0:#b}", val);
// "[255] dec:255 hex:0xff oct:0o377 bin:0b11111111"

// Coordinate display
Vita::format("({:+8.3f}, {:+8.3f})", -12.5, 100.25);
// "(  -12.500, +100.250)"
```

---

## Supported Types

### Strings

```cpp
Vita::format("{}", "hello");              // "hello"
Vita::format("{}", std::string("world")); // "world"
Vita::format("{:.3}", "abcdef");          // "abc" (truncated)
Vita::format("{:10}", "hi");              // "hi        " (padded)
```

### Integers

```cpp
Vita::format("{}", 42);        // "42"
Vita::format("{}", -17);       // "-17"
Vita::format("{:05}", 7);      // "00007" (zero-padded)
Vita::format("{:+}", 5);       // "+5" (explicit sign)
```

**Number Bases:**

| Type | Description        | Example                    |
|------|--------------------|----------------------------|
| `d`  | Decimal (default)  | `{:d}` -> "42"             |
| `x`  | Lowercase hex      | `{:x}` -> "2a"             |
| `X`  | Uppercase hex      | `{:X}` -> "2A"             |
| `o`  | Octal              | `{:o}` -> "52"             |
| `b`  | Binary             | `{:b}` -> "101010"         |

```cpp
Vita::format("{:x}", 255);     // "ff"
Vita::format("{:X}", 255);     // "FF"
Vita::format("{:#x}", 255);    // "0xff" (with prefix)
Vita::format("{:08b}", 42);    // "00101010"
```

### Floating-Point

```cpp
Vita::format("{}", 3.14159);      // "3.14159"
Vita::format("{:.2f}", 3.14159);  // "3.14" (2 decimal places)
Vita::format("{:10.2f}", 3.14);   // "      3.14" (width 10)
```

**Float Format Types:**

| Type | Description               | Example                     |
|------|---------------------------|-----------------------------|
| `f`  | Fixed-point               | `{:.2f}` -> "3.14"          |
| `e`  | Scientific (lowercase)    | `{:.2e}` -> "3.14e+00"      |
| `E`  | Scientific (uppercase)    | `{:.2E}` -> "3.14E+00"      |
| `g`  | Shortest representation   | `{:g}` -> "3.14159"         |

```cpp
Vita::format("{:e}", 1234.5);      // "1.234500e+03"
Vita::format("{:.2E}", 1234.5);    // "1.23E+03"
```

### Booleans

```cpp
Vita::format("{}", true);    // "true"
Vita::format("{}", false);   // "false"
Vita::format("{:d}", true);  // "1" (as integer)
```

### Characters

```cpp
Vita::format("{}", 'A');     // "A"
Vita::format("{:c}", 65);    // "A" (int as char)
Vita::format("{:d}", 'A');   // "65" (char as int)
```

### Pointers

```cpp
int x = 42;
Vita::format("{}", &x);           // "0x7ffd5e8a1234"
Vita::format("{}", nullptr);      // "0x0"
Vita::format("{}", (void*)0x1000); // "0x1000"
```

---

## Format Specifications

### Alignment

| Specifier | Meaning                        | Example               |
|-----------|--------------------------------|-----------------------|
| `<`       | Left-align                     | `{:<10}` -> "text      " |
| `>`       | Right-align                    | `{:>10}` -> "      text" |
| `^`       | Center                         | `{:^10}` -> "   text   " |
| `=`       | Sign-aware (numbers only)      | `{:=+10}` -> "+     42" |

```cpp
Vita::format("[{:<10}]", "left");    // "[left      ]"
Vita::format("[{:>10}]", "right");   // "[     right]"
Vita::format("[{:^10}]", "center");  // "[  center  ]"
```

### Fill Characters

Combine a fill character with alignment:

```cpp
Vita::format("{:*<10}", "hi");   // "hi********"
Vita::format("{:*>10}", "hi");   // "********hi"
Vita::format("{:*^10}", "hi");   // "****hi****"
Vita::format("{:-^20}", "Title"); // "-------Title--------"
```

### Sign Options

| Specifier | Meaning                           |
|-----------|-----------------------------------|
| `+`       | Show sign for positive and negative |
| `-`       | Show sign only for negative (default) |
| ` `       | Space for positive, minus for negative |

```cpp
Vita::format("{:+}", 42);   // "+42"
Vita::format("{:+}", -42);  // "-42"
Vita::format("{: }", 42);   // " 42"
Vita::format("{: }", -42);  // "-42"
```

### Width and Precision

```cpp
// Width
Vita::format("{:10}", 42);       // "        42" (right-aligned by default for numbers)
Vita::format("{:10}", "hi");     // "hi        " (left-aligned by default for strings)

// Precision for floats
Vita::format("{:.4f}", 3.14159); // "3.1416"
Vita::format("{:8.2f}", 3.14);   // "    3.14"

// Precision for strings (truncation)
Vita::format("{:.5}", "Hello World"); // "Hello"
```

---

## Advanced Usage

### Compile-Time Optimized Formatting

Use `formatc()` or `VITA_FORMAT` for better performance when the format string is known at compile time:

```cpp
// Template specialization per format string
std::string s = Vita::formatc("Value: {}", 42);

// Macro wrapper
std::string s = VITA_FORMAT("Value: {}", 42);
```

### Custom Type Formatting

Specialize the `Formatter` template for custom types:

```cpp
struct Point { int x, y; };

namespace Vita {
template<>
struct Formatter<Point> {
    static void format(FormatOutput& out, const Point& p, const FormatSpec& spec) {
        out.append('(');
        Formatter<int>::format(out, p.x, spec);
        out.append(", ");
        Formatter<int>::format(out, p.y, spec);
        out.append(')');
    }
};
}

// Now works:
Vita::format("Location: {}", Point{10, 20});  // "Location: (10, 20)"
```

### Enum Formatting

```cpp
enum class Color { Red, Green, Blue };

namespace Vita {
template<>
struct Formatter<Color> {
    static void format(FormatOutput& out, Color c, const FormatSpec&) {
        switch(c) {
            case Color::Red:   out.append("Red"); break;
            case Color::Green: out.append("Green"); break;
            case Color::Blue:  out.append("Blue"); break;
        }
    }
};
}

Vita::format("Color: {}", Color::Red);  // "Color: Red"
```

---

## Configuration

### Compile-Time Options

Define these macros before including the header:

| Macro                        | Default | Description                          |
|------------------------------|---------|--------------------------------------|
| `VITA_FORMAT_SBO_SIZE`       | 256     | Small buffer size (stack allocation) |
| `VITA_FORMAT_MAX_ARGS`       | 32      | Maximum format arguments             |
| `VITA_FORMAT_NO_EXCEPTIONS`  | (unset) | Disable exceptions                   |

```cpp
#define VITA_FORMAT_SBO_SIZE 512
#define VITA_FORMAT_NO_EXCEPTIONS
#include "vita/format.hpp"
```

---

## Building from Source

### Requirements

- C++11 compatible compiler (GCC 4.8+, Clang 3.3+, MSVC 2015+)
- CMake 3.14+ (for building tests/benchmarks)

### Build Commands

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run tests
ctest --output-on-failure

# Run benchmarks
./vita_format_benchmark

# Install
cmake --install . --prefix /usr/local
```

### CMake Options

| Option                          | Default | Description               |
|---------------------------------|---------|---------------------------|
| `VITA_FORMAT_BUILD_TESTS`       | ON*     | Build unit tests          |
| `VITA_FORMAT_BUILD_BENCHMARKS`  | ON*     | Build benchmarks          |
| `VITA_FORMAT_INSTALL`           | ON*     | Generate install target   |

*Default ON when building as top-level project

```bash
# Build without tests
cmake -DVITA_FORMAT_BUILD_TESTS=OFF ..

# Build without benchmarks
cmake -DVITA_FORMAT_BUILD_BENCHMARKS=OFF ..
```

---

## Examples

### Logging

```cpp
void log(const std::string& level, const std::string& msg) {
    std::cout << Vita::format("[{}] {}: {}\n",
        std::time(nullptr), level, msg);
}

log("INFO", "Application started");
```

### Table Formatting

```cpp
std::cout << Vita::format("{:<20} {:>10} {:>10}\n", "Name", "Qty", "Price");
std::cout << Vita::format("{:-<42}\n", "");
std::cout << Vita::format("{:<20} {:>10} {:>10.2f}\n", "Widget", 5, 19.99);
std::cout << Vita::format("{:<20} {:>10} {:>10.2f}\n", "Gadget", 3, 29.99);
```

Output:
```
Name                        Qty      Price
------------------------------------------
Widget                        5      19.99
Gadget                        3      29.99
```

### Number Formatting

```cpp
// Currency
Vita::format("${:,.2f}", 1234567.89);  // "$1234567.89"

// Percentages
Vita::format("{:.1f}%", 0.856 * 100);  // "85.6%"

// Hex dump
for (int i = 0; i < 16; ++i) {
    std::cout << Vita::format("{:02x} ", i);
}
// "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f"
```

---

## Error Handling

By default, format errors throw exceptions:

```cpp
try {
    Vita::format("{0} {2}", "a", "b");  // Missing index 2
} catch (const std::exception& e) {
    std::cerr << "Format error: " << e.what() << '\n';
}
```

With `VITA_FORMAT_NO_EXCEPTIONS`, errors produce inline error markers instead.

---

## Performance Tips

1. **Use `formatc()` for static format strings** - enables compiler optimizations
2. **Pre-size output when possible** - reduces reallocations
3. **Prefer `{}` over explicit indices** - slightly faster parsing
4. **Keep format strings simple** - complex specs add overhead

---

## License

See the project repository for license information.
