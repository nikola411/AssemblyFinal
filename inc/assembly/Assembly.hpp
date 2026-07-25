#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

#include "Utility.hpp"
#include "AssemblyLine.hpp"
#include "ErrorHandling.hpp"
#include "Symbol.hpp"
#include "Section.hpp"

#include <vector>
#include <memory>
#include <stdint.h>

class Assembly
{
public:
    Assembly();
    ~Assembly() {};

    void ContinueParsing();
    void PrintProgram(std::string outFile);
    // labels and directives
    void CheckForErrors() const;

    void HandleLabel();
    void HandleDirective();
    void HandleInstruction();
    void HandleDataInstruction();
    void HandleBranchInstruction();
// virtual for the sake of testing
    virtual void CalculateOperandValue(ParserOperand& operand);
    virtual void CalculateDataOperandOffset(ParserOperand& operand);

    virtual void WriteInstructionToSection(const AssemblyLine::s_ptr& instruction);

    virtual void GenerateForwardReference(const std::string& name, const ForwardRefferenceType& type);

    virtual uint32_t GetSymbolValue(const std::string& name);
    virtual uint16_t GetSymbolPoolEntry(const Symbol::s_ptr& symbol);
    virtual uint32_t GetAbsoluteSymbolValue(const Symbol::s_ptr& symbol);
    virtual uint32_t GetConstantSymbolValue(const Symbol::s_ptr& symbol);

    virtual uint16_t GetLiteralValue(ParserOperand& operand);
    void inline IncrementLocationCounter(uint32_t amount);

    AssemblyLine::s_ptr mCurrentInstruction;
    Section::s_ptr mCurrentSection;

    SectionTable mSectionTable;
    SymbolTable mSymbolTable;
    ForwardRefferenceTable mForwardRefTable;

    std::vector<AssemblyLine::s_ptr> mProgram;
    bool mEnd; // .END encountered
};

#endif