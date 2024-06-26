#ifndef ASSEMBLY_INSTRUCTION_HPP
#define ASSEMBLY_INSTRUCTION_HPP

#include "Instruction.hpp"
#include "ErrorHandling.hpp"

struct AssemblyInstruction
{
    AssemblyInstruction() = default;
    ~AssemblyInstruction() = default;

    virtual eOperandType GetOperandType() const;
    virtual eAddressingType GetAddressingType() const;

    virtual int GetVariableOperandIndex() const;

    using s_ptr = std::shared_ptr<AssemblyInstruction>;

    eAddressingType addressing;
    eInstructionType type;
    eInstructionIdentifier identifier;
    std::vector<ParserOperand> operands;
};

#endif