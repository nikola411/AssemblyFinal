#include "Utility.hpp"

#include <iomanip>
#include <string>

Symbol::s_ptr GetSymbol(const SymbolTable& table, const std::string& label)
{
    for (const auto& entry : table)
        if (entry->name == label)
            return entry;

    return nullptr;
}

std::string SymbolTableToString(const SymbolTable& table)
{
    std::stringstream stream;
    stream << "\n";
    int i = 0;
    stream << std::setw(3) << std::setfill(' ') << ""
        << std::setw(16) << std::setfill(' ') << "Name"
        << std::setw(16) << std::setfill(' ') << "Section"
        << std::setw(8) << std::setfill(' ') << "Offset"
        << std::setw(8) << std::setfill(' ') << "Defined"
        << std::setw(8) << std::setfill(' ') << "Extern"
        << std::setw(8) << std::setfill(' ') << "Global"
        << std::setw(8) << std::setfill(' ') << "Value"
        << std::setw(4) << std::setfill(' ') << "Big"
        << "\n";

    for (const auto& symbol : table)
    {
        stream
            << std::setw(3) << std::setfill(' ') << ++i
            << std::setw(16) << std::setfill(' ') << symbol->name
            << std::setw(16) << std::setfill(' ') << symbol->section
            << std::setw(8) << std::setfill(' ') << symbol->offset
            << std::setw(8) << std::setfill(' ') << symbol->defined
            << std::setw(8) << std::setfill(' ') << symbol->isExtern
            << std::setw(8) << std::setfill(' ') << symbol->isGlobal
            << std::setw(8) << std::setfill(' ') << symbol->value
            << std::setw(4) << std::setfill(' ') << symbol->isBig
            << "\n";
    }
    stream << "\n";

    return stream.str();
}

std::string SectionTableToString(const SectionTable& table)
{
    std::stringstream stream;

    for (const auto& section : table)
    {
        if (section->data.empty())
            continue;

        stream << section->name << " " << section->locationCounter << "\n";
        int eol = 0;
        for (int i = 0; i < section->data.size(); ++i)
        {
            stream << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)section->data[i] << " ";
            if (++eol == 4)
            {
                eol = 0;
                stream << "\n";
            }
        }

        stream << section->name << ".constants \n";
        eol = 0;
        for (int i = 0; i < section->literalPool.size(); ++i)
        {
            stream << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)section->literalPool[i] << " ";
            if (++eol == 4)
            {
                eol = 0;
                stream << "\n";
            }
        }

        stream << "\n";
    }

    return stream.str();
}

std::string RelocationTableToString(const RelocationTable& table)
{
    std::stringstream stream;
    stream << "Relocations \n";
    for (const auto& relocation : table)
    {
        stream << std::setfill(' ') 
             << relocation->sectionName<<std::setw(16)
             << relocation->symbolName<<std::setw(16)
            << relocation->offset<<std::setw(8) 
            << relocation->type <<std::setw(1) << "\n";
    }

    return stream.str();
}

eGPR GPRStringToEnum(std::string reg)
{
    return (eGPR)std::stoi(reg.substr(1, reg.size()));
}

eCSR CSRStringToEnum(std::string csr)
{
    if (csr == "cause")
        return eCSR::CAUSE;
    if (csr == "handler")
        return eCSR::HANDLER;
    
    return eCSR::STATUS;
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

void Section::WriteInstructionDisplacement(const uint32_t& offset, const uint16_t& toWrite)
{
    if (offset + 3 > data.size())
        return;

    data[offset + 2] = (data[offset + 2] & 0xF0) | ((toWrite >> 8) & 0xF);
    data[offset + 3] = toWrite & 0xFF;
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
