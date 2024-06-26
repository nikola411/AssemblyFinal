#include "InstructionMapping.hpp"

using namespace Conversion;

void Conversion::InstructionEntry(eInstructionIdentifier instruction, std::vector<AddressingEntry> entries)
{
    for (const auto& addr : entries)
    {
        for (const auto& payload : addr.payloadEntry)
        {
            for (const auto& size : payload.sizes)
            {
                if (size.value == eValueSize::SIZE_BIG)
                    bigValueInstructions.insert(instruction);

                methods[instruction][addr.type][payload.type][size.value] = size.processorInstructions;
            }
        }
    }
}

void Conversion::NONE_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods
        [instruction]
        [eAddressingType::ADDR_NONE]
        [ePayloadType::PAYLOAD_NONE]
        [eValueSize::SIZE_NONE]
        = processorInstructions;
}

void Conversion::GPR_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods
        [instruction]
        [eAddressingType::ADDR_DIRECT]
        [ePayloadType::PAYLOAD_GPR]
        [eValueSize::SIZE_NONE]
        = processorInstructions;
}

void Conversion::VALUE_Instruction(eInstructionIdentifier instruction, eAddressingType type, eValueSize size, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods
        [instruction]
        [type]
        [ePayloadType::PAYLOAD_VALUE]
        [size]
        = processorInstructions;
}

void Conversion::CSR_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods
        [instruction]
        [eAddressingType::ADDR_DIRECT]
        [ePayloadType::PAYLOAD_CSR]
        [eValueSize::SIZE_NONE]
        = processorInstructions;
}

