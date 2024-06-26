#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Assembly.hpp"

#include <memory>

#include "Assembly.hpp"
#include "AssemblyInstruction.hpp"

class MockInstruction : public AssemblyInstruction
{
public:
    MockInstruction() = default;
    ~MockInstruction() = default;

    MOCK_METHOD(eOperandType, GetOperandType, ());
    MOCK_METHOD(eAddressingType, GetAddressingType, ());
    MOCK_METHOD(void, GetVariableOperandIndex, ());

    using s_ptr = std::shared_ptr<AssemblyInstruction>;

    eAddressingType addressing;
    eInstructionType type;
    eInstructionIdentifier identifier;
    std::vector<ParserOperand> operands;
};

TEST(AssemblyTests, HandleLabelThrowAssebmlyExceptionOnDefinedSymbol)
{
    Assembly assembly;

    MockInstruction::s_ptr instruction = std::make_shared<MockInstruction>();

    ParserOperand operand;
    operand.value = "my_label:";
    instruction->operands.push_back(operand);
    instruction->type = eInstructionType::LABEL;
    
    assembly.mCurrentInstruction = instruction;
    assembly.mSymbolTable.push_back(std::make_shared<Symbol>("my_label", "section1", 0, 0, true, true, true, true));
    
    ASSERT_THROW(assembly.HandleLabel(), AssemblyException);
}

TEST(AssemblyTests, ConstructorShouldNotThrow)
{
    ASSERT_NO_THROW(new Assembly());
}

TEST(AssemblyTests, DecodeInstructionValuesShouldNotThrow)
{
    MockInstruction::s_ptr instruction = std::make_shared<MockInstruction>();
    instruction->addressing = ADDR_NONE;
    instruction->identifier = eInstructionIdentifier::HALT;
    instruction->type = eInstructionType::PROCESSOR;

    Assembly assembly;
    assembly.mCurrentInstruction = instruction;

    ASSERT_NO_THROW(assembly.DecodeInstructionValues());
}

// go from singular, independet methods

TEST(AssemblyTests, GetLiteralValueForSmallLiteralShouldThrow)
{
    Assembly assembly;
    assembly.mCurrentInstruction = std::make_shared<AssemblyInstruction>();

    ParserOperand operand;
    operand.value = "0x8888";

    EXPECT_THROW(assembly.GetLiteralValue(operand), AssemblyException);
}

TEST(AssemblyTests, GetLiteralValueForSmallLiteralShouldReturnRightValue)
{
    Assembly assembly;
    assembly.mCurrentInstruction = std::make_shared<AssemblyInstruction>();
    assembly.mCurrentInstruction->identifier = eInstructionIdentifier::ADD;
    assembly.mCurrentInstruction->addressing = eAddressingType::ADDR_DIRECT;

    ParserOperand operand;
    operand.value = "0x688";

    EXPECT_EQ(assembly.GetLiteralValue(operand), 0x688);
}

TEST(AssemblyTests, GetLiteralValueForBigLiteralShouldReturnPoolEntry)
{
    Assembly assembly;
    assembly.mCurrentInstruction = std::make_shared<AssemblyInstruction>();
    assembly.mCurrentInstruction->identifier = eInstructionIdentifier::LD;
    assembly.mCurrentInstruction->addressing = eAddressingType::ADDR_DIRECT;

    ParserOperand operand;
    operand.value = "0x77777";

    EXPECT_NE(assembly.GetLiteralValue(operand), 0x77777);
}

TEST(AssemblyTests, GetLiteralValueForBigLiteralShouldReturnRightPoolEntry)
{
    Assembly assembly;
    assembly.mCurrentInstruction = std::make_shared<AssemblyInstruction>();
    assembly.mCurrentInstruction->identifier = eInstructionIdentifier::LD;
    assembly.mCurrentInstruction->addressing = eAddressingType::ADDR_DIRECT;

    ParserOperand operand;
    operand.value = "0x77777";

    EXPECT_EQ(assembly.GetLiteralValue(operand), 0);
    EXPECT_EQ(assembly.GetLiteralValue(operand), 0); //second time should check if literal is present in pool
}

