#ifndef X86_KERNEL_ELF_HH
#define X86_KERNEL_ELF_HH

#include <iostream>
#include <span>


class Section;


/* We only represent relocatable x86-64 object files since we're trying to
 * change REL relocations to RELA relocations. Simplifying assumptions include:
 *
 *   - Fixed values for most ELF header fields
 *   - All fields are little-endian
 *   - No program headers
 *   - No section groups
 *   - No dynamic section
 *   - No notes, interpreter, or version information
 */
class ObjectFile {
private:
    Elf64_Addr _entry;
    std::set<Section> _sections;
    uint32_t _flags;

    explicit ObjectFile(Elf64_Addr entry,
                        std::set<Section> sections,
                        uint32_t flags)
        : _entry(entry),
          _sections(std::move(sections)),
          _flags(flags) {
    }

public:
    static ObjectFile parse(std::span<const unsigned char> raw);

    void serialize(std::ostream& os) const;
};


#endif  // X86_KERNEL_ELF_HH
