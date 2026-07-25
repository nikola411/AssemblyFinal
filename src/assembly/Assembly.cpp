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

    auto poolIndex = mCurrentSection->IsLiteralPresentInPool(mCurrentSection->locationCounter);
    if (poolIndex == -1)
        poolIndex = mCurrentSection->InsertLiteralInPool(mCurrentSection->locationCounter);

    entry->value = poolIndex;
    // treba nam relokacija jer u vreme linkovanja, sama adresa labele je nepoznata
    mCurrentSection->AddPoolRelocation(eRelocationType::REL32_ABS, entry->name, mCurrentSection->literalPool.size());
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

                auto symbol = GetSymbol(mSymbolTable, operand.value);
                value = GetAbsoluteSymbolValue(symbol);

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
        case eAssemblyIdentifier::EQU:
        {
            auto operands = mCurrentInstruction->operands;
            auto entry = GetSymbol(mSymbolTable, operands.front().value);
            if (entry)
            {
                throw std::exception();
            }

            entry = std::make_shared<Symbol>();
            entry->name = operands.front().value;
            entry->value = std::stoi(operands.back().value, nullptr, 16);
            entry->isConstant = true;
            mSymbolTable.push_back(entry);
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
    for (auto& operand : mCurrentInstruction->operands)
    {
        operand.asmValue = operand.type == eOperandType::GPR
            ? GPRStringToEnum(operand.value)
            : CSRStringToEnum(operand.value);
    }

    WriteInstructionToSection(mCurrentInstruction);
}

void Assembly::HandleDataInstruction()
{
    for (auto& operand : mCurrentInstruction->operands)
    {
        if (!operand.offset.empty())
        {
            CalculateDataOperandOffset(operand);
        }

        CalculateOperandValue(operand);
    }

    WriteInstructionToSection(mCurrentInstruction);
}

void Assembly::HandleBranchInstruction()
{
    auto variableOperand = mCurrentInstruction->operands.back();
    CalculateOperandValue(variableOperand);

    WriteInstructionToSection(mCurrentInstruction);
}

uint32_t Assembly::GetSymbolValue(const std::string& name)
{
    uint32_t value = 0;
    auto entry = GetSymbol(mSymbolTable, name);

    if (!entry || !entry->defined)
    {
        GenerateForwardReference(name, ForwardRefferenceType::FREF12_PC);
        return value;
    }

    if (entry->isConstant)
        return GetConstantSymbolValue(entry);

    return GetSymbolPoolEntry(entry);
}

uint16_t Assembly::GetSymbolPoolEntry(const Symbol::s_ptr &entry)
{
    uint32_t value = 0;

    // za eksterne vrednosti, definisemo rel32_direct relokaciju jer ne znamo velicinu simbola, pa svakako koristimo taj tip relokacije
    if (entry->isExtern)
    {
        mCurrentSection->AddPoolRelocation(eRelocationType::REL32_ABS, entry->name, mCurrentSection->literalPool.size());
        value = mCurrentSection->InsertLiteralInPool(0);

        return value;
    }
    // relokacija izmedju sekcija ili relokacija prilikom nedefinisanosti simbola
    if ((!entry->defined || entry->section != mCurrentSection->name) && !entry->isExtern)
    {
        mCurrentSection->AddPoolRelocation(eRelocationType::REL32_ABS, entry->name, mCurrentSection->literalPool.size());
        value = mCurrentSection->InsertLiteralInPool(0);

        return value;
    }

    return entry->value;
}

// za .word direktivu, trazimo apsolutnu vrednost simbola
//
uint32_t Assembly::GetAbsoluteSymbolValue(const Symbol::s_ptr& entry)
{
    uint32_t value = 0;

    if (!entry || !entry->defined)
    {
        GenerateForwardReference(entry->name, ForwardRefferenceType::FREF32_ABS);
        return value;
    }

    // word direktiva moze da referencira konstante
    if (entry->isConstant)
        return entry->value;

    // generisemo relokaciju svakako, jer sama vrednost simbola se menja prilikom linkovanja. ovo ne vredi za konstante
    mCurrentSection->AddSectionRelocation(eRelocationType::REL32_ABS, entry->name, mCurrentSection->locationCounter);
    return value;
}

