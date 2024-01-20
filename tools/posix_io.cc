#include "posix_io.hh"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "format.hh"


RAII<OpenFile> OpenFile::open(std::filesystem::path path, int flags) {
    int fd = ::open(path.c_str(), flags);
    if (fd < 0) {
        auto msg = strcat("failed to open ", path);
        throw std::system_error(errno, std::system_category(), msg);
    }
    return RAII(OpenFile(fd));
}


void OpenFile::release() {
    close(_fd);
}


RAII<MappedFile> MappedFile::open(std::filesystem::path path) {
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


void MappedFile::release() {
    munmap(_impl.data, _impl.length);
}
