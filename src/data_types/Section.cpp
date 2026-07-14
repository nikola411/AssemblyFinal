#include "Section.hpp"

#include "Utility.hpp"


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

ADDRESS Section::InsertLiteralInPool(uint32_t value)
{
    auto bytes = IntToByteArray(value);
    literalPool.insert(literalPool.end(), bytes.begin(), bytes.end());

    return literalPool.size() - bytes.size();
}

uint32_t Section::AddressToPoolEntry(ADDRESS address)
{
    return address / 4;
}

ADDRESS Section::PoolEntryToAddress(uint32_t entry)
{
    return entry * 4;
}

void Section::AddSectionRelocation(const eRelocationType &type, const std::string &name, int offset)
{
    Relocation::s_ptr relocation = std::make_shared<Relocation>();

    relocation->type = type;
    relocation->offset = offset;
    relocation->symbolName = name;

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
