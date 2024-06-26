#include "AssemblyAdapter.hpp"

void AssemblyAdapter::SetInstruction(eInstructionIdentifier instruction, eInstructionType type)
{
    assembly.mCurrentInstruction = std::make_shared<AssemblyInstruction>();
    assembly.mCurrentInstruction->identifier = instruction;
    assembly.mCurrentInstruction->type = type;
}

void AssemblyAdapter::SetOperand(std::string value, eOperandType type)
{
    assembly.mCurrentInstruction->operands.push_back(ParserOperand(value, type));
}

void AssemblyAdapter::SetOperand(ParserOperand& operand)
{
    assembly.mCurrentInstruction->operands.push_back(operand);
}

void AssemblyAdapter::SetMultipleOperands(std::vector<ParserOperand> operands)
{
    for (auto op : operands)
    {
        assembly.mCurrentInstruction->operands.push_back(op);
    }
}

void AssemblyAdapter::FinishInstruction()
{
    if (assembly.mEnd)
        return;

    switch(assembly.mCurrentInstruction->type)
    {
        case eInstructionType::LABEL:
        {
            assembly.HandleLabel();
            break;
        }
        case eInstructionType::DIRECTIVE:
        {
            assembly.HandleDirective();
            break;
        }
        default:
        {
            assembly.HandleInstruction();
            break;
        }
    }   

    assembly.mProgram.push_back(assembly.mCurrentInstruction);
    return;
}

void AssemblyAdapter::Backpatch()
{
    // time to backref bois
    assembly.ContinueParsing();
}

#include <fstream>

void AssemblyAdapter::GenerateOutput(const std::string& output) const
{
    std::ofstream out;
    out.open(output, std::ios_base::out);
    if (!out.is_open())
        throw AssemblyException(ExceptionMessage::OutputFileNotOpen, { output });

    auto symbolTableString = SymbolTableToString(assembly.mSymbolTable);
    out.write(symbolTableString.c_str(), symbolTableString.size());

    auto sectionsString = SectionTableToString(assembly.mSectionTable);
    out.write(sectionsString.c_str(), sectionsString.size());

    auto relocationsString = RelocationTableToString(assembly.mRelocationTable);
    out.write(relocationsString.c_str(), relocationsString.size());

    out.close();
}
