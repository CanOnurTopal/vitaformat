// vita/detail/output.hpp
// string buffer with small buffer optimization
#ifndef VITA_DETAIL_OUTPUT_HPP
#define VITA_DETAIL_OUTPUT_HPP

#include <cstddef>
#include <cstring>
#include <string>

#ifndef VITA_FORMAT_SBO_SIZE
#define VITA_FORMAT_SBO_SIZE 256
#endif

namespace Vita {
namespace detail {

class FormatOutput {
public:
    FormatOutput() noexcept
        : size_(0), capacity_(VITA_FORMAT_SBO_SIZE),
          data_(sbo_), heap_(false) {}

    ~FormatOutput() {
        if (heap_) delete[] data_;
    }

    FormatOutput(const FormatOutput&) = delete;
    FormatOutput& operator=(const FormatOutput&) = delete;

    FormatOutput(FormatOutput&& other) noexcept
        : size_(other.size_), capacity_(other.capacity_), heap_(other.heap_)
    {
        if (other.heap_) {
            data_ = other.data_;
            other.data_ = other.sbo_;
            other.heap_ = false;
        } else {
            std::memcpy(sbo_, other.sbo_, other.size_);
            data_ = sbo_;
        }
        other.size_ = 0;
        other.capacity_ = VITA_FORMAT_SBO_SIZE;
    }

    void append(char c) {
        ensure(1);
        data_[size_++] = c;
    }

    void append(const char* s, std::size_t len) {
        if (len == 0) return;
        ensure(len);
        std::memcpy(data_ + size_, s, len);
        size_ += len;
    }

    void append(const char* s) {
        append(s, std::strlen(s));
    }

    void append_fill(char c, std::size_t n) {
        if (n == 0) return;
        ensure(n);
        std::memset(data_ + size_, c, n);
        size_ += n;
    }

    void reserve(std::size_t n) { ensure(n); }

    std::string finish() {
        std::string result(data_, size_);
        size_ = 0;
        return result;
    }

    char* grow(std::size_t n) {
        ensure(n);
        char* p = data_ + size_;
        size_ += n;
        return p;
    }

    void shrink(std::size_t n) { size_ -= n; }

    std::size_t size() const noexcept { return size_; }
    const char* data() const noexcept { return data_; }

private:
    void ensure(std::size_t extra) {
        std::size_t need = size_ + extra;
        if (need <= capacity_) return;

        std::size_t cap = capacity_ + capacity_ / 2;
        if (cap < need) cap = need;

        char* buf = new char[cap];
        std::memcpy(buf, data_, size_);
        if (heap_) delete[] data_;

        data_ = buf;
        capacity_ = cap;
        heap_ = true;
    }

    char sbo_[VITA_FORMAT_SBO_SIZE];
    std::size_t size_;
    std::size_t capacity_;
    char* data_;
    bool heap_;
};

} // namespace detail
} // namespace Vita

#endif
