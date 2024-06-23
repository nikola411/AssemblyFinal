#ifndef ASSEMBLY_INSTRUCTION_HPP
#define ASSEMBLY_INSTRUCTION_HPP

#include "Instruction.hpp"
#include "Codes.hpp"
#include "ErrorHandling.hpp"

struct AssemblyInstruction
{
    AssemblyInstruction() = default;
    ~AssemblyInstruction() = default;

    eOperandType GetOperandType() const;
    eAddressingType GetAddressingType() const;

    using s_ptr = std::shared_ptr<AssemblyInstruction>;

    eAddressingType addressing;
    eInstructionType type;
    eInstructionIdentifier identifier;
    std::vector<ParserOperand> operands;

    

    std::vector<InstructionPopulationMetadata> metadata;
};

#endif