#include "elf.hh"

#include <algorithm>
#include <bit>

#include "format.hh"
#include "span.hh"


namespace {

template <typename T>
std::span<const T> extract_section(std::span<const unsigned char> raw,
                                   const Elf64_Shdr& sh) {
    return extract_span<T>(raw, sh.sh_offset, Bytes{sh.sh_size});
}


static const unsigned char ELF_MAGIC[4] = {ELFMAG0, 'E', 'L', 'F'};

}  // namespace


ObjectFile parse_elf(std::span<const unsigned char> raw) {
    ObjectFile obj;
    auto elf = extract_one<Elf64_Ehdr>(raw, 0);

    // We should be looking at a relocatable x86-64 object file since we're
    // trying to change REL relocations to RELA relocations
    auto ident = std::span(elf->e_ident);
    auto magic = ident.subspan(0, 4);
    if (!std::ranges::equal(magic, ELF_MAGIC))
        throw std::runtime_error("magic does not match");
    if (ident[EI_CLASS] != ELFCLASS64)
        throw std::runtime_error("not a 64-bit ELF file");
    if (ident[EI_DATA] != ELFDATA2LSB)
        throw std::runtime_error("not a little-endian ELF file");
    if (ident[EI_VERSION] != EV_CURRENT)
        throw std::runtime_error("unrecognized ELF version");
    if (ident[EI_OSABI] != ELFOSABI_SYSV)
        throw std::runtime_error("not a System V ABI ELF file");
    if (ident[EI_ABIVERSION] != 0)
        throw std::runtime_error("invalid ABI version");

    if (elf->e_type != ET_REL)
        throw std::runtime_error("not a relocatable ELF file");
    if (elf->e_machine != EM_X86_64)
        throw std::runtime_error("not an x86-64 ELF file");
    if (elf->e_version != EV_CURRENT)
        throw std::runtime_error("unrecognized ELF version");

    obj.entry = elf->e_entry;
    obj.flags = elf->e_flags;

    if (elf->e_phnum != 0)
        throw std::runtime_error("cannot handle PHDRs");

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
            s.data.emplace<Nobits>(sh.sh_size);
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


namespace {

class StringTable {
private:
    std::vector<char> _data = {'\0'};

public:
    size_t store(std::string_view s) {
        size_t result = _data.size();
        std::ranges::copy(s, std::back_inserter(_data));
        _data.push_back('\0');
        return result;
    }

    std::span<const char> get() const {
        return _data;
    }
};


class Allocator {
public:
    struct Entry {
        std::span<const unsigned char> data;
        size_t vmsize;
        size_t offset;
        size_t align;
        size_t entsize;
    };

private:
    std::vector<Entry> _entries;
    size_t _next = 0;

public:
    Entry allocate(std::span<const unsigned char> data,
                   size_t vmsize, size_t align, size_t entsize = 0) {
        switch (std::popcount(align)) {
        case 0:
            break;
        case 1:
            _next = (_next + (align - 1)) & ~(align - 1);
            break;
        default:
            _next = align * ((_next + (align - 1)) / align);
        }
        size_t offset = _next;
        _next += data.size();
        _entries.emplace_back(data, vmsize, offset, align, entsize);
        return _entries.back();
    }

    template <typename T>
    Entry allocate(const T* data) {
        auto p = reinterpret_cast<const unsigned char*>(data);
        size_t len = sizeof(T);
        return allocate({p, len}, len, alignof(T), sizeof(T));
    }

    template <typename T>
    Entry allocate(std::span<T> data) {
        auto p = reinterpret_cast<const unsigned char*>(data.data());
        size_t len = data.size() * sizeof(T);
        return allocate({p, len}, len, alignof(T), sizeof(T));
    }

    std::span<const Entry> entries() const { return _entries; }
};

template <typename... T>
struct match : T... { using T::operator()...; };

}  // namespace


void write_elf(std::ostream& os, const ObjectFile& obj) {
    Elf64_Ehdr elf{};
    std::vector<Elf64_Shdr> shdrs(obj.sections.size());

    auto ident = std::span(elf.e_ident);
    std::ranges::copy(ELF_MAGIC, std::begin(ident));
    ident[EI_CLASS] = ELFCLASS64;
    ident[EI_DATA] = ELFDATA2LSB;
    ident[EI_VERSION] = EV_CURRENT;
    ident[EI_OSABI] = ELFOSABI_SYSV;
    ident[EI_ABIVERSION] = 0;
    ident[EI_NIDENT] = sizeof(elf.e_ident);
    elf.e_type = ET_REL;
    elf.e_machine = EM_X86_64;
    elf.e_version = EV_CURRENT;
    elf.e_entry = obj.entry;
    elf.e_phoff = 0;
    // e_shoff initialized below
    elf.e_flags = obj.flags;
    elf.e_ehsize = sizeof(elf);
    elf.e_phentsize = 0;
    elf.e_phnum = 0;
    elf.e_shentsize = sizeof(Elf64_Shdr);
    elf.e_shnum = shdrs.size();
    // e_shstrndx initialized below

    // Recompute the contents of .shstrtab
    StringTable shstrtab;
    std::vector<uint32_t> sh_names;
    for (const Section& section : obj.sections) {
        if (std::holds_alternative<Null>(section.data))
            sh_names.push_back(0);
        else
            sh_names.push_back(shstrtab.store(section.name));
    }

    // Compute file offsets
    Allocator a;
    a.allocate(&elf);
    elf.e_shoff = a.allocate(std::span(shdrs)).offset;
    for (size_t i = 0; i < obj.sections.size(); ++i) {
        const Section& section = obj.sections[i];
        Elf64_Shdr& sh = shdrs[i];

        sh.sh_name = sh_names[i];
        sh.sh_flags = section.flags;
        sh.sh_addr = section.addr;
        sh.sh_link = section.link;
        sh.sh_info = section.info;

        auto allocate = [&](auto&&... args) {
            Allocator::Entry entry = a.allocate(
                std::forward<decltype(args)>(args)...);
            sh.sh_offset = entry.offset;
            sh.sh_size = entry.vmsize;
            sh.sh_addralign = entry.align;
            sh.sh_entsize = entry.entsize;
        };

        match visitor {
            [&](const Null&) {
                sh.sh_type = SHT_NULL;
                sh.sh_offset = 0;
                sh.sh_size = 0;
                sh.sh_addralign = 0;
                sh.sh_entsize = 0;
            },
            [&](const Progbits& data) {
                sh.sh_type = SHT_PROGBITS;
                allocate(data.bytes, data.bytes.size(), section.align);
            },
            [&](const Nobits& data) {
                sh.sh_type = SHT_NOBITS;
                allocate(std::span<const unsigned char>{},
                         data.size, section.align);
            },
            [&](const Rel& data) {
                sh.sh_type = SHT_REL;
                allocate(data.rels);
            },
            [&](const Rela& data) {
                sh.sh_type = SHT_RELA;
                allocate(std::span(data.relas));
            },
            [&](const Symtab& data) {
                sh.sh_type = SHT_SYMTAB;
                allocate(data.symbols);
            },
            [&](const Strtab& data) {
                sh.sh_type = SHT_STRTAB;
                allocate(data.strings);
            },
            [&](const ShStrtab&) {
                sh.sh_type = SHT_STRTAB;
                allocate(shstrtab.get());
                elf.e_shstrndx = i;
            },
        };
        std::visit(visitor, section.data);
    }

    // Write out the bytes
    os.exceptions(os.exceptions() | std::ostream::failbit);
    for (const Allocator::Entry& entry : a.entries()) {
        os.seekp(entry.offset);
        os.write(reinterpret_cast<const char*>(entry.data.data()),
                 entry.data.size());
    }
}
