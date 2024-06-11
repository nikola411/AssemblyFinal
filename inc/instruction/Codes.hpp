#pragma once

#include "Instruction.hpp"

#include <map>

enum eValueToUse
{
    FIRST_OPERAND = 0x00, SECOND_OPERAND = 0x01, THIRD_OPERAND = 0x02,
    FIRST_OFFSET = 0x03, SECOND_OFFSET = 0x04, THIRD_OFFSET = 0x05
};

typedef void (Instruction::* ptrInstructionMethod)(uint16_t);

struct CodePopulation
{
    ptrInstructionMethod method;
    eValueToUse operand;

    CodePopulation(ptrInstructionMethod method, eValueToUse operand):
        method(method), operand(operand) {}
    
    typedef std::shared_ptr<CodePopulation> s_ptr;
};

typedef std::pair<Instruction, std::vector<CodePopulation::s_ptr>> InstructionPopulationMetadata;

class CodesMap
{
public:
    static void PopulateMap();
    static int GetInstructionCount(eInstructionIdentifier identifier, eOperandType operand, eAddressingType addressing);
    static std::vector<InstructionPopulationMetadata> GetInstructionCodes(eInstructionIdentifier identifier, eOperandType operand, eAddressingType addressing);
    static inline bool IsInitialized() { return Initialized; } 
private:
    static void AddInstructionPair(int code, std::vector<std::pair<ptrInstructionMethod, eValueToUse>> methods);
    static void AddInstructionList(std::vector<InstructionPopulationMetadata>& instructions, std::vector<std::pair<int, std::vector<std::pair<ptrInstructionMethod, eValueToUse>>>> pairsVector);
    static void SetMapEntry(eInstructionIdentifier identifier, eOperandType operand, eAddressingType addressing);
    static void AddMapEntry();

    static bool Initialized;

    static eInstructionIdentifier currentIdentifier;
    static eOperandType currentOperandType;
    static eAddressingType currentAddressingType;

    static std::vector<InstructionPopulationMetadata> currentEntry;

    static std::map<eInstructionIdentifier, std::map<eOperandType, std::map<eAddressingType, std::vector<InstructionPopulationMetadata>>>> InstructionCodesMap;
};