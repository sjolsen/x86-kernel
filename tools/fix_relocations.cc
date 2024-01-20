#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "elf.hh"
#include "format.hh"
#include "posix_io.hh"
#include "span.hh"


std::vector<unsigned char> read_file(std::filesystem::path path) {
    // Quick and dirty solution to prevent the input file contents from getting
    // unmapped when we truncate the output file, if they're the same file
    RAII<MappedFile> file = MappedFile::open(path);
    std::vector<unsigned char> result(file->data().size());
    std::ranges::copy(file->data(), std::begin(result));
    return result;
}


int64_t resolve_addend(const Elf64_Rel& rel,
                       const std::span<const unsigned char>& data) {
    auto type = ELF64_R_TYPE(rel.r_info);
    switch (type) {
    case R_X86_64_PC32: {
        /* PC relative 32 bit signed */
        uint32_t addend32 = le32toh(
            extract_unaligned<uint32_t>(data, rel.r_offset));
        int64_t addend64 = static_cast<int32_t>(addend32);
        return addend64;
    }
    case R_X86_64_32: {
        /* Direct 32 bit zero extended */
        uint32_t addend32 = le32toh(
            extract_unaligned<uint32_t>(data, rel.r_offset));
        int64_t addend64 = static_cast<uint64_t>(addend32);
        return addend64;
    }
    default:
        auto msg = strcat("unrecognized relocation type", type);
        throw std::runtime_error(msg);
    }
}


std::string rename_section(std::string_view name) {
    std::string result{name};
    if (!name.starts_with(".rel."))
        throw std::runtime_error(strcat("couldn't parse section name: ", name));
    result.insert(4, "a");
    return result;
}


void fix_relocations(ObjectFile& obj) {
    for (auto& section : obj.sections) {
        if (const Rel* data = std::get_if<Rel>(&section.data)) {
            const Section& target = obj.sections[section.info];
            const Progbits& progbits = std::get<Progbits>(target.data);

            std::vector<Elf64_Rela> relas;
            for (Elf64_Rel rel : data->rels) {
                int64_t addend = resolve_addend(rel, progbits.bytes);
                relas.push_back({rel.r_offset, rel.r_info, addend});
            }

            section.name = rename_section(section.name);
            section.data.emplace<Rela>(std::move(relas));
        }
    }
}


int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: fix_relocations in_file out_file\n";
        return EXIT_FAILURE;
    }
    std::vector<unsigned char> in_file = read_file(argv[1]);
    std::ofstream out_file(argv[2], std::ios::binary | std::ios::trunc);

    ObjectFile obj = parse_elf(std::span(in_file));
    fix_relocations(obj);
    write_elf(out_file, obj);

    return EXIT_SUCCESS;
}
