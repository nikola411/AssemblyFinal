#include "AssemblyLine.hpp"
#include "Utility.hpp"

#include <set>

static std::set<eAssemblyIdentifierType> operandInstructions =
{
    MEMORY, BRANCH
};

static std::set<eAssemblyIdentifierType> registerInstructions =
{
    PROCESSOR, STACK, DATA, SPECIAL,
};

eOperandType AssemblyLine::GetOperandType() const
{
    switch (identifier)
    {
        case eAssemblyIdentifier::CALL:
        case eAssemblyIdentifier::JMP:
            return operands.front().type;
        case eAssemblyIdentifier::BEQ:
        case eAssemblyIdentifier::BNE:
        case eAssemblyIdentifier::BGT:
            return operands.back().type;
        case eAssemblyIdentifier::LD:
            return operands.front().type;
        case eAssemblyIdentifier::ST:
            return operands.back().type;
        case eAssemblyIdentifier::CSRRD:
        case eAssemblyIdentifier::CSRWR:
            return operands.front().type;
        case eAssemblyIdentifier::HALT:
        case eAssemblyIdentifier::INT:
        case eAssemblyIdentifier::IRET:
        case eAssemblyIdentifier::RET:
            return eOperandType::NONE_TYPE;
        default:
            return eOperandType::GPR;
    }

    return eOperandType::NONE_TYPE;
}

eAddressingType AssemblyLine::GetAddressingType() const
{
    switch (identifier)
    {
        case eAssemblyIdentifier::CALL:
        case eAssemblyIdentifier::JMP:
            return operands.front().addressingType;
        case eAssemblyIdentifier::BEQ:
        case eAssemblyIdentifier::BNE:
        case eAssemblyIdentifier::BGT:
            return operands.back().addressingType;
        case eAssemblyIdentifier::LD:
            return operands.front().addressingType;
        case eAssemblyIdentifier::ST:
            return operands.back().addressingType;
        case eAssemblyIdentifier::CSRRD:
        case eAssemblyIdentifier::CSRWR:
            return operands.front().addressingType;
        case eAssemblyIdentifier::HALT:
        case eAssemblyIdentifier::INT:
        case eAssemblyIdentifier::IRET:
        case eAssemblyIdentifier::RET:
            return eAddressingType::ADDR_NONE;
        default:
            return eAddressingType::ADDR_DIRECT;
    }

    return eAddressingType::ADDR_NONE;
}

int AssemblyLine::GetVariableOperandIndex() const
{
    switch (identifier)
    {
        case eAssemblyIdentifier::CALL:
        case eAssemblyIdentifier::JMP:
            return 0;
        case eAssemblyIdentifier::BEQ:
        case eAssemblyIdentifier::BNE:
        case eAssemblyIdentifier::BGT:
            return 2;
        case eAssemblyIdentifier::LD:
            return 0;
        case eAssemblyIdentifier::ST:
            return 1;
    }

    return -1;
}
