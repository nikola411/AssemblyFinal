#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include "Utility.hpp"

namespace InstructionOffsets
{
    constexpr uint8_t Code = 28;
    constexpr uint8_t Modifier = 24;
    constexpr uint8_t RegisterA = 20;
    constexpr uint8_t RegisterB = 16;
    constexpr uint8_t RegisterC = 12;
    constexpr uint8_t Displacement = 0;
}

struct Instruction
{
    uint8_t GetCode() const;
    uint8_t GetModifier() const;
    uint8_t GetRegisterA() const;
    uint8_t GetRegisterB() const;
    uint8_t GetRegisterC() const;
    uint16_t GetDisplacement() const;

    virtual uint32_t GetData() const;
    virtual ~Instruction() = default;

    uint32_t data;
};

#endif