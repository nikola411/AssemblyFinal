#include "Section.hpp"

#include "Utility.hpp"
#include <algorithm>


void Section::AppendData(const std::vector<BYTE>& inData)
{
    data.insert(data.end(), inData.begin(), inData.end());
    locationCounter += inData.size();
}

// this function will throw an exception only if there is not enough place to write the given data to the section
// basically this is just a wrapper for std::vector::insert
void Section::WriteData(const uint32_t& offset, const std::vector<BYTE>& inData)
{
    if (data.size() < offset + inData.size())
    {
        throw std::exception();
    }

    for (auto i = offset; i < inData.size() + offset; ++i)
        data[i] = inData[i - offset];
}

void Section::WriteInstructionDisplacement(const uint32_t& offset, const uint16_t& toWrite)
{
    if (offset + 1 > data.size())
        return;

    //data[offset + 1] |=  ((toWrite >> 8) & 0xF) << 4;
    data[offset] = toWrite & 0xFF;
    data[offset + 1] |= (toWrite >> 8);
}

uint32_t Section::ReadPoolEntry(const uint32_t &entry)
{
    uint32_t start = PoolEntryToAddress(entry);
    uint32_t value = 0;
    for (int i = 0; i < 4; i++)
        value |= (uint32_t)literalPool[start + i] << (i * 8);

    return value;
}

ADDRESS Section::InsertLiteralInPool(uint32_t value)
{
    auto bytes = IntToByteArray(value);
    literalPool.insert(literalPool.end(), bytes.begin(), bytes.end());

    return literalPool.size() / 4 - 1;
}

uint32_t Section::AddressToPoolEntry(ADDRESS address)
{
    return address / 4;
}

ADDRESS Section::PoolEntryToAddress(uint32_t entry)
{
    return entry * 4;
}

void Section::AddSectionRelocation(const eRelocationType &type, const std::string &name, int offset, int32_t addend)
{
    Relocation::s_ptr relocation = std::make_shared<Relocation>();

    relocation->type = type;
    relocation->offset = offset;
    relocation->symbolName = name;
    relocation->addend = addend;

    sectionRelocations.push_back(relocation);
}

void Section::AddPoolRelocation(const eRelocationType &type, const std::string &name, int offset)
{
    Relocation::s_ptr relocation = std::make_shared<Relocation>();

    relocation->type = type;
    relocation->offset = offset;
    relocation->symbolName = name;

    poolRelocations.push_back(relocation);
}

void Section::UpdateSectionRelocationsOffsets(int offset)
{
    for (auto rel : sectionRelocations)
    {
        rel->offset += offset;
    }
}

void Section::UpdatePoolRelocationsOffsets(int offset)
{
    for (auto rel : poolRelocations)
    {
        rel->offset += offset;
    }
}

void Section::UpdateSectionRelocationsAddends(int offset)
{
    for (auto rel : sectionRelocations)
    {
        if (rel->type == eRelocationType::REL12_PC)
            rel->addend += offset;
    }
}

Section::s_ptr Section::MergeSections(Section::s_ptr &first, Section::s_ptr &second)
{
    Section::s_ptr result = std::make_shared<Section>(*first);

    int sectionOffset = first->data.size();
    result->AppendData(second->data);

    second->UpdateSectionRelocationsOffsets(sectionOffset);
    result->sectionRelocations.insert(result->sectionRelocations.end(),
        second->sectionRelocations.begin(), second->sectionRelocations.end());

    int poolOffset = first->literalPool.size();
    second->UpdateSectionRelocationsAddends(poolOffset);

    result->literalPool.insert(result->literalPool.end(),
        second->literalPool.begin(), second->literalPool.end());

    second->UpdatePoolRelocationsOffsets(poolOffset);
    result->poolRelocations.insert(result->poolRelocations.end(),
        second->poolRelocations.begin(), second->poolRelocations.end());

    return result;
}

Section::s_ptr Section::FindSection(const std::vector<Section::s_ptr>& table, const std::string &name)
{
    auto iter = std::find_if(table.begin(), table.end(),
        [&](const Section::s_ptr& sec) { return sec->name == name; });

    if (iter == table.end())
        return nullptr;

    return *iter;
}

/// @brief vraca index u bazenu. index je broj uint32_t vrednosti (4 bajta) pre odredjenog literala
ADDRESS Section::IsLiteralPresentInPool(const uint32_t value) const
{
    auto bytes = IntToByteArray(value);

    int64_t addressEntry = -1;

    for (unsigned int entryIndex = 0; entryIndex < literalPool.size(); entryIndex += 4)
    {
        if (literalPool[entryIndex] == bytes.front())
        {
            bool isSame = Compare(
                std::vector<uint8_t>(literalPool.begin() + entryIndex, literalPool.begin() + entryIndex + 4),
                bytes);

            if (isSame)
                return entryIndex;
        }
    }

    return addressEntry;
}

void Section::WritePoolEntry(const uint32_t &value, const uint32_t &entry)
{
    auto bytes = IntToByteArray(value);
    int start = PoolEntryToAddress(entry);

    for (int i = 0; i < 4; i++)
    {
        literalPool[start + i] = bytes[i];
    }
}
