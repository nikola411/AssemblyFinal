#ifndef INSTRUCTION_MAPPING_HPP
#define INSTRUCTION_MAPPING_HPP

#include "Instruction.hpp"
#include "AssemblyLine.hpp"
#include "ErrorHandling.hpp"

#include <map>
#include <set>
#include <tuple>
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

    using AssemblyLineMetadata = std::vector<ProcessorInstructionMetadata>;
    using InstructionKey = std::tuple<eAssemblyIdentifier, eAddressingType, ePayloadType>;
    using InstructionMap = std::map<InstructionKey, AssemblyLineMetadata>;

    using BigValueInstructions = std::set<eAssemblyIdentifier>;

    struct PayloadTypeEntry
    {
        ePayloadType type;
        std::vector<ProcessorInstructionMetadata> processorInstructions;

        PayloadTypeEntry(ePayloadType type, std::vector<ProcessorInstructionMetadata> processorInstructions) : type(type), processorInstructions(processorInstructions) {}
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

    static void InstructionEntry(eAssemblyIdentifier instruction, std::vector<AddressingEntry> entries);
    static void NONE_Instruction(eAssemblyIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions);
    static void GPR_Instruction(eAssemblyIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions);
    static void VALUE_Instruction(eAssemblyIdentifier instruction, eAddressingType type, eValueSize size, std::vector<ProcessorInstructionMetadata> processorInstructions);
    static void CSR_Instruction(eAssemblyIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions);
    static void PopulateSpecial();

    // public functions of this interface
    void PopulateInstructionsMap();
    AssemblyLineMetadata GetProcessorInstructions(const AssemblyLine::s_ptr& instruction);
    ePayloadType OperandTypeToPayloadType(eOperandType type);
    uint16_t GetOperandValue(const AssemblyLine::s_ptr& instruction, ValueToUseMasks enumerator);

}


#endif