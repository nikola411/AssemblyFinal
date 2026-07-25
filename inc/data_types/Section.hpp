#ifndef SECTION_HPP
#define SECTION_HPP

#include "ParserTypes.hpp"
#include "Relocation.hpp"

#include <string>
#include <inttypes.h>
#include <vector>
#include <memory>

struct Section
{
    std::string name;
    std::vector<BYTE> data;
    uint32_t locationCounter = 0;

    void AppendData(const std::vector<BYTE>& data);
    void WriteData(const uint32_t& offset, const std::vector<BYTE>& data);
    void WriteInstructionDisplacement(const uint32_t& offset, const uint16_t& toWrite);

    std::vector<BYTE> literalPool;

    uint32_t ReadPoolEntry(const uint32_t& entry);
    ADDRESS InsertLiteralInPool(uint32_t value);
    ADDRESS IsLiteralPresentInPool(const uint32_t value) const;
    void WritePoolEntry(const uint32_t& value, const uint32_t& entry);

    static uint32_t AddressToPoolEntry(ADDRESS address);
    static ADDRESS PoolEntryToAddress(uint32_t entry);

    RelocationTable sectionRelocations;
    RelocationTable poolRelocations;

    void AddSectionRelocation(const eRelocationType& type, const std::string& name, int offset);
    void AddPoolRelocation(const eRelocationType& type, const std::string& name, int offset);

    void UpdateSectionRelocationsOffsets(int offset);
    void UpdatePoolRelocationsOffsets(int offset);

    uint32_t startAddress = 0;

    typedef std::shared_ptr<Section> s_ptr;

    static Section::s_ptr MergeSections(Section::s_ptr& first, Section::s_ptr& second);
};
typedef std::vector<Section::s_ptr> SectionTable;

std::string SectionTableToString(const SectionTable& table);

#endif

