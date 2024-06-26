#ifndef ASSEMBLY_ADAPTER_HPP
#define ASSEMBLY_ADAPTER_HPP

#include "Assembly.hpp"

class AssemblyAdapter
{
public:
    void SetInstruction(eInstructionIdentifier instruction, eInstructionType type);
    void SetOperand(std::string value, eOperandType type);
    void SetOperand(ParserOperand& operand);
    void SetMultipleOperands(std::vector<ParserOperand> operands);
    void FinishInstruction();
/// @brief Used for resloving backreferences
    void Backpatch();

    void GenerateOutput(const std::string& output) const;
private:
    Assembly assembly;
};

#endif