#ifndef ELF_HPP
#define ELF_HPP

#include "ElfDataTypes.hpp"

#include "Utility.hpp"
#include "Symbol.hpp"
#include "Section.hpp"

#include <inttypes.h>
#include <cstring>
#include <vector>
#include <string>

class Elf
{
public:
    Elf(){};
    ~Elf(){};

    size_t WriteSymtabSection(std::vector<Elf64_Shdr>& shdrt);
    size_t WriteProgramSections(std::vector<Elf64_Shdr> &shdrt, int startOffset, int shstrtOffset);
    size_t WriteStrtabSection(std::vector<Elf64_Shdr>& shdrt);
    size_t WriteShstrtabSection(std::vector<Elf64_Shdr>& shdrt);
    size_t WriteShdrtSection(const std::vector<Elf64_Shdr>& shdrt, int offset);

    // specialized
    Elf64_Ehdr GetElfHeader() const;
    std::vector<Elf64_Phdr> GetProgramHeaderTable() const;
    std::vector<Elf64_Shdr> GetSectionHeaderTable() const;
    std::vector<Elf64_Sym> GetSymbolTable() const;
    std::vector<uint8_t> GetStringTable() const;
    std::string GetSymbolName(const Elf64_Sym& sym) const;
    Elf64_Sym GetSymbolByName(const std::string& name) const;
    uint32_t GetSymbolIndexInSymbolTable(const Elf64_Sym& sym) const;
    Elf64_Sym GetSymbolByIndex(const uint32_t& index) const;
    
    bool SetElfHeader(Elf64_Ehdr ehdr);
    bool SetProgramHeaderTable(const std::vector<Elf64_Phdr>& phdr);
    bool SetSectionHeaderTable(const std::vector<Elf64_Shdr>& shdr);

    // generic
    std::vector<uint8_t> GetSectionContent(int shdrtIndex) const;
    bool UpdateSectionContent(int shdrtIndex, const std::vector<uint8_t>& content);

    // conversions
    static Elf64_Rela ConvertRelocationToRela(const Relocation& rel, const Elf& elf);
    static Relocation ConvertRelaToRelocation(const Elf64_Rela& rel, const Elf& elf);
    
    // output
    void HexDump();

    // loaders
    void LoadLinkable(const SymbolTable& symTable, const SectionTable& sections);
    void LoadBinary(const std::vector<uint8_t>& content);
    void UnloadLinkable(SymbolTable& symTable, SectionTable& sections);

//private:
    // raw file content
    std::vector<uint8_t> content;

    // sadrzaj fajla na najvisem nivou apstrakcije
    SymbolTable symTable;
    SectionTable sections;
    RelocationTable relocations;
};

#endif