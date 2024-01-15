#include <algorithm>
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


template <typename T, size_t N>
T* safe_at(const std::span<T, N>& a, size_t i) {
    if (i >= a.size())
        throw std::out_of_range("span bound exceeded");
    return &a[i];
}


template <typename T>
const T* extract_header(std::span<const unsigned char> raw) {
    auto address = reinterpret_cast<uintptr_t>(raw.data());
    if (address % alignof(T) != 0)
        throw std::logic_error("invalid alignment");
    auto length = raw.size();
    if (raw.size() < sizeof(T))
        throw std::runtime_error("exceeded file bounds");
    return reinterpret_cast<const T*>(address);
}


template <typename T>
std::span<const T> extract_span(std::span<const unsigned char> raw,
                                size_t offset, size_t n) {
    // TODO: arithmetic overflow checks
    auto address = reinterpret_cast<uintptr_t>(raw.data()) + offset;
    if (address % alignof(T) != 0)
        throw std::runtime_error("invalid alignment");
    if (raw.size() < offset + sizeof(T) * n)
        throw std::runtime_error("exceeded file bounds");
    return std::span(reinterpret_cast<const T*>(address), n);
}


void parse_elf(std::span<const unsigned char> raw) {
    auto elf = extract_header<Elf64_Ehdr>(raw);

    // We should be looking at a relocatable x86-64 object file since we're
    // trying to change REL relocations to RELA relocations
    auto ident = std::span<const unsigned char, EI_NIDENT>{elf->e_ident};
    auto magic = ident.subspan(0, 4);
    static const unsigned char good_magic[4] = {ELFMAG0, 'E', 'L', 'F'};
    if (!std::ranges::equal(magic, good_magic))
        throw std::runtime_error("magic does not match");
    if (ident[EI_CLASS] != ELFCLASS64)
        throw std::runtime_error("not a 64-bit ELF file");
    if (ident[EI_DATA] != ELFDATA2LSB)
        throw std::runtime_error("not a little-endian ELF file");
    if (ident[EI_VERSION] != EV_CURRENT)
        throw std::runtime_error("unrecognized ELF version");
    if (ident[EI_OSABI] != ELFOSABI_SYSV)
        throw std::runtime_error("not a System V ABI ELF file");

    if (elf->e_type != ET_REL)
        throw std::runtime_error("not a relocatable ELF file");
    if (elf->e_machine != EM_X86_64)
        throw std::runtime_error("not an x86-64 ELF file");
    if (elf->e_version != EV_CURRENT)
        throw std::runtime_error("unrecognized ELF version");

    // Validate the alignment and size of the section header table
    if (elf->e_shentsize != sizeof(Elf64_Shdr))
        throw std::runtime_error("ELF section headers not sized correctly");
    auto section_headers = extract_span<Elf64_Shdr>(
        raw, elf->e_shoff, elf->e_shnum);

    // Find the section name string table
    const Elf64_Shdr* shstrtab = safe_at(section_headers, elf->e_shstrndx);
    if (shstrtab->sh_type != SHT_STRTAB)
        throw std::runtime_error(
            "section header string table has the wrong type");
    auto section_names = extract_span<const char>(
        raw, shstrtab->sh_offset, shstrtab->sh_size);
    if (!(section_names.size() > 0 && section_names.back() == '\0'))
        throw std::runtime_error(
            "section header string table is not NUL-terminated");

    // Locate the implicit relocation sections
    for (const auto& sh : section_headers)
        if (sh.sh_type == SHT_REL) {
            std::string_view name = safe_at(section_names, sh.sh_name);
            std::cout << name << std::endl;
        }
}


int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: fix_relocations in_file out_file\n";
        return EXIT_FAILURE;
    }
    RAII<MappedFile> in_file = MappedFile::open(argv[1]);
    std::filesystem::path out_file = argv[2];

    parse_elf(in_file->data());

    return EXIT_SUCCESS;
}
