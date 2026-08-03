#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <vector>
#include <string>
#include <stdint.h>
#include <cmath>
#include <atomic>
#include <unordered_map>

#include "Instruction.hpp"

// MEMORY_SIZE / START_ADDRESS / MEMORY_MAPPED_REGISTERS su definisani u DataTypes.hpp

enum CauseValues
{
    BAD_OP = 1,
    TIM_INT = 2,
    TERM_INT = 3,
    SW_INT = 4
};

namespace Interrupts
{
    // deklarisemo extern da najavimo postojanje, definicija u cpp fajlu i globalna
    extern std::atomic<bool> timerInterrupt;
    extern std::atomic<bool> terminalInterrupt;
};

class Emulator
{
public:
    Emulator();
    ~Emulator() {};

    void Load(std::string input);
    void Execute();
    void Finish();

    std::unordered_map<uint32_t, uint8_t>& GetMemoryRef();

    std::vector<uint8_t> ReadMemory(uint32_t& offset, uint32_t& size);
    void WriteMemory(uint32_t& offset, std::vector<uint8_t>& data);

    inline bool InterruptsEnabled() { return status >> 2 == 0; }
    inline bool TimerInterruptsEnabled() { return  InterruptsEnabled() && (status & 1) == 0; }
    inline bool TerminalInterruptsEnabled() { return  InterruptsEnabled() && (status & 2) == 0; }

    void ExecuteNextInstruction();

    // instruction handlers, dispatched by the top opcode nibble
    void HandleHalt(Instruction instruction);
    void HandleSoftwareInterrupt(Instruction instruction);
    void HandleCall(Instruction instruction);
    void HandleJump(Instruction instruction);
    void HandleAtomicSwap(Instruction instruction);
    void HandleArithmetic(Instruction instruction);
    void HandleLogic(Instruction instruction);
    void HandleShift(Instruction instruction);
    void HandleStore(Instruction instruction);
    void HandleLoad(Instruction instruction);

    void Push(uint32_t value);

    uint32_t ReadWord(uint32_t addr);
    void WriteWord(uint32_t addr, uint32_t value);

    uint32_t GetRegisterValue(const uint8_t& index);
    void SetRegisterValue(const uint8_t& index, const uint32_t& value);

private:
    uint32_t registers[16];
    uint32_t status, handler, cause;

    std::unordered_map<uint32_t, uint8_t> memory;
    bool running;
};

#endif