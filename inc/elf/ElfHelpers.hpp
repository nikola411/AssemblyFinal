#ifndef ELF_HELPERS_HPP
#define ELF_HELPERS_HPP

#include "ElfDataTypes.hpp"
#include "Section.hpp"
#include "Relocation.hpp"

#include <cstring>
#include <vector>
#include <string>

void FileWrite(std::string filePath, const std::vector<uint8_t>& content);

int GetSectionIndex(const std::vector<Elf64_Shdr>& shdr, const std::vector<uint8_t>& shrstrtab, std::string name);
Elf64_Shdr MakeSectionHeader(Elf64_Word name, const Section::s_ptr& section, Elf64_Off offset);

Elf64_Shdr GetShdrByIndex(const std::vector<Elf64_Shdr>& shdrt, int index);
Elf64_Shdr GetShdrByType(const std::vector<Elf64_Shdr>& shdrt, Elf64_Shdr_Type type);

Elf64_Ehdr InitEhdr();

std::string ReadString(const std::vector<uint8_t>& src, int start);

int GetSectionIndex(const SectionTable& sections, const std::string& name);

/*
Funkcije za proveru tacnosti elf fajla
*/
void ValidateEhdr(const Elf64_Ehdr& ehdr, size_t fileSize);
namespace ElfIO
{
    /*
    JAKO BITNO: svaki write tretira ulazni dst kao strukturu koja moze da poraste
    */
    template<typename T>
    std::vector<T> ReadTable(Elf64_Off offset, Elf64_Half count, const std::vector<uint8_t>& src)
    {
        std::vector<T> result(count);
        size_t byteCount = (size_t)count * sizeof(T);

        if (offset + byteCount > src.size())
            return result;

        std::memcpy(result.data(), src.data() + offset, byteCount);
        return result;
    }

    template<typename T>
    void WriteTable(Elf64_Off offset, std::vector<uint8_t>& dst, const std::vector<T>& table)
    {
        size_t byteCnt = table.size() * sizeof(T);
        if (dst.size() < offset + byteCnt)
            dst.resize(offset + byteCnt);

        std::memcpy(dst.data() + offset, table.data(), byteCnt);
    }

    template<typename T>
    T ReadObject(Elf64_Off offset, const std::vector<uint8_t>& src)
    {
        T result = {};
        if (offset + sizeof(T) > src.size())
            return result;

        std::memcpy(&result, src.data() + offset, sizeof(T));

        return result;
    }

    template<typename T>
    void WriteObject(Elf64_Off offset, std::vector<uint8_t>& dst, const T& src)
    {
        size_t byteCnt = sizeof(T);
        if (dst.size() < offset + byteCnt)
            dst.resize(offset + byteCnt);

        std::memcpy(dst.data() + offset, &src, sizeof(T));
    }

    template<typename T>
    bool IsDefaultValue(T src)
    {
        T defaultValue = {};
        return std::memcmp(&src, &defaultValue, sizeof(T)) == 0;
    }
};

#endif