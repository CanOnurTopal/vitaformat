// benchmarks/benchmark.cpp
// Simple benchmark for Vita::format
// Compile: g++ -std=c++11 -O2 -DNDEBUG -I.. benchmark.cpp -o benchmark

#include "../vita/format.hpp"
#include <chrono>
#include <iostream>
#include <cstdio>
#include <sstream>

using Clock = std::chrono::high_resolution_clock;

template <typename Func>
double benchmark(const char* name, int iterations, Func func) {
    // Warmup
    for (int i = 0; i < iterations / 10; ++i) {
        func();
    }

    auto start = Clock::now();
    for (int i = 0; i < iterations; ++i) {
        func();
    }
    auto end = Clock::now();

    double ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    double avg_ns = ns / iterations;

    std::cout << name << ": " << avg_ns << " ns/op\n";
    return avg_ns;
}

// Prevent compiler from optimizing away
volatile char sink;
void escape(const std::string& s) {
    if (!s.empty()) sink = s[0];
}

int main() {
    const int ITERATIONS = 1000000;

    std::cout << "Vita::format Benchmark\n";
    std::cout << "======================\n";
    std::cout << "Iterations: " << ITERATIONS << "\n\n";

    // Benchmark: format("{}", 42)
    std::cout << "--- Integer formatting ---\n";

    benchmark("Vita::format(\"{}\", 42)", ITERATIONS, []() {
        escape(Vita::format("{}", 42));
    });

    benchmark("Vita::formatc(\"{}\", 42)", ITERATIONS, []() {
        escape(Vita::formatc("{}", 42));
    });

    benchmark("sprintf (int)", ITERATIONS, []() {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", 42);
        sink = buf[0];
    });

    benchmark("std::to_string(42)", ITERATIONS, []() {
        escape(std::to_string(42));
    });

    std::cout << "\n--- Float formatting ---\n";

    benchmark("Vita::format(\"{}\", 3.14)", ITERATIONS, []() {
        escape(Vita::format("{}", 3.14));
    });

    benchmark("Vita::formatc(\"{}\", 3.14)", ITERATIONS, []() {
        escape(Vita::formatc("{}", 3.14));
    });

    benchmark("sprintf (float)", ITERATIONS, []() {
        char buf[32];
        snprintf(buf, sizeof(buf), "%g", 3.14);
        sink = buf[0];
    });

    std::cout << "\n--- String formatting ---\n";

    benchmark("Vita::format(\"{} {} {}\", ...)", ITERATIONS, []() {
        escape(Vita::format("{} {} {}", "a", "b", "c"));
    });

    benchmark("Vita::formatc(\"{} {} {}\", ...)", ITERATIONS, []() {
        escape(Vita::formatc("{} {} {}", "a", "b", "c"));
    });

    benchmark("sprintf (3 strings)", ITERATIONS, []() {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s %s %s", "a", "b", "c");
        sink = buf[0];
    });

    std::cout << "\n--- Complex formatting ---\n";

    benchmark("Vita::format(\"The answer is {:d}.\", 42)", ITERATIONS, []() {
        escape(Vita::format("The answer is {:d}.", 42));
    });

    benchmark("Vita::formatc(\"The answer is {:d}.\", 42)", ITERATIONS, []() {
        escape(Vita::formatc("The answer is {:d}.", 42));
    });

    benchmark("sprintf complex", ITERATIONS, []() {
        char buf[64];
        snprintf(buf, sizeof(buf), "The answer is %d.", 42);
        sink = buf[0];
    });

    std::cout << "\n--- Hello World ---\n";

    benchmark("Vita::format(\"{} {}!\", ...)", ITERATIONS, []() {
        escape(Vita::format("{} {}!", "Hello", "world"));
    });

    benchmark("Vita::formatc(\"{} {}!\", ...)", ITERATIONS, []() {
        escape(Vita::formatc("{} {}!", "Hello", "world"));
    });

    std::cout << "\n--- ostringstream comparison ---\n";

    benchmark("ostringstream (int)", ITERATIONS / 10, []() {
        std::ostringstream oss;
        oss << 42;
        escape(oss.str());
    });

    benchmark("ostringstream (3 strings)", ITERATIONS / 10, []() {
        std::ostringstream oss;
        oss << "a" << " " << "b" << " " << "c";
        escape(oss.str());
    });

    std::cout << "\n--- Large integers ---\n";

    benchmark("Vita::format uint64_max", ITERATIONS, []() {
        escape(Vita::format("{}", 18446744073709551615ULL));
    });

    benchmark("sprintf uint64_max", ITERATIONS, []() {
        char buf[32];
        snprintf(buf, sizeof(buf), "%llu", 18446744073709551615ULL);
        sink = buf[0];
    });

    std::cout << "\n--- Hex formatting ---\n";

    benchmark("Vita::format(\"{:x}\", 0xdeadbeef)", ITERATIONS, []() {
        escape(Vita::format("{:x}", 0xdeadbeef));
    });

    benchmark("sprintf hex", ITERATIONS, []() {
        char buf[32];
        snprintf(buf, sizeof(buf), "%x", 0xdeadbeef);
        sink = buf[0];
    });

    std::cout << "\n======================\n";
    std::cout << "Benchmark complete.\n";

    return 0;
}
