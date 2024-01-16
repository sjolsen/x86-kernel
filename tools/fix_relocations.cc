#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <span>
#include <vector>

#include <elf.h>

#include "format.hh"
#include "posix_io.hh"


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


template <typename T>
const T* extract_header(std::span<const unsigned char> raw) {
    auto address = reinterpret_cast<uintptr_t>(raw.data());
    if (address % alignof(T) != 0)
        throw std::logic_error("invalid alignment");
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


template <typename T>
T extract_value(std::span<const unsigned char> raw, size_t offset) {
    T result;
    auto bytes = safe_subspan<const unsigned char>(raw, offset, sizeof(T));
    memcpy(&result, bytes.data(), sizeof(T));
    return result;
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

    // Associate section names with their contents. This will be necessary when
    // reading the implicit addend
    std::map<std::string_view, std::span<const unsigned char>> name_to_contents;
    for (const auto& sh : section_headers) {
        std::string_view name = safe_at(section_names, sh.sh_name);
        auto contents = extract_span<const unsigned char>(
            raw, sh.sh_offset, sh.sh_size);
        name_to_contents.insert({name, contents});
    }

    // Locate the implicit relocation sections
    for (const auto& sh : section_headers)
        if (sh.sh_type == SHT_REL) {
            // By convention section ".relNAME" contains relocation information
            // for section "NAME" (note that "NAME" itself will have a leading
            // dot).
            std::string_view name = safe_at(section_names, sh.sh_name);
            if (!name.starts_with(".rel."))
                throw std::runtime_error(
                    strcat("could not parse section name", name));
            std::string_view section_name = name.substr(4);
            std::span<const unsigned char> section_contents =
                name_to_contents.at(section_name);
            std::cout << name << " " << section_name << std::endl;

            // Construct a new RELA for each REL
            auto rels = extract_span<Elf64_Rel>(
                raw, sh.sh_offset, sh.sh_size / sizeof(Elf64_Rel));
            std::vector<Elf64_Rela> relas;
            relas.reserve(rels.size());
            for (const auto& rel : rels) {
                auto type = ELF64_R_TYPE(rel.r_info);
                switch (type) {
                    case R_X86_64_PC32: {
                        /* PC relative 32 bit signed */
                        uint32_t addend32 = le32toh(
                            extract_value<uint32_t>(
                                section_contents, rel.r_offset));
                        int64_t addend64 = static_cast<int32_t>(addend32);
                        relas.push_back({rel.r_offset, rel.r_info, addend64});
                        break;
                    }
                    case R_X86_64_32: {
                        /* Direct 32 bit zero extended */
                        uint32_t addend32 = le32toh(
                            extract_value<uint32_t>(
                                section_contents, rel.r_offset));
                        int64_t addend64 = static_cast<uint64_t>(addend32);
                        relas.push_back({rel.r_offset, rel.r_info, addend64});
                        break;
                    }
                    default:
                        auto msg = strcat("unrecognized relocation type", type);
                        throw std::runtime_error(msg);
                };
            }

            for (const auto& rela : relas)
                std::cout << rela.r_addend << std::endl;
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
