#include "Utility.hpp"

Symbol::s_ptr GetSymbol(const SymbolTable& table, const std::string& label)
{
    for (const auto& entry : table)
        if (entry->name == label)
            return entry;

    return nullptr;
    ///sss///sss
}

eGPR GPRStringToEnum(std::string reg)
{
    return eGPR();
}

eCSR CSRStringToEnum(std::string csr)
{
    return eCSR();
}

std::vector<uint8_t> IntToByteArray(const uint32_t& value)
{
    auto result = std::vector<uint8_t>();

    for (uint8_t i = 0; i < 4; ++i)
    {
        auto byte = value >> ((3 - i) * 8);
        byte &= 0xFF;
        result.push_back(byte);
    }

    return result;
}

std::vector<uint8_t> ShortToByteArray(const uint16_t& value)
{
    return std::vector<uint8_t>();
}

uint32_t LiteralStringToInt(const std::string& value)
{
    int base = 10;
    if (value.find_first_of('x') != std::string::npos)
        base = 16;
    
    return std::stoll(value, 0, base);
}

bool Compare(const std::vector<uint8_t>& first, const std::vector<uint8_t>& second)
{
    if (first.size() != second.size())
        return false;
    
    for (unsigned int i = 0; i < first.size(); ++i)
        if (first[i] != second[i])
            return false;

    return true;
}

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

ADDRESS Section::InsertLiteralInPool(uint32_t value)
{
    auto bytes = IntToByteArray(value);
    literalPool.insert(literalPool.end(), bytes.begin(), bytes.end());

    return literalPool.end() - literalPool.begin();
}

uint32_t Section::AddressToPoolEntry(ADDRESS address)
{
    return address / 4;
}

ADDRESS Section::PoolEntryToAddress(uint32_t entry)
{
    return entry * 4;
}

int64_t Section::IsLiteralPresentInPool(const uint32_t value) const
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
