#include "AssemblyInstruction.hpp"
#include "Utility.hpp"

#include <set>

static std::set<eInstructionType> operandInstructions =
{
    MEMORY, BRANCH
};

static std::set<eInstructionType> registerInstructions =
{
    PROCESSOR, STACK, DATA, SPECIAL,
};

eOperandType AssemblyInstruction::GetOperandType() const
{
    switch (identifier)
    {
        case eInstructionIdentifier::CALL:
        case eInstructionIdentifier::JMP:
            return operands.front().type;
        case eInstructionIdentifier::BEQ:
        case eInstructionIdentifier::BNE:
        case eInstructionIdentifier::BGT:
            return operands.back().type;
        case eInstructionIdentifier::LD:
            return operands.front().type;
        case eInstructionIdentifier::ST:
            return operands.back().type;
        case eInstructionIdentifier::CSRRD:
        case eInstructionIdentifier::CSRWR:
            return operands.front().type;
        case eInstructionIdentifier::HALT:
        case eInstructionIdentifier::INT:
        case eInstructionIdentifier::IRET:
        case eInstructionIdentifier::RET:
            return eOperandType::NONE_TYPE;
        default:
            return eOperandType::GPR;
    }

    return eOperandType::NONE_TYPE;
}

eAddressingType AssemblyInstruction::GetAddressingType() const
{
    switch (identifier)
    {
        case eInstructionIdentifier::CALL:
        case eInstructionIdentifier::JMP:
            return operands.front().addressingType;
        case eInstructionIdentifier::BEQ:
        case eInstructionIdentifier::BNE:
        case eInstructionIdentifier::BGT:
            return operands.back().addressingType;
        case eInstructionIdentifier::LD:
            return operands.front().addressingType;
        case eInstructionIdentifier::ST:
            return operands.back().addressingType;
        case eInstructionIdentifier::CSRRD:
        case eInstructionIdentifier::CSRWR:
            return operands.front().addressingType;
        case eInstructionIdentifier::HALT:
        case eInstructionIdentifier::INT:
        case eInstructionIdentifier::IRET:
        case eInstructionIdentifier::RET:
            return eAddressingType::ADDR_NONE;
        default:
            return eAddressingType::ADDR_DIRECT;
    }

    return eAddressingType::ADDR_NONE;
}

int AssemblyInstruction::GetVariableOperandIndex() const
{
    switch (identifier)
    {
        case eInstructionIdentifier::CALL:
        case eInstructionIdentifier::JMP:
            return 0;
        case eInstructionIdentifier::BEQ:
        case eInstructionIdentifier::BNE:
        case eInstructionIdentifier::BGT:
            return 2;
        case eInstructionIdentifier::LD:
            return 0;
        case eInstructionIdentifier::ST:
            return 1;
    }

    return -1;
}
