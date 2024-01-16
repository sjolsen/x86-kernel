#ifndef X86_KERNEL_POSIX_IO_HH
#define X86_KERNEL_POSIX_IO_HH

#include <filesystem>
#include <span>

#include "raii.hh"


class OpenFile {
private:
    int _fd;

    explicit OpenFile(int fd) : _fd(fd) {}

    friend RAII<OpenFile>;
    void release();

public:
    static RAII<OpenFile> open(std::filesystem::path path, int flags);

    int fd() const { return _fd; }
};


class MappedFile {
private:
    struct Impl {
        std::filesystem::path path;
        unsigned char* data;
        size_t length;
    } _impl;

    explicit MappedFile(Impl impl) : _impl(std::move(impl)) {}

    friend RAII<MappedFile>;
    void release();

public:
    static RAII<MappedFile> open(std::filesystem::path path);

    const std::filesystem::path& path() const {
        return _impl.path;
    }

    std::span<const unsigned char> data() const {
        return {_impl.data, _impl.length};
    }
};

#endif  // X86_KERNEL_POSIX_IO_HH
