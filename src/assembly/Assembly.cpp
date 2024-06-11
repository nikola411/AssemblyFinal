#include "Assembly.hpp"
#include "Utility.hpp"

#include <iostream>
#include <iomanip>
#include <memory>
#include <bitset>
#include <fstream>

#define INSTRUCTION_SIZE 4
#define LOCAL true
#define DEFINED true
#define UNDEFIED false
#define DEFAULT_SECTION "default"
#define PAYLOAD_MASK 0x0FFF

#define CallInstructionMethod\
    (instruction.*function->method)
#define CallAssemblyMethod(pass, line) \
    (this->*m_handles[pass][line->type][line->instruction])(line)

#define NO_ACTION &Assembly::DoNothing

Assembly::Assembly() :
    mEnd(false)
{
    CodesMap::PopulateMap();
    mCurrentSection = std::make_shared<Section>();
    mCurrentSection->name = DEFAULT_SECTION;

    //m_symbolAndLiteralPool = {};
}

void Assembly::SetInstruction(eInstructionIdentifier instruction, eInstructionType type)
{
    mCurrentInstruction = std::make_shared<AssemblyInstruction>();
    mCurrentInstruction->identifier = instruction;
    mCurrentInstruction->type = type;
}

void Assembly::SetOperand(std::string value, eOperandType type)
{
    mCurrentInstruction->operands.push_back(ParserOperand(value, type));
}

void Assembly::SetOperand(ParserOperand& operand)
{
    mCurrentInstruction->operands.push_back(operand);
}

void Assembly::SetMultipleOperands(std::vector<ParserOperand> operands)
{
    for (auto op : operands)
    {
        mCurrentInstruction->operands.push_back(op);
    }
}

AsmResult Assembly::FinishInstruction()
{
    AsmResult result = ASM_RESULT_NOT_SUPPORTED;

    if (mEnd)
        return ASM_RESULT_END_ENCOUNTERED;

    switch(mCurrentInstruction->type)
    {
        case eInstructionType::LABEL:
        {
            result = HandleLabel();
            break;
        }
        case eInstructionType::DIRECTIVE:
        {
            result = HandleDirective();
            break;
        }
        default:
        {
            result = HandleInstruction();
            break;
        }
    }   

    mProgram.push_back(mCurrentInstruction);

    return result;
}

void Assembly::ContinueParsing()
{
}

void Assembly::PrintProgram(std::string outFile)
{
    //std::fstream file;
    //file.open(outFile, std::ios_base::openmode::_S_out);
//
    //file << SymbolTableToString(mSymbolTable);
    //file << SectionsToString(m_sections);
    //file << RelocationTableToString(m_relocationTable);
    //
    //file.close();
}

AsmResult Assembly::HandleLabel()
{
    auto entry = std::make_shared<Symbol>();
    entry->name = mCurrentInstruction->operands.front().value;
    entry->section = mCurrentSection->name;
    entry->offset = mCurrentSection->locationCounter;
    entry->isGlobal = false;
    entry->defined = true;

    auto address = mCurrentSection->IsLiteralPresentInPool(mCurrentSection->locationCounter);
    if (address == -1)
        address = mCurrentSection->InsertLiteralInPool(mCurrentSection->locationCounter);

    entry->value = Section::AddressToPoolEntry(address);

    mSymbolTable.push_back(entry);

    return ASM_RESULT_SUCCESS;
}

AsmResult Assembly::HandleDirective()
{
    switch(mCurrentInstruction->identifier)
    {
        case eInstructionIdentifier::GLOBAL:
        {
            for (const auto& operand : mCurrentInstruction->operands)
            {
                auto entry = GetSymbol(mSymbolTable, operand.value);
                if (entry == nullptr)
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
                    mCurrentSection->AppendData(std::vector<uint8_t>(4, 0));
                    mCurrentSection->WriteData(mCurrentSection->locationCounter - 4, IntToByteArray(LiteralStringToInt(operand.value)));
                    continue;
                }
                //else if (operand.type == eOperandType::SYM)
                uint32_t value = 0;
                auto entry = GetSymbol(mSymbolTable, operand.value);
                if (entry == nullptr)
                {
                    //relocation forward ref
                }

                //add zeroes
                mCurrentSection->AppendData(std::vector<uint8_t>(4, 0));
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
            break;
        }
    }

    return ASM_RESULT_SUCCESS;
}

