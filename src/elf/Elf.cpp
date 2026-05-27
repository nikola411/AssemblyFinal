#include "Elf.hpp"

#include <vector>
#include <cstring>
#include <fstream>

void Elf::Load(Elf64_EType type, const SymbolTable &symTable, const SectionTable &sections, const RelocationTable &relocations)
{
    // postavi elf header
    Elf64_Ehdr ehdr = InitEhdr();

    std::vector<Elf64_Shdr> elfShdr;
    std::vector<Elf64_Phdr> elfPhdr;

    // write sections
    int sectionsSizeInBytes = 0;
    for (auto& section: sections)
    {
        sectionsSizeInBytes += section->data.size();
    }

    std::vector<uint8_t> elfSections = std::vector<uint8_t>(sectionsSizeInBytes);
    std::vector<uint8_t> elfShstrtab;

    Elf64_Off sectionOffset = 0;
    for (auto section: sections)
    {
        Elf64_Shdr secEntr;
        secEntr.sh_name = elfShstrtab.size();
        elfShstrtab.insert(elfShstrtab.end(), section->name.begin(), section->name.end());
        elfShstrtab.push_back('\0');

        secEntr.sh_type = Elf64_Shdr_Type::SHT_PROGBITS;
        secEntr.sh_flags |= Elf64_Shdr_Flags::SHF_ALLOC | Elf64_Shdr_Flags::SHF_WRITE;
        secEntr.sh_addr = 0; // za .o fajlove, razmisli za exec sta ces da radis
        secEntr.sh_offset = sizeof(ehdr) + /* velicina program header tablee*/ + sectionOffset;
        secEntr.sh_size = section->data.size();
        secEntr.sh_link = Elf64_SHN::SHN_UNDEF;
        secEntr.sh_info = 0; // drugi tip sekcije, nije specijalna sekcija
        secEntr.sh_addralign = 4;
        secEntr.sh_entsize = 0;

        elfShdr.push_back(secEntr);
        elfSections.insert(elfSections.end(), section->data.begin(), section->data.end());
    }

    // elfSections.push_back(elfShstrtab);

    // alociraj prostor
    int size = sizeof(Elf64_Ehdr) + elfPhdr.size() * sizeof(Elf64_Phdr) + elfSections.size() + elfShdr.size() * sizeof(Elf64_Shdr);
    content = std::vector<uint8_t>(size, 0);

    // upisi podatke
    SetElfHeader(ehdr);
    SetProgramHeaderTable(elfPhdr);
    //SetSections(elfSections);
    SetSectionHeaderTable(elfShdr);
}

Elf64_Ehdr Elf::GetElfHeader() const
{
    return ReadObject<Elf64_Ehdr>(0);
}

bool Elf::SetElfHeader(Elf64_Ehdr ehdr)
{
    WriteObject<Elf64_Ehdr>(0, ehdr);
    return true;
}

std::vector<Elf64_Phdr> Elf::GetProgramTableHeader() const
{
    Elf64_Ehdr ehdr = GetElfHeader();

    Elf64_Off start = ehdr.e_phoff;
    Elf64_Off size = ehdr.e_phnum * ehdr.e_phentsize;

    return ReadTable<Elf64_Phdr>(start, size);
}

bool Elf::SetProgramHeaderTable(const std::vector<Elf64_Phdr>& phdr)
{
    Elf64_Ehdr ehdr = GetElfHeader();

    Elf64_Off start = ehdr.e_phoff;
    WriteTable<Elf64_Phdr>(start, phdr);

    return true;
}

std::vector<Elf64_Shdr> Elf::GetSectionTableHeader() const
{
    Elf64_Ehdr ehdr = GetElfHeader();

    Elf64_Off start = ehdr.e_shoff;
    Elf64_Off size = ehdr.e_shnum * ehdr.e_shentsize;

    return ReadTable<Elf64_Shdr>(start, size);
}

bool Elf::SetSectionHeaderTable(const std::vector<Elf64_Shdr>& shdr)
{
    Elf64_Ehdr ehdr = GetElfHeader();

    Elf64_Off start = ehdr.e_shoff;
    WriteTable<Elf64_Shdr>(start, shdr);

    return true;
}

std::vector<uint8_t> Elf::GetSectionContent(Elf64_Shdr shdr) const
{
    return std::vector<uint8_t>();
}

void Elf::WriteToFile(std::string filePath)
{
    std::ofstream file(filePath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(content.data()), content.size());
    file.close();
}

void Elf::HexDump()
{
}

Elf64_Ehdr Elf::InitEhdr()
{
    Elf64_Ehdr ehdr;

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