TEST(AssemblyTests, GetSymbolValueShouldGenerateForwardReference)
{
    class MockAssembly : public Assembly
    {
    public:
        MOCK_METHOD(void, GenerateForwardRefference, (const std::string&), (override));
    };

    MockAssembly assembly;
    EXPECT_CALL(assembly, GenerateForwardRefference);

    ParserOperand operand;
    operand.value = "test_symbol";
    
    EXPECT_EQ(assembly.GetSymbolValue(operand), 0);
}

TEST(AssemblyTests, GetSymbolValueShouldGenerateARelocation)
{
    class MockAssembly : public Assembly
    {
    public:
        MOCK_METHOD(void, GenerateRelocation, (const eRelocationType&, const std::string&), (override));
    };

    MockAssembly assembly;
    
    Symbol::s_ptr symbol = std::make_shared<Symbol>("test_symbol", "test_section_1", 0, 0);
    assembly.mSymbolTable.push_back(symbol);

    ParserOperand operand;
    operand.value = "test_symbol";

    symbol->isExtern = true;
    EXPECT_CALL(assembly, GenerateRelocation);
    EXPECT_EQ(assembly.GetSymbolValue(operand), 0);

    symbol->isExtern = false;
    symbol->defined = false;
    EXPECT_CALL(assembly, GenerateRelocation);
    EXPECT_EQ(assembly.GetSymbolValue(operand), 0);

    symbol->isExtern = false;
    symbol->defined = true;
    symbol->section = "test_section_2";
    EXPECT_CALL(assembly, GenerateRelocation);
    EXPECT_EQ(assembly.GetSymbolValue(operand), 0);
}

TEST(AssemblyTests, GetSymbolValueShouldReturnSymbolOffsetForInstructionWithSmallValues)
{
    class MockAssembly : public Assembly
    {
    };

    MockAssembly assembly;
    assembly.mCurrentInstruction = std::make_shared<AssemblyInstruction>();
    assembly.mCurrentInstruction->identifier = eInstructionIdentifier::LD;
    assembly.mCurrentInstruction->addressing = eAddressingType::ADDR_MEMORY;

    assembly.mCurrentSection = std::make_shared<Section>();
    assembly.mCurrentSection->name = "test_section_1";

    Symbol::s_ptr symbol = std::make_shared<Symbol>("test_symbol", "test_section_1", 16*5, 16*5, true, true, false, false);
    assembly.mSymbolTable.push_back(symbol);

    ParserOperand operand;
    operand.value = "test_symbol";

    EXPECT_EQ(assembly.GetSymbolValue(operand), 16*5);
}

TEST(AssemblyTests, GetSymbolValueShouldThrowWhenInstructionDoesNotSupportBigValues)
{
    class MockAssembly : public Assembly
    {
    };

    MockAssembly assembly;
    assembly.mCurrentInstruction = std::make_shared<AssemblyInstruction>();
    assembly.mCurrentInstruction->identifier = eInstructionIdentifier::LD;
    assembly.mCurrentInstruction->addressing = eAddressingType::ADDR_MEMORY;

    assembly.mCurrentSection = std::make_shared<Section>();
    assembly.mCurrentSection->name = "test_section_1";

    Symbol::s_ptr symbol = std::make_shared<Symbol>("test_symbol", "test_section_1", 0x1999, 0x1999, true, true, false, false);
    assembly.mSymbolTable.push_back(symbol);

    ParserOperand operand;
    operand.value = "test_symbol";

    EXPECT_THROW(assembly.GetSymbolValue(operand), AssemblyException);
}

TEST(AssemblyTests, GetSymbolValueShouldReturnPoolEntryForBigValueSymbols)
{
    class MockAssembly : public Assembly
    {
    };

    MockAssembly assembly;
    assembly.mCurrentInstruction = std::make_shared<AssemblyInstruction>();
    assembly.mCurrentInstruction->identifier = eInstructionIdentifier::LD;
    assembly.mCurrentInstruction->addressing = eAddressingType::ADDR_DIRECT;

    assembly.mCurrentSection = std::make_shared<Section>();
    assembly.mCurrentSection->name = "test_section_1";

    Symbol::s_ptr symbol = std::make_shared<Symbol>("test_symbol", "test_section_1", 0x1999, 0x1999, true, true, false, false);
    assembly.mSymbolTable.push_back(symbol);

    ParserOperand operand;
    operand.value = "test_symbol";

    EXPECT_EQ(assembly.GetSymbolValue(operand), 0);
}