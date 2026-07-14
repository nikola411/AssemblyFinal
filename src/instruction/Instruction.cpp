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

void Instruction::SetCode(uint16_t value)
{
    data &= ~(0xF << InstructionOffsets::Code);
    data |= (value & 0xF) << InstructionOffsets::Code;
}

void Instruction::SetModifier(uint16_t value)
{
    data &= ~(0xF << InstructionOffsets::Modifier);
    data |= (value & 0xF) << InstructionOffsets::Modifier;
}

void Instruction::SetRegisterA(uint16_t value)
{
    data &= ~(0xF << InstructionOffsets::RegisterA);
    data |= (value & 0xF) << InstructionOffsets::RegisterA;
}

void Instruction::SetRegisterB(uint16_t value)
{
    data &= ~(0xF << InstructionOffsets::RegisterB);
    data |= (value & 0xF) << InstructionOffsets::RegisterB;
}

void Instruction::SetRegisterC(uint16_t value)
{
    data &= ~(0xF << InstructionOffsets::RegisterC);
    data |= (value & 0xF) << InstructionOffsets::RegisterC;
}

void Instruction::SetDisplacement(uint16_t value)
{
    data &= ~(0xFFF << InstructionOffsets::Displacement);
    data |= (value & 0xFFF) << InstructionOffsets::Displacement;
}

uint32_t Instruction::GetData() const
{
    return data;
}

void Instruction::SetData(uint32_t data)
{
    this->data = data;
}
