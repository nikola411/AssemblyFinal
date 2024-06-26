#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

#include "Utility.hpp"
#include "AssemblyInstruction.hpp"
#include "ErrorHandling.hpp"

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
/// @brief Used for resloving backreferences
    void ContinueParsing();

/// @brief Used for printing generated ASM structures
/// @param outFile - path to output file.
    void PrintProgram(std::string outFile);
    // labels and directives
    void CheckForErrors() const;

    void HandleLabel();
    void HandleDirective();
    void HandleInstruction();
// virtual for the sake of testing
    virtual void DecodeInstructionValues();
    virtual void CalculateOperandValue(ParserOperand& operand);
    virtual void CalculateOperandOffset(ParserOperand& operand);

    virtual void WriteInstructionToSection(const AssemblyInstruction::s_ptr& instruction);

    virtual void GenerateRelocation(const eRelocationType& type, const std::string& name = "");
    virtual void GenerateForwardRefference(const std::string& name);

    virtual uint16_t GetSymbolValue(const ParserOperand& operand);
    virtual uint16_t GetLiteralValue(const ParserOperand& operand);
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