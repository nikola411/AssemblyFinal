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

    void LoadLinkable(const SymbolTable& symTable, const SectionTable& sections);
    void LoadBinary(const std::vector<uint8_t>& content);
    void UnloadLinkable(SymbolTable& symTable, SectionTable& sections);

    void LoadSymbolTable(std::vector<Elf64_Shdr>& shdrt, std::vector<uint8_t>& shstrt);
    void LoadSectionData(std::vector<Elf64_Shdr> &shdrt, int startOffset, int shstrtOffset);

    // specialized
    Elf64_Ehdr GetElfHeader() const;
    std::vector<Elf64_Phdr> GetProgramHeaderTable() const;
    std::vector<Elf64_Shdr> GetSectionHeaderTable() const;
    std::vector<Elf64_Sym> GetSymbolTable() const;
    std::string GetSymbolName(const Elf64_Sym& sym) const;
    int GetElfSize() const;

    bool SetElfHeader(Elf64_Ehdr ehdr);
    bool SetProgramHeaderTable(const std::vector<Elf64_Phdr>& phdr);
    bool SetSectionHeaderTable(const std::vector<Elf64_Shdr>& shdr);

    // generic
    std::vector<uint8_t> GetSectionContent(int shdrtOffset) const;

    bool UpdateSectionContent(int shdrtOffset, const std::vector<uint8_t>& content);

    void HexDump();


private:
    // raw file content
    std::vector<uint8_t> content;

    // sadrzaj fajla na najvisem nivou apstrakcije
    SymbolTable symTable;
    SectionTable sections;
    RelocationTable relocations;
};

#endif