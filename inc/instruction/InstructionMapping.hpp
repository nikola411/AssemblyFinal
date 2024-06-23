#ifndef INSTRUCTION_MAPPING_HPP
#define INSTRUCTION_MAPPING_HPP

#include "Utility.hpp"
#include "Instruction.hpp"

#include <map>
#include <vector>

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
    FIRST_OPERAND = 0x00, SECOND_OPERAND = 0x01, THIRD_OPERAND = 0x02,
    FIRST_OFFSET = 0x03, SECOND_OFFSET = 0x04, THIRD_OFFSET = 0x05,
    NONE, POOL_ENTRY
};

#define MANIPULATION_PAIR(operand, method) { eValueToUse::operand, &Instruction::method } 

using InstructionMethod = void (Instruction::*)(uint16_t);
using ManipulationPair = std::pair<eValueToUse, InstructionMethod>;
using ProcessorInstructionMetadata = std::pair<Instruction, std::vector<ManipulationPair>>;
using InstructionMap = std::map<eInstructionIdentifier, std::map<eAddressingType, std::map<ePayloadType, std::map<eValueSize, std::vector<std::pair<Instruction, std::vector<ManipulationPair>>>>>>>;

InstructionMap methods;

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

void InstructionEntry(eInstructionIdentifier instruction, std::vector<AddressingEntry> entries);
void NONE_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions);
void GPR_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions);
void VALUE_Instruction(eInstructionIdentifier instruction, eAddressingType type, eValueSize size, std::vector<ProcessorInstructionMetadata> processorInstructions);
void CSR_Instruction(eInstructionIdentifier instruction, std::vector<ProcessorInstructionMetadata> processorInstructions);

#define NO_MANIPULATION {}

inline std::pair<Instruction, std::vector<ManipulationPair>> ProcessorInstruction(uint32_t code, std::vector<ManipulationPair> data)
{
    return { Instruction(code), data };
}

inline std::vector<std::pair<Instruction, std::vector<ManipulationPair>>> SingleProcessorInstruction(uint32_t code, std::vector<ManipulationPair> data)
{
    return { { Instruction(code), data } };
}

inline std::vector<ManipulationPair> SingleManipulationEntry(const eValueToUse valueToUse, const InstructionMethod method)
{
    return { { valueToUse, method } };
}

ManipulationPair ManipulationEntry(eValueToUse valueToUse, InstructionMethod method)
{
    return { valueToUse, method };
}

void GenerateSpecial()
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
                        ManipulationEntry(POOL_ENTRY, &Instruction::SetDisplacement)
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
                        ManipulationEntry(POOL_ENTRY, &Instruction::SetDisplacement)
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
                        ManipulationEntry(POOL_ENTRY, &Instruction::SetDisplacement)
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
                        ManipulationEntry(POOL_ENTRY, &Instruction::SetDisplacement)
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
                        ManipulationEntry(POOL_ENTRY, &Instruction::SetDisplacement)
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
                        ManipulationEntry(POOL_ENTRY, &Instruction::SetDisplacement)
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
                        ManipulationEntry(POOL_ENTRY, &Instruction::SetDisplacement)
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

void Generate()
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
   
}



#endif