#include "Utility.hpp"

#include "ParserTypes.hpp"
#include "Symbol.hpp"
#include "Section.hpp"
#include "Relocation.hpp"

#include <iomanip>
#include <string>



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

        if (!section->literalPool.empty())
        {
            stream << section->name << ".pool \n";
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

        if (!section->sectionRelocations.empty())
        {
            stream << section->name << ".rel\n";
            stream << RelocationTableToString(section->sectionRelocations);
        }

        if (!section->poolRelocations.empty())
        {
            stream << section->name << ".pool.rel\n";
            stream << RelocationTableToString(section->poolRelocations);
        }

    }

    return stream.str();
}

static const char* RelocationTypeName(eRelocationType t)
{
    switch (t)
    {
        case eRelocationType::REL12_PC: return "REL12_PC";
        case eRelocationType::REL12_ABS: return "REL12_ABS";
        case eRelocationType::REL32_ABS: return "REL32_ABS";
        default:                            return "UNKNOWN";
    }
}

std::string RelocationTableToString(const RelocationTable& table)
{
    if (table.empty())
        return "\n";

    std::stringstream stream;
    stream << "\n";
    stream << std::setfill(' ')
           << std::setw(18) << "Section"
           << std::setw(18) << "Symbol"
           << std::setw(10) << "Offset"
           << std::setw(16) << "Type"
           << std::setw(10) << "Addend"
           << "\n";
    stream << std::string(72, '-') << "\n";

    for (const auto& r : table)
    {
        stream << std::setfill(' ')
               << std::setw(18) << r->symbolName
               << std::setw(10) << std::dec << r->offset
               << std::setw(16) << RelocationTypeName(r->type)
               << std::setw(10) << r->addend
               << "\n";
    }
    stream << "\n";

    return stream.str();
}

// enum helpers
eGPR GPRStringToEnum(std::string reg)
{
    // covnert sp and pc to r14 and r15 repsectively
    if (reg == "sp" || reg == "pc")
    {
        reg = reg == "sp" ? "r14" : "r15";
    }

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

// int helpers
std::vector<uint8_t> IntToByteArray(const uint32_t& value)
{
    auto result = std::vector<uint8_t>();

    for (uint8_t i = 0; i < 4; ++i)
    {
        auto byte = value >> (i * 8);
        byte &= 0xFF;
        result.push_back(byte);
    }

    return result;
}

std::vector<uint8_t> ShortToByteArray(const uint16_t& value)
{
    return std::vector<uint8_t>();
}

// string helpers
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

bool StartsWith(const std::string& input, const std::string& c1)
{
    return input.substr(0, c1.size()) == c1;
}

bool EndsWith(const std::string& in, const std::string& c1)
{
    if (c1.size() > in.size()) return false;
    return in.substr(in.size() - c1.size(), in.size()) == c1;
}

std::vector<std::string> Split(const std::string& input, char delim)
{
    std::vector<std::string> result = {};
    int last = 0;
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == delim)
        {
            result.push_back(input.substr(last, i - last));
            last = i + 1;
        }
    }

    result.push_back(input.substr(last)); // last token
    return result;
}

