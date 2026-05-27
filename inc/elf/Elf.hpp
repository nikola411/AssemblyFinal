#ifndef ELF_HPP
#define ELF_HPP

#include "ElfDataTypes.hpp"

#include "Utility.hpp"

#include <inttypes.h>
#include <vector>
#include <string>

class Elf
{
public:
    Elf(){};
    ~Elf(){};

    void Load(Elf64_EType type, const SymbolTable& symTable, const SectionTable& sections, const RelocationTable& relocations);

    // osnovne metode za objekte najviseg nivoa slozenosti
    Elf64_Ehdr GetElfHeader() const;
    bool SetElfHeader(Elf64_Ehdr ehdr);

    std::vector<Elf64_Phdr> GetProgramTableHeader() const;
    bool SetProgramHeaderTable(const std::vector<Elf64_Phdr>& phdr);

    std::vector<Elf64_Shdr> GetSectionTableHeader() const;
    bool SetSectionHeaderTable(const std::vector<Elf64_Shdr>& shdr);

    std::vector<uint8_t> GetSectionContent(Elf64_Shdr shdr) const;

    // kraj osnovnih metoda

    // specijalne metode

    // std::vector<Elf64_Sym> ReadSymbolTable() const;
    // std::vector<Elf64_
    // std::vector<Elf64_Rela> ReadRelocationsTable() const;

    void WriteToFile(std::string filePath);
    void HexDump();

private:
    std::vector<uint8_t> content;

    Elf64_Ehdr InitEhdr();

    template<typename T>
    std::vector<T> ReadTable(Elf64_Off offset, Elf64_Half count) const
    {
        std::vector<T> result(count);
        std::memcpy(result.data(), content.data() + offset, count * sizeof(T));
        return result;
    }

    template<typename T>
    void WriteTable(Elf64_Off offset, const std::vector<T>& table)
    {
        std::memcpy(content.data() + offset, table.data(), table.size() * sizeof(T));
    }

    template<typename T>
    T ReadObject(Elf64_Off offset) const
    {
        T result;
        std::memcpy(&result, content.data() + offset, sizeof(T));

        return result;
    }

    template<typename T>
    void WriteObject(Elf64_Off offset, const T& object)
    {
        std::memcpy(content.data() + offset, &object, sizeof(T));
    }
};

#endif