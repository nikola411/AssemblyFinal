#include "InstructionMapping.hpp"

void InstructionEntry(eInstructionIdentifier instruction, std::vector<AddressingEntry> entries)
{
    for (const auto& addr : entries)
    {
        for (const auto& payload : addr.payloadEntry)
        {
            for (const auto& size : payload.sizes)
            {
                methods[instruction][addr.type][payload.type][size.value] = size.processorInstructions;
            }
        }
    }
}

void NONE_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods
        [instruction]
        [eAddressingType::ADDR_NONE]
        [ePayloadType::PAYLOAD_NONE]
        [eValueSize::SIZE_NONE]
        = processorInstructions;
}

void GPR_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods
        [instruction]
        [eAddressingType::ADDR_DIRECT]
        [ePayloadType::PAYLOAD_GPR]
        [eValueSize::SIZE_NONE]
        = processorInstructions;
}

void VALUE_Instruction(eInstructionIdentifier instruction, eAddressingType type, eValueSize size, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods
        [instruction]
        [type]
        [ePayloadType::PAYLOAD_VALUE]
        [size]
        = processorInstructions;
}

void CSR_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods
        [instruction]
        [eAddressingType::ADDR_DIRECT]
        [ePayloadType::PAYLOAD_CSR]
        [eValueSize::SIZE_NONE]
        = processorInstructions;
}