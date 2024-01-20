#ifndef X86_KERNEL_RAII_H
#define X86_KERNEL_RAII_H

#include <optional>


template <typename T>
class RAII {
private:
    std::optional<T> _data;

public:
    RAII() = delete;
    RAII(const RAII&) = delete;
    RAII& operator=(const RAII&) = delete;

    explicit RAII(T data) : _data(std::move(data)) {}

    RAII(RAII&& other)
        : _data(other._data) {
        other._data.reset();
    }

    RAII& operator=(RAII&& other) {
        if (_data.has_value()) {
            _data->release();
            _data.reset();
        }
        _data = std::move(other._data);
        other._data.reset();
    }

    ~RAII() {
        if (_data.has_value())
            _data->release();
    }

    T* operator->() { return _data.operator->(); }
    const T* operator->() const { return _data.operator->(); }

    T& operator*() { return *_data; }
    const T& operator*() const { return *_data; }
};

#endif  // X86_KERNEL_RAII_H
