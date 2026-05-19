#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

#include "Utility.hpp"
#include "AssemblyLine.hpp"
#include "ErrorHandling.hpp"

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
// virtual for the sake of testing
    virtual void DecodeInstructionValues();
    virtual void CalculateOperandValue(ParserOperand& operand);
    virtual void CalculateOperandOffset(ParserOperand& operand);

    virtual void WriteInstructionToSection(const AssemblyLine::s_ptr& instruction);

    virtual void GenerateRelocation(const eRelocationType& type, const std::string& name = "");
    virtual void GenerateRelocation(const eRelocationType& type, const std::string& name, const int& offset, const std::string& section);

    virtual void GenerateForwardReference(const std::string& name);

    virtual uint16_t GetSymbolValue(const ParserOperand& operand);
    virtual uint16_t GetLiteralValue(const ParserOperand& operand);
    void inline IncrementLocationCounter(uint32_t amount);

    AssemblyLine::s_ptr mCurrentInstruction;
    Section::s_ptr mCurrentSection;

    SectionTable mSectionTable;
    SymbolTable mSymbolTable;
    RelocationTable mRelocationTable;
    ForwardRefferenceTable mForwardRefTable;

    std::vector<AssemblyLine::s_ptr> mProgram;
    bool mEnd; // .END encountered
};

#endif