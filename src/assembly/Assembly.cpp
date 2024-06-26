#include "Assembly.hpp"
#include "Utility.hpp"
#include "InstructionMapping.hpp"

#include <iostream>
#include <iomanip>
#include <memory>
#include <bitset>
#include <fstream>

#define DEFAULT_SECTION ".GLOBAL_DATA"

#define CHECK_SMALL_VALUE(value) \
    if ((value & ~(0x800)) > 0x7FF) \
        throw AssemblyException(ExceptionMessage::LiteralOffsetTooBig, { std::to_string(value) });

#include <chrono>

using namespace Conversion;

Assembly::Assembly() :
    mEnd(false)
{
    mCurrentSection = std::make_shared<Section>();
    mCurrentSection->name = DEFAULT_SECTION;

    Conversion::PopulateInstructionsMap();
    int x = 1;
    //m_symbolAndLiteralPool = {};
}

void Assembly::ContinueParsing()
{
    for (auto& fref : mForwardRefTable)
    {
        std::string symbolName = fref->symbolName;
        auto entry = GetSymbol(mSymbolTable, symbolName);
        if (!entry)
            throw AssemblyException(ExceptionMessage::UndefinedSymbol, { symbolName });

        for (auto& section : mSectionTable)
        {
            if (section->name == fref->sectionName)
            {
                int16_t offset = section->data.size() - fref->offset;
                CHECK_SMALL_VALUE(offset);

                offset &= 0xFFF;
                section->WriteInstructionDisplacement(fref->offset, offset);
                break;
            }
        }
    }

    CheckForErrors();

    std::cout << SymbolTableToString(mSymbolTable);
    std::cout << SectionTableToString(mSectionTable);
    std::cout << RelocationTableToString(mRelocationTable);
}

void Assembly::PrintProgram(std::string outFile)
{
    std::fstream file;
    file.open(outFile, std::ios_base::openmode::_S_out);

    file << SymbolTableToString(mSymbolTable);
    // file << SectionsToString(mSectionTable);
    // file << RelocationTableToString(mRelocationTable);
    
    file.close();
}

void Assembly::CheckForErrors() const
{
    for (const auto& symbol : mSymbolTable)
    {
        if (!symbol->defined && symbol->isGlobal)
            throw AssemblyException(ExceptionMessage::ExportingUndefinedSymbol, { symbol->name });

        if (symbol->defined && symbol->isExtern)
            throw AssemblyException(ExceptionMessage::ImportingDefinedSymbol, { symbol->name });
    }    
}

void Assembly::HandleLabel()
{
    auto value = mCurrentInstruction->operands.front().value;
    value = value.substr(0, value.size() - 1);

    auto entry = GetSymbol(mSymbolTable, value);
    if (entry && entry->defined)
        throw AssemblyException(ExceptionMessage::SymbolAlreadyDefined, { entry->name });
    
    if (!entry)
    {
        entry = std::make_shared<Symbol>();
        mSymbolTable.push_back(entry);
        entry->isGlobal = false;
    }
        
    entry->name = value;
    entry->section = mCurrentSection->name;
    entry->offset = mCurrentSection->locationCounter;
    entry->defined = true;

    if (mCurrentSection->locationCounter < 0x7FF)
    {
        entry->value = mCurrentSection->locationCounter;
        entry->isBig = false;
        return;
    }

    auto address = mCurrentSection->IsLiteralPresentInPool(mCurrentSection->locationCounter);
    if (address == -1)
        address = mCurrentSection->InsertLiteralInPool(mCurrentSection->locationCounter);

    entry->value = Section::AddressToPoolEntry(address);
    entry->isBig = true;
}

void Assembly::HandleDirective()
{
    switch(mCurrentInstruction->identifier)
    {
        case eInstructionIdentifier::GLOBAL:
        {
            for (const auto& operand : mCurrentInstruction->operands)
            {
                auto entry = GetSymbol(mSymbolTable, operand.value);
                if (!entry)
                {
                    entry = std::make_shared<Symbol>();
                    entry->name = operand.value;
                    mSymbolTable.push_back(entry);
                }

                entry->isGlobal = true;
            }
            break;
        }
        case eInstructionIdentifier::SECTION:
        {
            mSectionTable.push_back(mCurrentSection);
            mCurrentSection = std::make_shared<Section>();
            mCurrentSection->name = mCurrentInstruction->operands.front().value;
            mCurrentSection->locationCounter = 0;
            mCurrentSection->data = {};

            auto entry = std::make_shared<Symbol>();
            entry->name = mCurrentSection->name;
            entry->section = mCurrentSection->name;
            entry->defined = true;
            entry->offset = 0;
            mSymbolTable.push_back(entry);

            break;
        }
        case eInstructionIdentifier::EXTERN:
        {
            /*
                Define the symbol in symbol table as extern and every time we encounter this symbol we will generate a relocation
            */
            for (const auto& operand : mCurrentInstruction->operands)
            {
                auto entry = std::make_shared<Symbol>();
                entry->name = operand.value;
                entry->defined = false;
                entry->section = DEFAULT_SECTION;
                entry->isExtern = true;

                mSymbolTable.push_back(entry);
            }

            break;
        }
        case eInstructionIdentifier::WORD:
        {
            for (const auto& operand : mCurrentInstruction->operands)
            {
                if (operand.type == eOperandType::LTR)
                {
                    mCurrentSection->AppendData(IntToByteArray(LiteralStringToInt(operand.value)));
                    continue;
                }
                uint16_t value = GetSymbolValue(operand);
                mCurrentSection->AppendData(IntToByteArray(value));
            }

            break;
        }
        case eInstructionIdentifier::SKIP:
        {
            uint32_t bytesToAllocate = LiteralStringToInt(mCurrentInstruction->operands.front().value);
            mCurrentSection->AppendData(std::vector<uint8_t>(bytesToAllocate, 0));
            break;
        }
        case eInstructionIdentifier::END:
        {
            mEnd = true;
            mSectionTable.push_back(mCurrentSection);
            break;
        }
    }
}

