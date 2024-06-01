#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

#include "Utility.hpp"
#include "AssemblyInstruction.hpp"
//#include "AssemblyUtil.hpp"
//#include "InstructionUtil.hpp"

#include <vector>
#include <memory>
#include <stdint.h>

class Assembly
{
public:
    Assembly();
    ~Assembly() {};

    void SetInstruction(eInstructionIdentifier instruction, eInstructionType type);
    void SetOperand(std::string value, eOperandType type);
    void SetOperand(ParserOperand& operand);
    void SetMultipleOperands(std::vector<ParserOperand> operands);
    
    void FinishInstruction();
    void ContinueParsing();

    void PrintProgram(std::string outFile);
private:

    void AnalyzeInstruction();

    eOperandType GetInstructionOperandType() const;

    AssemblyInstruction::s_ptr m_currentInstruction;
    Section::s_ptr m_currentSection;
    std::vector<AssemblyInstruction::s_ptr> m_program;

    bool m_end; // .END encountered
    uint32_t m_locationCounter;
};

#endif