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
    for (ForwardRefference::s_ptr& fref : mForwardRefTable)
    {
        std::string symbolName = fref->symbolName;
        auto entry = GetSymbol(mSymbolTable, symbolName);
        if (!entry)
            throw AssemblyException(ExceptionMessage::UndefinedSymbol, { symbolName });

        for (auto& section : mSectionTable)
        {
            if (section->name == fref->sectionName)
            {
                // ukoliko je simbol nadjen u drugoj sekciji, moramo da generisemo relokaciju
                if (entry->section != section->name)
                {
                    // upisujemo placeholder vrednost u bazen literala, gde cemo upisati realnu vrednost u vreme linkovanja
                    auto poolEntry = section->InsertLiteralInPool(0);
                    int16_t poolAddress = section->locationCounter + poolEntry;

                    section->AddSectionRelocation(eRelocationType::REL12_PC, entry->name, fref->offset);
                    section->AddPoolRelocation(eRelocationType::REL32_ABS, entry->name, poolAddress);
                    break;
                }

                eValueSize operandSize = GetOperandValueSize(fref->instruction);
                // vrednost operanda je mala - mozemo je zameniti u mestu
                if (operandSize == eValueSize::SIZE_SMALL)
                {
                    int16_t displacement = entry->offset;
                    CHECK_SMALL_VALUE(displacement);
                    section->WriteInstructionDisplacement(fref->offset, displacement & 0xFFF);
                    break;
                }

                // vrednost operanda je velika, moramo da generisemo relokaciju i da nadjemo ulaz u bazenu literala
                auto poolEntry = section->IsLiteralPresentInPool(entry->offset);
                if (poolEntry == -1)
                    poolEntry = section->InsertLiteralInPool(entry->offset);

                int16_t displacement = (section->locationCounter + poolEntry) - fref->offset - 4;
                CHECK_SMALL_VALUE(displacement);
                section->WriteInstructionDisplacement(fref->offset, displacement & 0xFFF);

                // generisemo relokaciju na offsetu gde je generisana fref, jer cemo morati da upisemo novi offset
                // kada se sekcije spoje u linking fazi
                section->AddSectionRelocation(eRelocationType::REL12_PC, entry->name, fref->offset);

                break;
            }
        }
    }

    CheckForErrors();

    std::cout << SymbolTableToString(mSymbolTable);
    std::cout << SectionTableToString(mSectionTable);
    //std::cout << RelocationTableToString(mRelocationTable);
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
    // ucitaj labelu i skini :
    auto value = mCurrentInstruction->operands.front().value;
    value = value.substr(0, value.size() - 1);

    // radimo lookup u tabelu simobla da vidimo da li je labela vec definisana -> greska
    auto entry = GetSymbol(mSymbolTable, value);
    if (entry && entry->defined)
        throw AssemblyException(ExceptionMessage::SymbolAlreadyDefined, { entry->name });

    // pravimo ulaz u tabelu simoblal  i popunjavamo
    if (!entry)
    {
        entry = std::make_shared<Symbol>();
        mSymbolTable.push_back(entry);
        entry->isGlobal = false;
    }

    auto poolName = mCurrentSection->name + ".pool";
    if (!GetSymbol(mSymbolTable, poolName))
    {
        auto poolEntry = std::make_shared<Symbol>();
        poolEntry->name = poolName;
        poolEntry->section = poolName;
        poolEntry->defined = true;
        poolEntry->offset = 0;
        mSymbolTable.push_back(poolEntry);
    }

    entry->name = value;
    entry->section = mCurrentSection->name;
    entry->offset = mCurrentSection->locationCounter;
    entry->defined = true;

    auto address = mCurrentSection->IsLiteralPresentInPool(mCurrentSection->locationCounter);
    if (address == -1)
        address = mCurrentSection->InsertLiteralInPool(mCurrentSection->locationCounter);

    entry->value = address;
    entry->isBig = true;

    //GenerateRelocation(eRelocationType::REL32_ABS, entry->name, );
}

