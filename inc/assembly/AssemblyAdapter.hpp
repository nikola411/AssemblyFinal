#ifndef ASSEMBLY_ADAPTER_HPP
#define ASSEMBLY_ADAPTER_HPP

#include "Assembly.hpp"

class AssemblyAdapter
{
public:
    void SetInstruction(eAssemblyIdentifier instruction, eAssemblyIdentifierType type);
    void SetOperand(std::string value, eOperandType type);
    void SetOperand(ParserOperand& operand);
    void SetMultipleOperands(std::vector<ParserOperand> operands);
    void FinishInstruction();

    void Backpatch();

    void GenerateOutput(const std::string& output) const;
private:
    Assembly assembly;
};

#endif