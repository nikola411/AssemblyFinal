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
    Instruction (uint32_t value) : data(value) {};
    Instruction() = default;
    virtual ~Instruction() = default;

    uint8_t GetCode() const;
    uint8_t GetModifier() const;
    uint8_t GetRegisterA() const;
    uint8_t GetRegisterB() const;
    uint8_t GetRegisterC() const;
    uint16_t GetDisplacement() const;

    void SetCode(uint16_t value);
    void SetModifier(uint16_t value);
    void SetRegisterA(uint16_t value);
    void SetRegisterB(uint16_t value);
    void SetRegisterC(uint16_t value);
    void SetDisplacement(uint16_t value);

    void DoNothing(uint16_t value) {};

    uint32_t GetData() const;
    void SetData(uint32_t);

    uint32_t data;
};

#endif