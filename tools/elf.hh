#ifndef X86_KERNEL_ELF_HH
#define X86_KERNEL_ELF_HH

#include <iostream>
#include <span>
#include <variant>
#include <vector>

#include <elf.h>


/* We only represent relocatable x86-64 object files since we're only trying to
 * change REL relocations to RELA relocations. Simplifying assumptions include:
 *
 *   - Fixed values for most ELF header fields
 *   - All fields are little-endian
 *   - No program headers
 *   - No section groups
 *   - No dynamic section
 *   - No notes, interpreter, or version information
 */

struct Null {};
struct Progbits { std::span<const unsigned char> bytes; };
struct Nobits { size_t size; };
struct Rel { std::span<const Elf64_Rel> rels; };
struct Rela { std::vector<Elf64_Rela> relas; };
struct Symtab { std::span<const Elf64_Sym> symbols; };
struct Strtab { std::span<const char> strings; };
struct ShStrtab {};

using SectionData = std::variant<
    Null, Progbits, Nobits, Rel, Rela, Symtab, Strtab, ShStrtab>;


struct Section {
    std::string name;
    uint64_t flags;
    Elf64_Addr addr;
    SectionData data;
    uint32_t link;
    uint32_t info;
    uint64_t align;
};


struct ObjectFile {
    Elf64_Addr entry;
    std::vector<Section> sections;
    uint32_t flags;
};


ObjectFile parse_elf(std::span<const unsigned char> raw);
void write_elf(std::ostream& os, const ObjectFile& obj);

#endif  // X86_KERNEL_ELF_HH
