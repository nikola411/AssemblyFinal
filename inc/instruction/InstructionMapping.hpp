#ifndef INSTRUCTION_MAPPING_HPP
#define INSTRUCTION_MAPPING_HPP

#include "Instruction.hpp"
#include "AssemblyInstruction.hpp"
#include "ErrorHandling.hpp"

#include <map>
#include <set>
#include <vector>

namespace Conversion
{
    enum ePayloadType
    {
        PAYLOAD_GPR,
        PAYLOAD_CSR,
        PAYLOAD_VALUE,
        PAYLOAD_NONE
    };

    enum eValueSize
    {
        SIZE_BIG,
        SIZE_SMALL,
        SIZE_NONE
    };

    enum eValueToUse
    {
        FIRST_OPERAND = 0x1 << 0,
        SECOND_OPERAND = 0x1 << 1,
        THIRD_OPERAND = 0x1 << 2,
        FIRST_OFFSET = 0x1 << 3,
        SECOND_OFFSET = 0x1 << 4,
        THIRD_OFFSET = 0x1 << 5,
        NONE = 0x1 << 6,
        POOL_ENTRY = 0x1 << 7
    };

    #define MANIPULATION_PAIR(operand, method) { ValueToUseMasks::operand, &Instruction::method } 
    #define NO_MANIPULATION {}

    using ValueToUseMasks = int;
    using InstructionMethod = void (Instruction::*)(uint16_t);
    using ManipulationPair = std::pair<ValueToUseMasks, InstructionMethod>;
    using ProcessorInstructionMetadata = std::pair<Instruction, std::vector<ManipulationPair>>;

    using AssemblyInstructionMetadata = std::vector<ProcessorInstructionMetadata>;
    using ValueSizeMap = std::map<eValueSize, AssemblyInstructionMetadata>;
    using PayloadTypeMap = std::map<ePayloadType, ValueSizeMap>;
    using AddressingMap = std::map<eAddressingType, PayloadTypeMap>;
    using InstructionMap = std::map<eInstructionIdentifier, AddressingMap>;

    using BigValueInstructions = std::set<eInstructionIdentifier>;

    struct SizeEntry
    {
        eValueSize value;
        std::vector<ProcessorInstructionMetadata> processorInstructions;

        SizeEntry(eValueSize value, std::vector<ProcessorInstructionMetadata> processorInstructions = {}) : value(value), processorInstructions(processorInstructions) {}
    };

    struct PayloadTypeEntry
    {
        ePayloadType type;
        std::vector<SizeEntry> sizes;

        PayloadTypeEntry(ePayloadType type, std::vector<SizeEntry> sizes = {}) : type(type), sizes(sizes) {}
    };

    struct AddressingEntry
    {
        eAddressingType type;
        std::vector<PayloadTypeEntry> payloadEntry;

        AddressingEntry(eAddressingType type, std::vector<PayloadTypeEntry> payloads = {}) : type(type), payloadEntry(payloads) {}
    };

    // Private data of this interface
    static InstructionMap methods;
    static BigValueInstructions bigValueInstructions;

    // Private functions of this interface
    static inline std::pair<Instruction, std::vector<ManipulationPair>> ProcessorInstruction(uint32_t code, std::vector<ManipulationPair> data)
    {
        return { Instruction(code), data };
    }

    static inline std::vector<std::pair<Instruction, std::vector<ManipulationPair>>> SingleProcessorInstruction(uint32_t code, std::vector<ManipulationPair> data)
    {
        return { { Instruction(code), data } };
    }

    static inline std::vector<ManipulationPair> SingleManipulationEntry(const ValueToUseMasks valueToUse, const InstructionMethod method)
    {
        return { { valueToUse, method } };
    }

    static ManipulationPair ManipulationEntry(ValueToUseMasks valueToUse, InstructionMethod method)
    {
        return { valueToUse, method };
    }

    static void InstructionEntry(eInstructionIdentifier instruction, std::vector<AddressingEntry> entries);
    static void NONE_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions);
    static void GPR_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions);
    static void VALUE_Instruction(eInstructionIdentifier instruction, eAddressingType type, eValueSize size, std::vector<ProcessorInstructionMetadata> processorInstructions);
    static void CSR_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions);
    static void PopulateSpecial();
    
    static eValueSize GetOperandValueSize(const AssemblyInstruction::s_ptr& instruction);

    // public functions of this interface
    void PopulateInstructionsMap();
    AssemblyInstructionMetadata GetProcessorInstructions(const AssemblyInstruction::s_ptr& instruction);
    ePayloadType OperandTypeToPayloadType(eOperandType type);
    uint16_t GetOperandValue(const AssemblyInstruction::s_ptr& instruction, ValueToUseMasks enumerator);

    bool IsBigValueInstruction(const eInstructionIdentifier& instruction, const eAddressingType& addressing, const ePayloadType& payloadType);
}


#endif