AsmResult Assembly::HandleInstruction()
{
    AsmResult result = CalculateOperandsValue();
    if (result != ASM_RESULT_SUCCESS)
        return result;
    
    return WriteInstructionToSection(mCurrentInstruction);
}

uint16_t Assembly::GetSymbolValue(const std::string& name)
{
    uint16_t value = 0;
    auto entry = GetSymbol(mSymbolTable, name);

    if (!entry || entry->section != mCurrentSection->name)
    {
        // relocation or backref
        auto relocation = std::make_shared<Relocation>();
        relocation->offset = mCurrentSection->locationCounter;
        relocation->sectionName = mCurrentSection->name;
        relocation->type = eRelocationType::REL_FORWARD_REFERENCING;
        relocation->symbolName = name;

        mForwardRefTable.push_back(relocation);
    }

    if (entry && entry->section == mCurrentSection->name)
        value = entry->value;
    
    return value;
}

uint16_t Assembly::GetLiteralValue(const std::string& literal)
{
    uint32_t value = LiteralStringToInt(literal);

    int address = mCurrentSection->IsLiteralPresentInPool(value);
    if (address == -1)
        address = mCurrentSection->InsertLiteralInPool(value);

    return Section::AddressToPoolEntry(address);
}

AsmResult Assembly::CalculateOperandsValue()
{
    for (auto& operand : mCurrentInstruction->operands)
    {
        if (operand.hasOffset)
        {
            if (operand.offsetType == eOperandType::LTR)
            {
                uint32_t offsetValue = LiteralStringToInt(operand.offset);
                if (offsetValue > 0x7FF)
                    return ASM_RESULT_LITERAL_OFFSET_TOO_BIG;
                
                operand.asmOffset = static_cast<uint16_t>(offsetValue);
            }
            else
            {
                operand.asmOffset = GetSymbolValue(operand.offset);
            }
        }

        if (operand.type == eOperandType::GPR || operand.type == eOperandType::CSR)
        {
            operand.asmValue = operand.type == eOperandType::GPR
                ? GPRStringToEnum(operand.value)
                : CSRStringToEnum(operand.value);
            
            continue;
        }

        if (operand.type == eOperandType::LTR)
        {
            operand.asmValue = GetLiteralValue(operand.value);
            continue;
        }

        operand.asmValue = GetSymbolValue(operand.value);
    }

    return ASM_RESULT_SUCCESS;
}

AsmResult Assembly::WriteInstructionToSection(const AssemblyInstruction::s_ptr& instruction)
{
    auto operands = instruction->operands;    
    auto operandType = instruction->GetOperandType();
    auto identifier = instruction->identifier;
    auto addressingType = instruction->GetAddressingType();

    auto populationMetadata = CodesMap::GetInstructionCodes(identifier, operandType, addressingType);
    if (populationMetadata.empty())
        return ASM_RESULT_CODE_MAP_EMPTY;
    
    for (const auto& entry : populationMetadata)
    {
        Instruction instruction = entry.first;
        for (const auto& method : entry.second)
        {
            if (!method) // for instructions without operands
                continue;
            
            if (method->operand > operands.size())
                return ASM_RESULT_CODE_MAP_INVALID_DATA;

            uint16_t operandValue = operands[method->operand].asmValue;
            (instruction.*method->method)(operandValue);

            uint32_t data = instruction.GetData();
            std::vector<uint8_t> bytes = IntToByteArray(data);
            mCurrentSection->AppendData(bytes);
        }
    }

    return ASM_RESULT_SUCCESS;
}

inline void Assembly::IncrementLocationCounter(uint32_t amount)
{
    mCurrentSection->locationCounter += amount;
}