void Assembly::HandleInstruction()
{
    DecodeInstructionValues();
    WriteInstructionToSection(mCurrentInstruction);
}

uint16_t Assembly::GetSymbolValue(const ParserOperand& operand)
{
    std::string name = operand.value;
    uint16_t value = 0;
    auto entry = GetSymbol(mSymbolTable, name);

    if (!entry)
    {
        GenerateForwardRefference(name);
        return 0;
    }
        
    if (entry->isExtern)
    {
        GenerateRelocation(eRelocationType::REL12_DIRECT, name);
        return value;
    }
        
    if ((!entry->defined || entry->section != mCurrentSection->name) && !entry->isExtern)
    {
        GenerateRelocation(eRelocationType::REL32_DIRECT, name);
        return value;
    }

    value = entry->value;

    if (IsBigValueInstruction(mCurrentInstruction->identifier, mCurrentInstruction->addressing, ePayloadType::PAYLOAD_VALUE))
    {
        int address = mCurrentSection->IsLiteralPresentInPool(value);
        if (address == -1)
            address = mCurrentSection->InsertLiteralInPool(value);

        return Section::AddressToPoolEntry(address);
    }

    CHECK_SMALL_VALUE(value);
    return value;
}

uint16_t Assembly::GetLiteralValue(const ParserOperand& operand)
{
    std::string literal = operand.value;
    uint32_t value = LiteralStringToInt(literal);
    if (IsBigValueInstruction(mCurrentInstruction->identifier, mCurrentInstruction->addressing, ePayloadType::PAYLOAD_VALUE))
    {
        int address = mCurrentSection->IsLiteralPresentInPool(value);
        if (address == -1)
            address = mCurrentSection->InsertLiteralInPool(value);

        return Section::AddressToPoolEntry(address);
    }

    CHECK_SMALL_VALUE(value);
    return value;
}

void Assembly::DecodeInstructionValues()
{
    for (auto& operand : mCurrentInstruction->operands)
    {
        if (!operand.offset.empty())
        {
            CalculateOperandOffset(operand);
        }

        CalculateOperandValue(operand);
    }
}

void Assembly::CalculateOperandValue(ParserOperand& operand)
{
    if (operand.type == eOperandType::GPR || operand.type == eOperandType::CSR)
    {
        operand.asmValue = operand.type == eOperandType::GPR
            ? GPRStringToEnum(operand.value)
            : CSRStringToEnum(operand.value);
        return;
    }

    if (operand.type == eOperandType::LTR)
    {
        operand.asmValue = GetLiteralValue(operand);
        return;
    }

    operand.asmValue = GetSymbolValue(operand);
}

void Assembly::CalculateOperandOffset(ParserOperand& operand)
{
    if (operand.offsetType == eOperandType::LTR)
    {
        uint32_t offsetValue = LiteralStringToInt(operand.offset);
        CHECK_SMALL_VALUE(offsetValue);
        
        operand.asmOffset = static_cast<uint16_t>(offsetValue);
        return;
    }

    operand.asmValue = 0;
    auto entry = GetSymbol(mSymbolTable, operand.offset);
    if (!entry || (!entry->defined && !entry->isExtern))
    {
        GenerateForwardRefference(operand.offset);
        return;   
    }

    if (entry->isExtern)
    {
        GenerateRelocation(eRelocationType::REL12_DIRECT, operand.offset);
        return;
    }

    operand.asmValue = entry->value;
}

void Assembly::WriteInstructionToSection(const AssemblyInstruction::s_ptr& instruction)
{
    AssemblyInstructionMetadata metadata = Conversion::GetProcessorInstructions(instruction);

    for (const auto& entry : metadata)
    {
        auto instructionValue = entry.first;
        
        for (const auto& manipulationPair : entry.second)
        {
            uint16_t value = GetOperandValue(instruction, manipulationPair.first);
            (instructionValue.*manipulationPair.second)(value);
        }

        auto bytes = IntToByteArray(instructionValue.data);
        mCurrentSection->AppendData(bytes);
    }
}

void Assembly::GenerateRelocation(const eRelocationType& type, const std::string& name)
{
    auto relocation = std::make_shared<Relocation>();
    relocation->type = type;
    relocation->offset = mCurrentSection->locationCounter;
    relocation->sectionName = mCurrentSection->name;
    relocation->symbolName = name;

    mRelocationTable.push_back(relocation);
}

void Assembly::GenerateForwardRefference(const std::string& name)
{
    auto relocation = std::make_shared<Relocation>();
    relocation->offset = mCurrentSection->locationCounter;
    relocation->sectionName = mCurrentSection->name;
    relocation->symbolName = name;

    mForwardRefTable.push_back(relocation);
}

inline void Assembly::IncrementLocationCounter(uint32_t amount)
{
    mCurrentSection->locationCounter += amount;
}