#ifndef X86_KERNEL_SPAN_HH
#define X86_KERNEL_SPAN_HH

#include <span>


template <typename T, size_t N>
T* safe_at(const std::span<T, N>& a, size_t i) {
    if (i >= a.size())
        throw std::out_of_range("span bound exceeded");
    return &a[i];
}


template <typename T, size_t N>
std::span<T> safe_subspan(const std::span<T, N>& a, size_t offset, size_t n) {
    // TODO: arithmetic overflow checks
    if (offset + n >= a.size())
        throw std::out_of_range("span bound exceeded");
    return a.subspan(offset, n);
}


struct Count { size_t n; };
struct Bytes { size_t n; };


template <typename T>
std::span<const T> extract_span(std::span<const unsigned char> raw,
                                size_t offset, Count count) {
    // TODO: arithmetic overflow checks
    auto address = reinterpret_cast<uintptr_t>(raw.data()) + offset;
    if (address % alignof(T) != 0)
        throw std::runtime_error("invalid alignment");
    if (raw.size() < offset + count.n * sizeof(T))
        throw std::runtime_error("exceeded file bounds");
    return std::span(reinterpret_cast<const T*>(address), count.n);
}


template <typename T>
std::span<const T> extract_span(std::span<const unsigned char> raw,
                                size_t offset, Bytes bytes) {
    if (bytes.n % sizeof(T) != 0)
        throw std::runtime_error("invalid size");
    return extract_span<T>(raw, offset, Count{bytes.n / sizeof(T)});
}


template <typename T>
const T* extract_one(std::span<const unsigned char> raw, size_t offset) {
    auto values = extract_span<T>(raw, offset, Count{1});
    return &values[0];
}


template <typename T>
T extract_unaligned(std::span<const unsigned char> raw, size_t offset) {
    T result;
    auto bytes = safe_subspan<const unsigned char>(raw, offset, sizeof(T));
    memcpy(&result, bytes.data(), sizeof(T));
    return result;
}


#endif  // X86_KERNEL_SPAN_HH
