#include "elf.hh"

#include <algorithm>

#include "format.hh"
#include "span.hh"


namespace {

template <typename T>
std::span<const T> extract_section(std::span<const unsigned char> raw,
                                   const Elf64_Shdr& sh) {
    return extract_span<T>(raw, sh.sh_offset, Bytes{sh.sh_size});
}

}  // namespace


ObjectFile parse_elf(std::span<const unsigned char> raw) {
    ObjectFile obj;
    auto elf = extract_one<Elf64_Ehdr>(raw, 0);

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

    obj.entry = elf->e_entry;
    obj.flags = elf->e_flags;

    // Validate the alignment and size of the section header table
    if (elf->e_shentsize != sizeof(Elf64_Shdr))
        throw std::runtime_error("ELF section headers not sized correctly");
    auto section_headers = extract_span<Elf64_Shdr>(
        raw, elf->e_shoff, Count{elf->e_shnum});

    // Find the section name string table
    const Elf64_Shdr* shstrtab = safe_at(section_headers, elf->e_shstrndx);
    if (shstrtab->sh_type != SHT_STRTAB)
        throw std::runtime_error(
            "section header string table has the wrong type");
    auto section_names = extract_section<char>(raw, *shstrtab);
    if (!(section_names.size() > 0 && section_names.back() == '\0'))
        throw std::runtime_error(
            "section header string table is not NUL-terminated");

    // Parse the sections
    for (size_t i = 0; i < section_headers.size(); ++i) {
        const Elf64_Shdr& sh = section_headers[i];
        Section& s = obj.sections.emplace_back();
        s.name = safe_at(section_names, sh.sh_name);
        s.flags = sh.sh_flags;
        s.addr = sh.sh_addr;
        // s.data defaults to Null{}
        s.link = sh.sh_link;
        s.info = sh.sh_info;
        s.align = sh.sh_addralign;

        switch (sh.sh_type) {
        case SHT_NULL:
            break;
        case SHT_PROGBITS:
            s.data.emplace<Progbits>(extract_section<unsigned char>(raw, sh));
            break;
        case SHT_NOBITS:
            s.data.emplace<Nobits>();
            break;
        case SHT_REL:
            s.data.emplace<Rel>(extract_section<Elf64_Rel>(raw, sh));
            break;
        case SHT_SYMTAB:
            s.data.emplace<Symtab>(extract_section<Elf64_Sym>(raw, sh));
            break;
        case SHT_STRTAB:
            if (i == elf->e_shstrndx)
                s.data.emplace<ShStrtab>();
            else
                s.data.emplace<Strtab>(extract_section<char>(raw, sh));
            break;
        default:
            auto msg = strcat("unsupported section type", sh.sh_type);
            throw std::runtime_error(msg);
        }
    }

    return obj;
}
