#include "Instruction.hpp"

uint8_t Instruction::GetCode() const
{
    return (data >> InstructionOffsets::Code) & 0x0F;
}

uint8_t Instruction::GetModifier() const
{
    return (data >> InstructionOffsets::Modifier) & 0x0F;
}

uint8_t Instruction::GetRegisterA() const
{
    return (data >> InstructionOffsets::RegisterA) & 0x0F;
}

uint8_t Instruction::GetRegisterB() const
{
    return (data >> InstructionOffsets::RegisterB) & 0x0F;
}

uint8_t Instruction::GetRegisterC() const
{
    return (data >> InstructionOffsets::RegisterC) & 0x0F;
}

uint16_t Instruction::GetDisplacement() const
{
    return (data >> InstructionOffsets::Displacement) & 0x0FFF;
}

uint32_t Instruction::GetData() const
{
    return data;
}
