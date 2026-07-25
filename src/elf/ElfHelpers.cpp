#include "ElfHelpers.hpp"

#include "ElfException.hpp"

#include <fstream>

void FileWrite(std::string filePath, const std::vector<uint8_t> &content)
{
    std::ofstream file(filePath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(content.data()), content.size());
    file.close();
}

int GetSectionIndex(const std::vector<Elf64_Shdr> &shdr, const std::vector<uint8_t> &shrstrtab, std::string name)
{
    for (Elf64_Half i = 0; i < shdr.size(); i++)
    {
        int index = shdr[i].sh_name;
        std::string currName(reinterpret_cast<const char*>(shrstrtab.data() + index));
        if (name == currName)
            return i;
    }

    return -1;
}

Elf64_Shdr MakeSectionHeader(Elf64_Word name, const Section::s_ptr& section, Elf64_Off offset)
{
    Elf64_Shdr shdr = {};
    shdr.sh_name      = name;
    shdr.sh_type      = Elf64_Shdr_Type::SHT_PROGBITS;
    shdr.sh_flags     = Elf64_Shdr_Flags::SHF_ALLOC | Elf64_Shdr_Flags::SHF_WRITE;
    shdr.sh_addr      = 0;
    shdr.sh_offset    = sizeof(Elf64_Ehdr) + offset;
    shdr.sh_size      = section->data.size();
    shdr.sh_link      = Elf64_SHN::SHN_UNDEF;
    shdr.sh_info      = 0;
    shdr.sh_addralign = 4;
    shdr.sh_entsize   = 0;
    return shdr;
}

Elf64_Shdr GetShdrByIndex(const std::vector<Elf64_Shdr>& shdrt, int index)
{
    if (index < 0 || index >= (int)shdrt.size())
        return Elf64_Shdr();
    return shdrt[index];
}

Elf64_Shdr GetShdrByType(const std::vector<Elf64_Shdr>& shdrt, Elf64_Shdr_Type type)
{
    for (const auto& shdr : shdrt)
    {
        if (shdr.sh_type == type)
            return shdr;
    }
    return Elf64_Shdr();
}

Elf64_Ehdr InitEhdr()
{
    Elf64_Ehdr ehdr = {};

    unsigned char magic[4] = {0x7F, 'E', 'L', 'F'};
    std::memcpy(ehdr.e_ident, magic, 4 * sizeof(unsigned char));
    //64bit objekti
    ehdr.e_ident[Elf64_IdentIndex::EI_CLASS] = Elf64_Class::ELFCLASS64;
    // little endian
    ehdr.e_ident[Elf64_IdentIndex::EI_DATA] = Elf64_DataEncoding::ELFDATA2LSB;
    // elf verzija
    ehdr.e_ident[Elf64_IdentIndex::EI_VERSION] = 1; // EI_CURRENT
    // linux abi
    ehdr.e_ident[Elf64_IdentIndex::EI_OSABI] = Elf64_OSABI::ELFOSABI_LINUX;
    // ABI Version
    ehdr.e_ident[Elf64_IdentIndex::EI_ABIVERSION] = 0;
    // postavi padding
    std::memset(ehdr.e_ident + EI_PAD, 0, 7);

    return ehdr;
}

std::string ReadString(const std::vector<uint8_t> &src, int start)
{
    if (start < 0 || start >= (int)src.size())
        return {};

    std::string ret;
    int i = start;
    while (i < (int)src.size() && src[i] != '\0')
        ret += src[i++];

    return ret;
}

int GetSectionIndex(const SectionTable &sections, const std::string& name)
{
    // znamo dve stvari: prve tri sekcije koje se pojavljuju ce biti .symtab, .strtab i .shstrtab
    // nakon toga stavljamo sve sekcije redom kako nailazimo na njih, tako da ce redni broj sekcije u tabeli
    // sekcijskih zaglavlja biti 3 + index u trenutnom poretku
    for (int i = 0; i < sections.size(); i++)
    {
        if (sections[i]->name == name)
        {
            return i + 3;
        }
    }

    return -1;
}

void ValidateEhdr(const Elf64_Ehdr& ehdr, size_t fileSize)
{
    // magic + class + version
    if (ehdr.e_ident[EI_MAG0] != 0x7F ||
        ehdr.e_ident[EI_MAG1] != 'E'  ||
        ehdr.e_ident[EI_MAG2] != 'L'  ||
        ehdr.e_ident[EI_MAG3] != 'F'  ||
        ehdr.e_ident[EI_VERSION] != 1)
        throw ElfException(ELF_EHDR_CORRUPTED);

    // section header table
    if (ehdr.e_shoff == 0 || ehdr.e_shoff >= fileSize)
        throw ElfException(ELF_SHDRT_NOT_FOUND);

    if (ehdr.e_shnum == 0 || ehdr.e_shentsize != sizeof(Elf64_Shdr))
        throw ElfException(ELF_SHDRT_CORRUPTED);

    if (ehdr.e_shoff + ehdr.e_shnum * sizeof(Elf64_Shdr) > fileSize)
        throw ElfException(ELF_SHDRT_CORRUPTED);
}

