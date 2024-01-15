#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <span>

#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "raii.hh"


template <typename... T>
std::string strcat(const T&... args) {
    std::stringstream ss;
    (ss << ... << args);
    return std::move(ss).str();
}


class OpenFile {
private:
    int _fd;

    explicit OpenFile(int fd) : _fd(fd) {}

    friend RAII<OpenFile>;
    void release() { close(_fd); }

public:
    static RAII<OpenFile> open(std::filesystem::path path, int flags) {
        int fd = ::open(path.c_str(), flags);
        if (fd < 0) {
            auto msg = strcat("failed to open ", path);
            throw std::system_error(errno, std::system_category(), msg);
        }
        return RAII(OpenFile(fd));
    }

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
    void release() { munmap(_impl.data, _impl.length); }

public:
    static RAII<MappedFile> open(std::filesystem::path path) {
        RAII<OpenFile> file = OpenFile::open(path, O_RDONLY);

        // mmap requires an explicit length argument
        struct stat statbuf;
        int rv = fstat(file->fd(), &statbuf);
        if (rv != 0) {
            auto msg = strcat("failed to stat ", path);
            throw std::system_error(errno, std::system_category(), msg);
        }
        auto length = static_cast<size_t>(statbuf.st_size);

        // Map the file into memory
        void* ret = mmap(nullptr, statbuf.st_size, PROT_READ, MAP_PRIVATE,
                         file->fd(), 0);
        if (ret == MAP_FAILED) {
            auto msg = strcat("failed to mmap ", path);
            throw std::system_error(errno, std::system_category(), msg);
        }
        auto data = static_cast<unsigned char*>(ret);

        return RAII(MappedFile{Impl{std::move(path), data, length}});
    }

    const std::filesystem::path& path() const {
        return _impl.path;
    }

    std::span<const unsigned char> data() const {
        return {_impl.data, _impl.length};
    }
};


int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: fix_relocations in_file out_file\n";
        return EXIT_FAILURE;
    }
    RAII<MappedFile> in_file = MappedFile::open(argv[1]);
    std::filesystem::path out_file = argv[2];

    std::cout << in_file->path() << std::endl;
    std::cout << in_file->data().size() << std::endl;

    return EXIT_SUCCESS;
}
