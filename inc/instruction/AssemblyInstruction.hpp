#ifndef ASSEMBLY_INSTRUCTION_HPP
#define ASSEMBLY_INSTRUCTION_HPP

#include "Instruction.hpp"

struct AssemblyInstruction : public Instruction
{
    AssemblyInstruction(){};
    ~AssemblyInstruction() = default;

    uint32_t GetData() const override;

    eAddressingType addressing;
    eInstructionType type;
    eInstructionIdentifier identifier;
    std::vector<ParserOperand> operands;

    typedef std::shared_ptr<AssemblyInstruction> s_ptr;
};

#endif