#include "Assembly.hpp"
#include "Utility.hpp"

#include <iostream>
#include <iomanip>
#include <memory>
#include <bitset>
#include <fstream>

#define LOG(func) \
    std::cout << "[" <<__FILE__ << " - " << __LINE__ << "]: "<< func <<"\n";

#define FIRST_PASS false
#define SECOND_PASS true
#define INSTRUCTION_SIZE 4
#define LOCAL true
#define DEFINED true
#define UNDEFIED false
#define DEFAULT_SECTION "default"
#define PAYLOAD_MASK 0x0FFF

#define CallInstructionMethod \
    (instruction.*function->method)
#define CallAssemblyMethod(pass, line) \
    (this->*m_handles[pass][line->type][line->instruction])(line)

#define NO_ACTION &Assembly::DoNothing

//using namespace AssemblyUtil;
//using namespace ParserUtil;
//
Assembly::Assembly() :
    m_end(FIRST_PASS), m_locationCounter(0)
{
    //InstructionUtil::CodesMap::PopulateMap();
    m_currentSection = std::make_shared<Section>();
    m_currentSection->name = DEFAULT_SECTION;

    //m_symbolAndLiteralPool = {};
}

void Assembly::SetInstruction(eInstructionIdentifier instruction, eInstructionType type)
{
    m_currentInstruction = std::make_shared<AssemblyInstruction>();
    m_currentInstruction->identifier = instruction;
    m_currentInstruction->type = type;
}

void Assembly::SetOperand(std::string value, eOperandType type)
{
    m_currentInstruction->operands.push_back(ParserOperand(value, type));
}

void Assembly::SetOperand(ParserOperand& operand)
{
    m_currentInstruction->operands.push_back(operand);
}

void Assembly::SetMultipleOperands(std::vector<ParserOperand> operands)
{
    for (auto op : operands)
    {
        m_currentInstruction->operands.push_back(op);
    }
}

void Assembly::FinishInstruction()
{
    //if (m_end == SECOND_PASS)
    //    return;
//
    //// first pass 
    //AnalyzeInstruction();    
//
    m_program.push_back(m_currentInstruction);
}

void Assembly::ContinueParsing()
{
}

void Assembly::PrintProgram(std::string outFile)
{
    //std::fstream file;
    //file.open(outFile, std::ios_base::openmode::_S_out);
//
    //file << SymbolTableToString(m_symbolTable);
    //file << SectionsToString(m_sections);
    //file << RelocationTableToString(m_relocationTable);
    //
    //file.close();
}

void Assembly::AnalyzeInstruction()
{

    //eOperandType mainOperandType = GetInstructionOperandType();
    //LOG(mainOperandType << " " << m_currentLine->type);
//
    //switch (m_currentLine->type)
    //{
    //    // instruction types that can use/define symbols
    //    case eInstructionType::LABEL:
    //    {
    //        break;
    //    }
    //    case eInstructionType::DIRECTIVE:
    //    {
    //        break;
    //    }
    //    case eInstructionType::MEMORY:
    //    {
    //        break;
    //    }
    //    case eInstructionType::BRANCH:
    //    {
    //        break;
    //    }
    //    // instruction types that cannot define/use symbol and can be decoded right away
    //    case eInstructionType::SPECIAL:
    //    case eInstructionType::PROCESSOR:
    //    case eInstructionType::DATA:
    //    case eInstructionType::STACK:
    //    {
    //        //decode instruction
    //        //write to section
    //        
    //        m_locationCounter += 4;
    //        break;
    //    }
//
    //}
    //// check for backref
    //for (const auto& operand : m_currentLine->operands)
    //{
    //    if (operand.type == eOperandType::SYM)
    //    {
    //        auto symEntry = FindSymbol(m_symbolTable, const_cast<std::string&>(operand.value));
    //        if (!symEntry || !symEntry->defined)
    //        {
    //            m_backrefference[operand.value].push_back({ m_currentSection->locationCounter, m_currentSection->name });
    //        }
    //    }
    //}


}

eOperandType Assembly::GetInstructionOperandType() const
{
    //eOperandType type = NONE_TYPE;
//
    //for (const auto& operand : m_currentLine->operands)
    //{
    //    if (type != eOperandType::SYM || operand.type == eOperandType::SYM)
    //        type = operand.type;
    //}
//
    //return type;
    return eOperandType();
}