void Conversion::PopulateSpecial()
{
    InstructionEntry(LD,
    {
        AddressingEntry(ADDR_DIRECT,
        {
            PayloadTypeEntry(PAYLOAD_GPR,
            {
                SizeEntry(SIZE_NONE,
                {
                    ProcessorInstruction(0x91000000,
                    {
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB)
                    }) 
                })
            }),
            PayloadTypeEntry(PAYLOAD_VALUE,
            {
                SizeEntry(SIZE_BIG,
                {
                    ProcessorInstruction(0x920F0000,
                    {
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                        ManipulationEntry(POOL_ENTRY | FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x91000000,
                    {
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                })
            })
        }),
        AddressingEntry(ADDR_MEMORY,
        {
            PayloadTypeEntry(PAYLOAD_GPR,
            {
                SizeEntry(SIZE_NONE,
                {
                    ProcessorInstruction(0x92000000,
                    {
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB)
                    })
                })
            }),
            PayloadTypeEntry(PAYLOAD_VALUE,
            {
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x92000000,
                    {
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                })
            }),
        }),
        AddressingEntry(ADDR_MEMORY_OFFSET,
        {
            PayloadTypeEntry(PAYLOAD_GPR,
            {
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x92000000,
                    {
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                        ManipulationEntry(FIRST_OFFSET, &Instruction::SetDisplacement)
                    })
                }),
            })
        })
    });

    InstructionEntry(ST,
    {
        AddressingEntry(ADDR_DIRECT,
        {
            PayloadTypeEntry(PAYLOAD_GPR,
            {
                SizeEntry(SIZE_NONE,
                {
                    ProcessorInstruction(0x80000000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterB)
                    }) 
                })
            }),
            PayloadTypeEntry(PAYLOAD_VALUE,
            {
                SizeEntry(SIZE_BIG,
                {
                    ProcessorInstruction(0x81F00000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(POOL_ENTRY | SECOND_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x80000000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetDisplacement)
                    })
                })
            })
        }),
        AddressingEntry(ADDR_MEMORY,
        {
            PayloadTypeEntry(PAYLOAD_GPR,
            {
                SizeEntry(SIZE_NONE,
                {
                    ProcessorInstruction(0x81000000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA)
                    })
                })
            }),
            PayloadTypeEntry(PAYLOAD_VALUE,
            {
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x81000000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetDisplacement)
                    })
                })
            })
        }),
        AddressingEntry(ADDR_MEMORY_OFFSET,
        {
            PayloadTypeEntry(PAYLOAD_GPR,
            {
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x81000000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                        ManipulationEntry(SECOND_OFFSET, &Instruction::SetDisplacement)
                    })
                })
            })
        })
    });

    InstructionEntry(CALL,
    {
        AddressingEntry(ADDR_DIRECT,
        {
            PayloadTypeEntry(PAYLOAD_VALUE,
            {
                SizeEntry(SIZE_BIG,
                {
                    ProcessorInstruction(0x21F00000,
                    {
                        ManipulationEntry(POOL_ENTRY | FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x20000000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                })
            })
        })
    });

    InstructionEntry(JMP,
    {
        AddressingEntry(ADDR_DIRECT,
        {
            PayloadTypeEntry(PAYLOAD_VALUE,
            {
                SizeEntry(SIZE_BIG,
                {
                    ProcessorInstruction(0x38F00000,
                    {
                        ManipulationEntry(POOL_ENTRY | FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x30000000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                })
            })
        })
    });

    InstructionEntry(BEQ,
    {
        AddressingEntry(ADDR_DIRECT,
        {
            PayloadTypeEntry(PAYLOAD_VALUE,
            {
                SizeEntry(SIZE_BIG,
                {
                    ProcessorInstruction(0x39F00000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(POOL_ENTRY | THIRD_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x31000000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                })
            })
        })
    });

    InstructionEntry(BNE,
    {
        AddressingEntry(ADDR_DIRECT,
        {
            PayloadTypeEntry(PAYLOAD_VALUE,
            {
                SizeEntry(SIZE_BIG,
                {
                    ProcessorInstruction(0x3AF00000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(POOL_ENTRY | THIRD_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x32000000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                })
            })
        })
    });

    InstructionEntry(BGT,
    {
        AddressingEntry(ADDR_DIRECT,
        {
            PayloadTypeEntry(PAYLOAD_VALUE,
            {
                SizeEntry(SIZE_BIG,
                {
                    ProcessorInstruction(0x3BF00000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(POOL_ENTRY | THIRD_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x33000000,
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                })
            })
        })
    });
}

void Conversion::PopulateInstructionsMap()
{
    NONE_Instruction(HALT, SingleProcessorInstruction(0x00000000, NO_MANIPULATION));

    NONE_Instruction(INT, SingleProcessorInstruction(0x10000000, NO_MANIPULATION));

    NONE_Instruction
    (
        IRET,
        {
            ProcessorInstruction(0x93FE0004, NO_MANIPULATION),
            ProcessorInstruction(0x970E0004, NO_MANIPULATION)
        }
    );

    NONE_Instruction(RET, SingleProcessorInstruction(0x93FE004, NO_MANIPULATION));

    GPR_Instruction
    (
        PUSH,
        SingleProcessorInstruction
        (
            0x81E00FFC,
            SingleManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC)
        )
    );

    GPR_Instruction
    (
        POP,
        SingleProcessorInstruction
        (
            0x930E0004,
            SingleManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterA)
        )
    );

    GPR_Instruction
    (
        XCHG,
        SingleProcessorInstruction
        (
            0x40000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterC)
            }
        )
    );

    GPR_Instruction
    (
        ADD,
        SingleProcessorInstruction
        (
            0x50000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    GPR_Instruction
    (
        SUB,
        SingleProcessorInstruction
        (
            0x51000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    GPR_Instruction
    (
        MUL,
        SingleProcessorInstruction
        (
            0x52000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    GPR_Instruction
    (
        DIV,
        SingleProcessorInstruction
        (
            0x53000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    GPR_Instruction
    (
        NOT,
        SingleProcessorInstruction
        (
            0x60000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    GPR_Instruction
    (
        AND,
        SingleProcessorInstruction
        (
            0x61000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    GPR_Instruction
    (
        OR,
        SingleProcessorInstruction
        (
            0x62000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    GPR_Instruction
    (
        XOR,
        SingleProcessorInstruction
        (
            0x63000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    GPR_Instruction
    (
        SHL,
        SingleProcessorInstruction
        (
            0x70000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    GPR_Instruction
    (
        SHR,
        SingleProcessorInstruction
        (
            0x71000000,
            {
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    CSR_Instruction
    (
        CSRRD,
        SingleProcessorInstruction
        (
            0x90000000,
            {
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    CSR_Instruction
    (
        CSRRD,
        SingleProcessorInstruction
        (
            0x95000000,
            {
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    PopulateSpecial();
}

/// @brief Get operand size value. For instructions that only support direct addressing, index will always be -1.
/// @param instruction 
/// @param index 
/// @return 
eValueSize Conversion::GetOperandValueSize(const AssemblyInstruction::s_ptr& instruction)
{
    auto index = instruction->GetVariableOperandIndex();
    if (index == -1)
        return eValueSize::SIZE_NONE;
    
    if (index >= instruction->operands.size())
        throw AssemblyException();
    
    return instruction->operands[index].asmValue > 0x7FF ? eValueSize::SIZE_BIG : eValueSize::SIZE_SMALL;
}


#define CONTAINS_THROW(map, entry) \
    if (map.find(entry) == map.end()) \
        throw AssemblyException();

#define CONTAINS_NO_THROW(map, entry) \
    if (map.find(entry) == map.end()) \
        return false;


AssemblyInstructionMetadata Conversion::GetProcessorInstructions(const AssemblyInstruction::s_ptr& instruction)
{
    auto identifier = instruction->identifier;
    auto addressing = instruction->GetAddressingType();

    auto operandType = instruction->GetOperandType();

    auto payloadType = OperandTypeToPayloadType(operandType);
    auto sizeValue = GetOperandValueSize(instruction);

    CONTAINS_THROW(methods, identifier);
    CONTAINS_THROW(methods[identifier], addressing);
    CONTAINS_THROW(methods[identifier][addressing], payloadType);
    CONTAINS_THROW(methods[identifier][addressing][payloadType], sizeValue);

    return methods[identifier][addressing][payloadType][sizeValue];
}

ePayloadType Conversion::OperandTypeToPayloadType(eOperandType type)
{
    if (type == eOperandType::NONE_TYPE)
        return ePayloadType::PAYLOAD_NONE;

    if (type == SYM || type == LTR)
        return ePayloadType::PAYLOAD_VALUE;

    return type == GPR ? ePayloadType::PAYLOAD_GPR : ePayloadType::PAYLOAD_CSR;
}

std::map<ValueToUseMasks, int> valueMaskToOperandIndex =
{
    { FIRST_OPERAND, 0 },
    { SECOND_OPERAND, 1 },
    { THIRD_OPERAND, 2 },
    { FIRST_OFFSET, -3}, // will be adding 3 to this val ue
    { SECOND_OFFSET, -2 },
    { THIRD_OFFSET, -1 },
    { NONE, INT32_MAX },
    { POOL_ENTRY, INT32_MAX }
};

uint16_t Conversion::GetOperandValue(const AssemblyInstruction::s_ptr& instruction, ValueToUseMasks enumerator)
{
    int index = valueMaskToOperandIndex[enumerator];
    if (index >= 0)
        return instruction->operands[index].asmValue;
    
    index += 3;
    return instruction->operands[index].asmOffset;
}

bool Conversion::IsBigValueInstruction(const eInstructionIdentifier& instruction, const eAddressingType& addressing, const ePayloadType& payloadType)
{
    CONTAINS_NO_THROW(methods, instruction);
    CONTAINS_NO_THROW(methods[instruction], addressing);
    CONTAINS_NO_THROW(methods[instruction][addressing], payloadType);

    return methods[instruction][addressing][payloadType].find(eValueSize::SIZE_BIG) != methods[instruction][addressing][payloadType].end();
}
