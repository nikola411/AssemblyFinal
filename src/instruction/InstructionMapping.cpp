#include "InstructionMapping.hpp"

using namespace Conversion;

void Conversion::InstructionEntry(eAssemblyIdentifier instruction, std::vector<AddressingEntry> entries)
{
    for (const auto& addr : entries)
        for (const auto& payload : addr.payloadEntry)
            for (const auto& size : payload.sizes)
            {
                if (size.value == eValueSize::SIZE_BIG)
                    bigValueInstructions.insert(instruction);

                methods[{instruction, addr.type, payload.type, size.value}] = size.processorInstructions;
            }
}

void Conversion::NONE_Instruction(eAssemblyIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods[{instruction, eAddressingType::ADDR_NONE, ePayloadType::PAYLOAD_NONE, eValueSize::SIZE_NONE}] = processorInstructions;
}

void Conversion::GPR_Instruction(eAssemblyIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods[{instruction, eAddressingType::ADDR_DIRECT, ePayloadType::PAYLOAD_GPR, eValueSize::SIZE_NONE}] = processorInstructions;
}

void Conversion::VALUE_Instruction(eAssemblyIdentifier instruction, eAddressingType type, eValueSize size, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods[{instruction, type, ePayloadType::PAYLOAD_VALUE, size}] = processorInstructions;
}