void Assembly::HandleDirective()
{
    switch(mCurrentInstruction->identifier)
    {
        case eAssemblyIdentifier::GLOBAL:
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
        case eAssemblyIdentifier::SECTION:
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
        case eAssemblyIdentifier::EXTERN:
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
        case eAssemblyIdentifier::WORD:
        {
            for (auto& operand : mCurrentInstruction->operands)
            {
                uint32_t value;
                std::vector<BYTE> memoryValue;

                if (operand.type == eOperandType::LTR)
                {
                    value = LiteralStringToInt(operand.value);
                    memoryValue = IntToByteArray(value);

                    mCurrentSection->AppendData(memoryValue);
                    continue;
                }

                value = GetSymbolValue(operand);
                memoryValue = IntToByteArray(value);
                mCurrentSection->AppendData(memoryValue);
            }

            break;
        }
        case eAssemblyIdentifier::SKIP:
        {
            uint32_t bytesToAllocate = LiteralStringToInt(mCurrentInstruction->operands.front().value);
            mCurrentSection->AppendData(std::vector<uint8_t>(bytesToAllocate, 0));
            break;
        }
        case eAssemblyIdentifier::END:
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

uint16_t Assembly::GetSymbolValue(ParserOperand& operand)
{
    std::string name = operand.value;
    uint16_t value = 0;
    auto entry = GetSymbol(mSymbolTable, name);

    // ako nismo nasisli na definiciju simbola, generisemo fref
    if (!entry)
    {
        GenerateForwardReference(name);
        return 0;
    }
    // za eksterne vrednosti, definisemo rel32_direct relokaciju jer ne znamo velicinu simbola, pa svakako koristimo taj tip relokacije
    if (entry->isExtern)
    {
        mCurrentSection->AddPoolRelocation(eRelocationType::REL32_ABS, name, mCurrentSection->literalPool.size());
        mCurrentSection->InsertLiteralInPool(0);
        operand.isBigValue = true;
        return value;
    }
    // relokacija izmedju sekcija ili relokacija prilikom nedefinisanosti simbola
    if ((!entry->defined || entry->section != mCurrentSection->name) && !entry->isExtern)
    {
        mCurrentSection->AddPoolRelocation(eRelocationType::REL32_ABS, name, mCurrentSection->literalPool.size());
        mCurrentSection->InsertLiteralInPool(0);
        operand.isBigValue = true;
        return value;
    }

    value = entry->value;
    // ukoliko instrukcija podrzava velike vrednosti (vise od 12 bita), gledamo u bazen literala
    if (IsBigValueInstruction(mCurrentInstruction->identifier, mCurrentInstruction->GetAddressingType(), ePayloadType::PAYLOAD_VALUE))
    {
        int address = mCurrentSection->IsLiteralPresentInPool(value);
        if (address == -1)
            address = mCurrentSection->InsertLiteralInPool(value);

        // zbog potencijalnog pomeranja bazena literala
        mCurrentSection->AddSectionRelocation(eRelocationType::REL12_PC, entry->name, entry->offset);

        operand.isBigValue = true;
        return Section::AddressToPoolEntry(address);
    }
    // instrukcija podrzava samo vrednosti do 12 bita, pa proveravamo da li postoji greska
    CHECK_SMALL_VALUE(value);
    return value;
}

uint16_t Assembly::GetLiteralValue(ParserOperand& operand)
{
    std::string literal = operand.value;
    uint32_t value = LiteralStringToInt(literal);
    if (IsBigValueInstruction(mCurrentInstruction->identifier, mCurrentInstruction->GetAddressingType(), ePayloadType::PAYLOAD_VALUE))
    {
        int address = mCurrentSection->IsLiteralPresentInPool(value);
        if (address == -1)
            address = mCurrentSection->InsertLiteralInPool(value);

        mCurrentSection->AddSectionRelocation(eRelocationType::REL12_PC, mCurrentSection->name, mCurrentSection->locationCounter);

        operand.isBigValue = true;
        return address - mCurrentSection->locationCounter;
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
    // ovde kazemo da je operand type literal, i podrzavamo samo vrednosti do 12 bita
    if (operand.offsetType == eOperandType::LTR)
    {
        uint32_t offsetValue = LiteralStringToInt(operand.offset);
        CHECK_SMALL_VALUE(offsetValue);

        operand.asmOffset = static_cast<uint16_t>(offsetValue);
        return;
    }

    operand.asmOffset = 0;
    auto entry = GetSymbol(mSymbolTable, operand.offset);
    if (!entry || (!entry->defined && !entry->isExtern))
    {
        GenerateForwardReference(operand.offset);
        return;
    }

    if (entry->isExtern)
    {
        mCurrentSection->AddSectionRelocation(eRelocationType::REL12_ABS, entry->name, std::stoi(operand.offset));
        return;
    }

    operand.asmOffset = entry->value;
}

void Assembly::WriteInstructionToSection(const AssemblyLine::s_ptr& instruction)
{
    AssemblyLineMetadata metadata = Conversion::GetProcessorInstructions(instruction);

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


void Assembly::GenerateForwardReference(const std::string& name)
{
    auto fref = std::make_shared<ForwardRefference>();
    fref->offset = mCurrentSection->locationCounter;
    fref->sectionName = mCurrentSection->name;
    fref->symbolName = name;
    fref->instruction = mCurrentInstruction;

    mForwardRefTable.push_back(fref);
}

inline void Assembly::IncrementLocationCounter(uint32_t amount)
{
    mCurrentSection->locationCounter += amount;
}