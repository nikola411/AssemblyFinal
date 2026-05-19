#ifndef ASSEMBLY_LINE_HPP
#define ASSEMBLY_LINE_HPP

#include "Instruction.hpp"
#include "ErrorHandling.hpp"

struct AssemblyLine
{
    AssemblyLine() = default;
    ~AssemblyLine() = default;

    virtual eOperandType GetOperandType() const;
    virtual eAddressingType GetAddressingType() const;

    virtual int GetVariableOperandIndex() const;

    using s_ptr = std::shared_ptr<AssemblyLine>;

    eAddressingType addressing;
    eAssemblyIdentifierType type;
    eAssemblyIdentifier identifier;
    std::vector<ParserOperand> operands;
};

#endif