void Conversion::CSR_Instruction(eAssemblyIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions)
{
    methods[{instruction, eAddressingType::ADDR_DIRECT, ePayloadType::PAYLOAD_CSR, eValueSize::SIZE_NONE}] = processorInstructions;
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
                    ProcessorInstruction(0x91000000, // OC=1001 MOD=0001: gpr[A]<=gpr[B]+D (D=0, ld %rS,%rD)
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
                    ProcessorInstruction(0x920F0000, // OC=1001 MOD=0010 RegB=r15(pc): gpr[A]<=mem32[pc+D]  (load big literal from pool)
                    {
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                        ManipulationEntry(POOL_ENTRY | FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x91000000, // OC=1001 MOD=0001: gpr[A]<=gpr[0]+D=D  (immediate small literal)
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
                    ProcessorInstruction(0x92000000, // OC=1001 MOD=0010: gpr[A]<=mem32[gpr[B]+0+0]  (ld [%rS],%rD)
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
                    ProcessorInstruction(0x92000000, // OC=1001 MOD=0010: gpr[A]<=mem32[0+0+D]  (ld addr,%rD, small addr)
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
                    ProcessorInstruction(0x92000000, // OC=1001 MOD=0010: gpr[A]<=mem32[gpr[B]+0+D]  (ld [%rS+offset],%rD)
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
                    ProcessorInstruction(0x80000000, // OC=1000 MOD=0000: mem32[0+gpr[B]+0]<=gpr[C]  (st %rS,[%rD])
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
                    ProcessorInstruction(0x81F00000, // OC=1000 MOD=0001 RegA=r15(pc): pc<=pc+D; mem32[pc]<=gpr[C]  (st %rS, big addr via pool)
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(POOL_ENTRY | SECOND_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x80000000, // OC=1000 MOD=0000: mem32[0+0+D]<=gpr[C]  (st %rS, small addr)
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
                    ProcessorInstruction(0x81000000, // OC=1000 MOD=0001: gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C]  (D=0, st %rS,[%rD])
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
                    ProcessorInstruction(0x81000000, // OC=1000 MOD=0001: gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C]  (st %rS, small addr)
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
                    ProcessorInstruction(0x81000000, // OC=1000 MOD=0001: gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C]  (st %rS,[%rD+offset])
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
                    ProcessorInstruction(0x21F00000, // OC=0010 MOD=0001 RegA=r15(pc): push pc; pc<=mem32[pc+D]  (call big addr via pool)
                    {
                        ManipulationEntry(POOL_ENTRY | FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x20000000, // OC=0010 MOD=0000: push pc; pc<=gpr[0]+gpr[0]+D=D  (call small addr)
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
                    ProcessorInstruction(0x38F00000, // OC=0011 MOD=1000 RegA=r15(pc): pc<=mem32[pc+D]  (jmp big addr via pool)
                    {
                        ManipulationEntry(POOL_ENTRY | FIRST_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x30000000, // OC=0011 MOD=0000: pc<=gpr[0]+D=D  (jmp small addr)
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
                    ProcessorInstruction(0x39F00000, // OC=0011 MOD=1001 RegA=r15(pc): if(gpr[B]==gpr[C]) pc<=mem32[pc+D]  (beq big)
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(POOL_ENTRY | THIRD_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x31000000, // OC=0011 MOD=0001: if(gpr[B]==gpr[C]) pc<=gpr[0]+D=D  (beq small)
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
                    ProcessorInstruction(0x3AF00000, // OC=0011 MOD=1010 RegA=r15(pc): if(gpr[B]!=gpr[C]) pc<=mem32[pc+D]  (bne big)
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(POOL_ENTRY | THIRD_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x32000000, // OC=0011 MOD=0010: if(gpr[B]!=gpr[C]) pc<=gpr[0]+D=D  (bne small)
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
                    ProcessorInstruction(0x3BF00000, // OC=0011 MOD=1011 RegA=r15(pc): if(gpr[B] signed> gpr[C]) pc<=mem32[pc+D]  (bgt big)
                    {
                        ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB),
                        ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterC),
                        ManipulationEntry(POOL_ENTRY | THIRD_OPERAND, &Instruction::SetDisplacement)
                    })
                }),
                SizeEntry(SIZE_SMALL,
                {
                    ProcessorInstruction(0x33000000, // OC=0011 MOD=0011: if(gpr[B] signed> gpr[C]) pc<=gpr[0]+D=D  (bgt small)
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
    NONE_Instruction(HALT, SingleProcessorInstruction(0x00000000, NO_MANIPULATION)); // OC=0000: zaustavlja procesor

    NONE_Instruction(INT, SingleProcessorInstruction(0x10000000, NO_MANIPULATION)); // OC=0001: push status; push pc; cause<=4; status<=status&(~1); pc<=handler

    NONE_Instruction
    (
        IRET,
        {
            ProcessorInstruction(0x93FE0004, NO_MANIPULATION), // OC=1001 MOD=0011: pc<=mem32[sp]; sp<=sp+4  (pop pc)
            ProcessorInstruction(0x970E0004, NO_MANIPULATION)  // OC=1001 MOD=0111: status<=mem32[sp]; sp<=sp+4  (pop status)
        }
    );

    NONE_Instruction(RET, SingleProcessorInstruction(0x93FE004, NO_MANIPULATION)); // OC=1001 MOD=0011: pc<=mem32[sp]; sp<=sp+4  (pop pc)

    GPR_Instruction
    (
        PUSH,
        SingleProcessorInstruction
        (
            0x81E00FFC, // OC=1000 MOD=0001: sp<=sp+(-4); mem32[sp]<=gpr[C]
            SingleManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterC)
        )
    );

    GPR_Instruction
    (
        POP,
        SingleProcessorInstruction
        (
            0x930E0004, // OC=1001 MOD=0011: gpr[A]<=mem32[sp]; sp<=sp+4
            SingleManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterA)
        )
    );

    GPR_Instruction
    (
        XCHG,
        SingleProcessorInstruction
        (
            0x40000000, // OC=0100: temp<=gpr[B]; gpr[B]<=gpr[C]; gpr[C]<=temp
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
            0x50000000, // OC=0101 MOD=0000: gpr[A]<=gpr[B]+gpr[C]
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
            0x51000000, // OC=0101 MOD=0001: gpr[A]<=gpr[B]-gpr[C]
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
            0x52000000, // OC=0101 MOD=0010: gpr[A]<=gpr[B]*gpr[C]
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
            0x53000000, // OC=0101 MOD=0011: gpr[A]<=gpr[B]/gpr[C]
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
            0x60000000, // OC=0110 MOD=0000: gpr[A]<=~gpr[B]
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
            0x61000000, // OC=0110 MOD=0001: gpr[A]<=gpr[B]&gpr[C]
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
            0x62000000, // OC=0110 MOD=0010: gpr[A]<=gpr[B]|gpr[C]
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
            0x63000000, // OC=0110 MOD=0011: gpr[A]<=gpr[B]^gpr[C]
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
            0x70000000, // OC=0111 MOD=0000: gpr[A]<=gpr[B]<<gpr[C]
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
            0x71000000, // OC=0111 MOD=0001: gpr[A]<=gpr[B]>>gpr[C]
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
            0x90000000, // OC=1001 MOD=0000: gpr[A]<=csr[B]
            {
                ManipulationEntry(SECOND_OPERAND, &Instruction::SetRegisterA),
                ManipulationEntry(FIRST_OPERAND, &Instruction::SetRegisterB)
            }
        )
    );

    CSR_Instruction
    (
        CSRWR,
        SingleProcessorInstruction
        (
            0x95000000, // OC=1001 MOD=0101: csr[A]<=csr[B]|D
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
eValueSize Conversion::GetOperandValueSize(const AssemblyLine::s_ptr& instruction)
{
    auto index = instruction->GetVariableOperandIndex();
    if (index == -1)
        return eValueSize::SIZE_NONE;

    if (index >= instruction->operands.size())
        throw AssemblyException();

    auto& op = instruction->operands[index];
    if (op.type == eOperandType::GPR || op.type == eOperandType::CSR)
    {
        if (instruction->GetAddressingType() == eAddressingType::ADDR_MEMORY_OFFSET)
            return op.asmOffset > 0x7FF ? eValueSize::SIZE_BIG : eValueSize::SIZE_SMALL;
        return eValueSize::SIZE_NONE;
    }

    return op.isBigValue ? eValueSize::SIZE_BIG : eValueSize::SIZE_SMALL;
}

AssemblyLineMetadata Conversion::GetProcessorInstructions(const AssemblyLine::s_ptr& instruction)
{
    InstructionKey key = {
        instruction->identifier,
        instruction->GetAddressingType(),
        OperandTypeToPayloadType(instruction->GetOperandType()),
        GetOperandValueSize(instruction)
    };

    auto it = methods.find(key);
    if (it == methods.end())
        throw AssemblyException();

    return it->second;
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

uint16_t Conversion::GetOperandValue(const AssemblyLine::s_ptr& instruction, ValueToUseMasks enumerator)
{
    int index = valueMaskToOperandIndex[enumerator];
    if (index >= 0)
        return instruction->operands[index].asmValue;
    
    index += 3;
    return instruction->operands[index].asmOffset;
}

bool Conversion::IsBigValueInstruction(const eAssemblyIdentifier& instruction, const eAddressingType& addressing, const ePayloadType& payloadType)
{
    return methods.find({instruction, addressing, payloadType, eValueSize::SIZE_BIG}) != methods.end();
}
