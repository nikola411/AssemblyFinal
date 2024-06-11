#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

#include "Utility.hpp"
#include "AssemblyInstruction.hpp"

#include <vector>
#include <memory>
#include <stdint.h>

class Assembly
{
public:
    Assembly();
    ~Assembly() {};

/// @brief Parser API
/// Used in next order: SetInstruction, [SetOperand, SetOperand, SetMultipleOperands...], FinishInstruction
    void SetInstruction(eInstructionIdentifier instruction, eInstructionType type);
    void SetOperand(std::string value, eOperandType type);
    void SetOperand(ParserOperand& operand);
    void SetMultipleOperands(std::vector<ParserOperand> operands);
    AsmResult FinishInstruction();

/// @brief Used for resloving backreferences
    void ContinueParsing();

/// @brief Used for printing generated ASM structures
/// @param outFile - path to output file.
    void PrintProgram(std::string outFile);
private:
    // labels and directives
    AsmResult HandleLabel();
    AsmResult HandleDirective();
    AsmResult HandleInstruction();

    AsmResult CalculateOperandsValue();
    AsmResult WriteInstructionToSection(const AssemblyInstruction::s_ptr& instruction);

    uint16_t GetSymbolValue(const std::string& name);
    uint16_t GetLiteralValue(const std::string& literal);
    void inline IncrementLocationCounter(uint32_t amount);

    AssemblyInstruction::s_ptr mCurrentInstruction;
    Section::s_ptr mCurrentSection;

    SectionTable mSectionTable;
    SymbolTable mSymbolTable;
    RelocationTable mRelocationTable;
    ForwardRefferenceTable mForwardRefTable;

    std::vector<AssemblyInstruction::s_ptr> mProgram;

    bool mEnd; // .END encountered
};

#endif