uint32_t Assembly::GetConstantSymbolValue(const Symbol::s_ptr& entry)
{
    uint32_t value = 0;

    if (!entry || !entry->defined)
    {
        throw AssemblyException(ExceptionMessage::UndefinedSymbol, { entry->name });
    }

    if (!entry->isConstant)
    {
        throw AssemblyException(ExceptionMessage::SymbolNotConstant, { entry->name });
    }

    return entry->value;
}

uint16_t Assembly::GetLiteralValue(ParserOperand &operand)
{
    std::string literal = operand.value;
    uint32_t value = LiteralStringToInt(literal);

    int poolEntry = mCurrentSection->IsLiteralPresentInPool(value);
    if (poolEntry == -1)
        poolEntry = mCurrentSection->InsertLiteralInPool(value);

    //mCurrentSection->AddSectionRelocation(eRelocationType::REL12_PC, literal, mCurrentSection->locationCounter);

    return Section::AddressToPoolEntry(poolEntry);
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

    operand.asmValue = GetSymbolValue(operand.value);
}

void Assembly::CalculateDataOperandOffset(ParserOperand& operand)
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
    if (!entry || (!entry->defined && !entry->isExtern) || entry->isExtern)
    {
        throw new AssemblyException(ExceptionMessage::UndefinedSymbol);
    }

    if (entry->isConstant)
    {
        CHECK_SMALL_VALUE(entry->value);
        operand.asmOffset = entry->value;
    }
    else
    {
        throw AssemblyException(ExceptionMessage::SymbolValueTooBig, { entry->name });
    }
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

void Assembly::GenerateForwardReference(const std::string& name, const ForwardRefferenceType& type)
{
    auto fref = std::make_shared<ForwardRefference>();
    fref->offset = mCurrentSection->locationCounter;
    fref->sectionName = mCurrentSection->name;
    fref->symbolName = name;
    fref->instruction = mCurrentInstruction;
    fref->type = type;

    mForwardRefTable.push_back(fref);
}

inline void Assembly::IncrementLocationCounter(uint32_t amount)
{
    mCurrentSection->locationCounter += amount;
}

void Assembly::ContinueParsing()
{
    for (ForwardRefference::s_ptr& fref : mForwardRefTable)
    {
        std::string symbolName = fref->symbolName;
        auto entry = GetSymbol(mSymbolTable, symbolName);
        if (!entry)
            throw AssemblyException(ExceptionMessage::UndefinedSymbol, { symbolName });


        // dodaj find_if umesto for loop-a, citljivije
        for (auto& section : mSectionTable)
        {
            if (section->name != fref->sectionName)
                continue;

            // ukoliko je simbol nadjen u drugoj sekciji, moramo da generisemo relokaciju
            if (entry->section != section->name)
            {
                auto poolEntry = section->InsertLiteralInPool(0);
                int16_t poolAddress = Section::PoolEntryToAddress(poolEntry);

                //section->AddSectionRelocation(eRelocationType::REL12_PC, entry->name, fref->offset);
                section->AddPoolRelocation(eRelocationType::REL32_ABS, entry->name, poolAddress);
                break;
            }

            // vrednost operanda je velika, moramo da generisemo relokaciju i da nadjemo ulaz u bazenu literala
            auto poolEntry = section->IsLiteralPresentInPool(entry->offset);
            if (poolEntry == -1)
                poolEntry = section->InsertLiteralInPool(entry->offset);

            auto poolAddress = Section::PoolEntryToAddress(poolEntry);

            int16_t displacement = (section->locationCounter + poolAddress) - fref->offset - 4;
            CHECK_SMALL_VALUE(displacement);
            section->WriteInstructionDisplacement(fref->offset, displacement & 0xFFF);

            // generisemo relokaciju na offsetu gde je generisana fref, jer cemo morati da upisemo novi offset
            // kada se sekcije spoje u linking fazi
            //section->AddSectionRelocation(eRelocationType::REL12_PC, entry->name, fref->offset);
            section->AddPoolRelocation(eRelocationType::REL32_ABS, entry->name, poolAddress);

            break;
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

        if (symbol->isGlobal && symbol->isExtern)
            throw AssemblyException(ExceptionMessage::SymbolAlreadyDefined, { symbol->name });
    }
}