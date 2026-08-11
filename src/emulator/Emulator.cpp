#include "Emulator.hpp"

#include "DataTypes.hpp"
#include "Utility.hpp"

#include <string>
#include <iostream>
#include <mutex>

#include "Instruction.hpp"

Emulator::Emulator()
{
    for (int i = 0; i < 16; i++)
    {
        registers[i] = 0;
    }

    // retka mapa: prazna na pocetku, nedefinisane adrese citaju se kao 0

    cause = 0;
    status = 0;
    handler = 0;

    running = false;
}

void Emulator::Load(std::string input)
{
    // svaki red: [4 bajta adresa little-endian][bajtovi podataka]
    auto data = ReadBinaryFile(input);

    for (const auto& row : data)
    {
        if (row.size() < 4)
            continue;

        uint32_t start = row[0] | (row[1] << 8) | (row[2] << 16) | (row[3] << 24);
        for (size_t i = 4; i < row.size(); i++)
            memory[start + (i - 4)] = row[i];
    }

    registers[15] = START_ADDRESS;   // pc — ulazna tacka
    registers[14] = 0xFFFF0000;      // sp (stek raste na dole)
    running = true;
}

void Emulator::Execute()
{
    while (running)
    {
        try
        {
            // proveri da li ima prekida
            if (TerminalInterruptsEnabled() && Interrupts::terminalInterrupt.exchange(false))
            {
                cause = CauseValues::TERM_INT;
                registers[15] = handler;
            }
            else if (TimerInterruptsEnabled() && Interrupts::timerInterrupt.exchange(false))
            {
                cause = CauseValues::TIM_INT;
                registers[15] = handler;
            }

            ExecuteNextInstruction();
        }
        catch (std::exception e)
        {
            std::cerr << "BADOP exception" << "\n";
            running = false;
            return;
        }
    }
}

void Emulator::Finish()
{
    for (int i = 0; i < 16; i++)
    {
        std::cout << "r" << i << "=" << registers[i] << " ";
    }
}

std::unordered_map<uint32_t, uint8_t>& Emulator::GetMemoryRef()
{
    return memory;
}

std::vector<uint8_t> Emulator::ReadMemory(uint32_t &offset, uint32_t &size)
{
    std::vector<uint8_t> data(size, 0);

    std::lock_guard<std::mutex> lock(memoryMutex);
    for (uint32_t i = 0; i < size; i++)
    {
        auto it = memory.find(offset + i);
        if (it != memory.end())
            data[i] = it->second;
    }

    return data;
}

void Emulator::WriteMemory(uint32_t &offset, std::vector<uint8_t> &data)
{
    std::lock_guard<std::mutex> lock(memoryMutex);
    for (size_t i = 0; i < data.size(); i++)
        memory[offset + i] = data[i];
}

void Emulator::ExecuteNextInstruction()
{
    uint32_t word = ReadWord(registers[15]);
    registers[15] += 4;

    Instruction current(word);

    switch (current.GetCode())
    {
        case 0: // instrukcija zaustavljanja procesora
        {
            HandleHalt(current);
            break;
        }
        case 0x1: // instrukcija softverskog prekida
        {
            HandleSoftwareInterrupt(current);
            break;
        }
        case 0x2: // instrukcija poziva potprograma
        {
            HandleCall(current);
            break;
        }
        case 0x3: // instrukcija skoka
        {
            HandleJump(current);
            break;
        }
        case 0x4: // instrukcija atomicne zamene vrednosti
        {
            HandleAtomicSwap(current);
            break;
        }
        case 0x5: // instrukcija aritmetickih operacija
        {
            HandleArithmetic(current);
            break;
        }
        case 0x6: // instrukcija logickih operacija
        {
            HandleLogic(current);
            break;
        }
        case 0x7: // instrukcija pomerackih operacija
        {
            HandleShift(current);
            break;
        }
        case 0x8: // instrukcija smestanja podataka
        {
            HandleStore(current);
            break;
        }
        case 0x9: // instrukcije ucitavanja podataka
        {
            HandleLoad(current);
            break;
        }
    }
}

void Emulator::HandleHalt(Instruction instruction)
{
    if (instruction.GetModifier() != 0)
        throw std::exception();

    running = false;
}

void Emulator::HandleSoftwareInterrupt(Instruction instruction)
{
    if (instruction.GetModifier() != 0)
        throw std::exception();

    Push(status); Push(registers[15]);

    cause = 4;
    status = (status & (~0x1));
    registers[15] = handler;
}

void Emulator::HandleCall(Instruction instruction)
{
    uint8_t mod = instruction.GetModifier();
    if (mod > 1)
        throw std::exception();

    Push(registers[15]);

    uint32_t value = 0;
    value += GetRegisterValue(instruction.GetRegisterA());
    value += GetRegisterValue(instruction.GetRegisterB());
    value += instruction.GetDisplacement();

    if (mod == 0)
        registers[15] = value;
    else
        registers[15] = ReadWord(value);
}

void Emulator::HandleJump(Instruction instruction)
{
    uint32_t value = 0;
    value += GetRegisterValue(instruction.GetRegisterA());
    value += instruction.GetDisplacement();

    uint8_t mod = instruction.GetModifier();
    uint32_t gprB = 0, gprC = 0;

    if (mod != 0x0 && mod != 0x8)
    {
        gprB = GetRegisterValue(instruction.GetRegisterA());
        gprC = GetRegisterValue(instruction.GetRegisterC());
    }

    if ((mod & 0b1000) >> 3 == 0b1)
        value = ReadWord(value);

    switch (mod & 0b0111)
    {
        case 0x0:
        {
            registers[15] = value;
            break;
        }
        case 0x1:
        {
            if (gprB == gprC)
                registers[15] = value;
            break;
        }
        case 0x2:
        {
            if (gprB != gprC)
                registers[15] = value;
            break;
        }
        case 0x3:
        {
            if ((int32_t)gprB > (int32_t)gprC)
                registers[15] = value;
            break;
        }
        default:
            throw std::exception();
    }
}

void Emulator::HandleAtomicSwap(Instruction instruction)
{
    uint8_t regB = instruction.GetRegisterB();
    uint8_t regC = instruction.GetRegisterC();

    if (regB > 15 || regC > 15)
        throw std::exception();

    std::swap(registers[regB], registers[regC]);
}

void Emulator::HandleArithmetic(Instruction instruction)
{
    uint8_t regA = instruction.GetRegisterA();
    uint8_t regB = instruction.GetRegisterB();
    uint8_t regC = instruction.GetRegisterC();

    uint8_t mod = instruction.GetModifier();
    switch (mod)
    {
        case 0x0:
        {
            SetRegisterValue(regA,
                GetRegisterValue(regB) + GetRegisterValue(regC));
            break;
        }
        case 0x1:
        {
            SetRegisterValue(regA,
                GetRegisterValue(regB) - GetRegisterValue(regC));
            break;
        }
        case 0x2:
        {
            SetRegisterValue(regA,
                GetRegisterValue(regB) * GetRegisterValue(regC));
            break;
        }
        case 0x3:
        {
            SetRegisterValue(regA,
                GetRegisterValue(regB) / GetRegisterValue(regC));
            break;
        }
        default:
            throw std::exception();
    }
}

void Emulator::HandleLogic(Instruction instruction)
{
    uint8_t regA = instruction.GetRegisterA();
    uint8_t regB = instruction.GetRegisterB();
    uint8_t regC = instruction.GetRegisterC();

    uint8_t mod = instruction.GetModifier();
    switch (mod)
    {
        case 0x0:
        {
            SetRegisterValue(regA, ~GetRegisterValue(regB));
            break;
        }
        case 0x1:
        {
            SetRegisterValue(regA,
                GetRegisterValue(regB) & GetRegisterValue(regC));
            break;
        }
        case 0x2:
        {
            SetRegisterValue(regA,
                GetRegisterValue(regB) | GetRegisterValue(regC));
            break;
        }
        case 0x3:
        {
            SetRegisterValue(regA,
                GetRegisterValue(regB) ^ GetRegisterValue(regC));
            break;
        }
        default:
            throw std::exception();
    }
}

void Emulator::HandleShift(Instruction instruction)
{
    uint8_t regA = instruction.GetRegisterA();
    uint8_t regB = instruction.GetRegisterB();
    uint8_t regC = instruction.GetRegisterC();

    uint8_t mod = instruction.GetModifier();
    switch (mod)
    {
        case 0x0:
        {
            SetRegisterValue(regA, GetRegisterValue(regB) << GetRegisterValue(regC));
            break;
        }
        case 0x1:
        {
            SetRegisterValue(regA, GetRegisterValue(regB) >> GetRegisterValue(regC));
            break;
        }
        default:
            throw std::exception();
    }
}

void Emulator::HandleStore(Instruction instruction)
{
    uint8_t regA = instruction.GetRegisterA();
    uint8_t regB = instruction.GetRegisterB();
    uint8_t regC = instruction.GetRegisterC();
    uint16_t disp = instruction.GetDisplacement();

    uint8_t mod = instruction.GetModifier();
    switch (mod)
    {
        case 0x0:
        {
            // mem32[gpr[A] + gpr[B] + D] <= gpr[C]
            uint32_t addr = GetRegisterValue(regA) + GetRegisterValue(regB) + disp;
            WriteWord(addr, GetRegisterValue(regC));
            break;
        }
        case 0x1:
        {
            // gpr[A] <= gpr[A] + D; mem32[gpr[A]] <= gpr[C]  (pre-modify, npr. push)
            uint32_t addr = GetRegisterValue(regA) + disp;
            SetRegisterValue(regA, addr);
            WriteWord(addr, GetRegisterValue(regC));
            break;
        }
        case 0x2:
        {
            uint32_t addr = GetRegisterValue(regA) + GetRegisterValue(regB) + disp;
            addr = ReadWord(addr);
            WriteWord(addr, GetRegisterValue(regC));
        }
        default:
            throw std::exception();
    }
}

void Emulator::HandleLoad(Instruction instruction)
{
    uint8_t regA = instruction.GetRegisterA();
    uint8_t regB = instruction.GetRegisterB();
    uint8_t regC = instruction.GetRegisterC();
    uint16_t disp = instruction.GetDisplacement();

    uint8_t mod = instruction.GetModifier();
    switch (mod)
    {
        case 0x1:
        {
            // gpr[A] <= gpr[B] + D
            SetRegisterValue(regA, GetRegisterValue(regB) + disp);
            break;
        }
        case 0x2:
        {
            // gpr[A] <= mem32[gpr[B] + gpr[C] + D]
            uint32_t addr = GetRegisterValue(regB) + GetRegisterValue(regC) + disp;
            SetRegisterValue(regA, ReadWord(addr));
            break;
        }
        default:
            throw std::exception();
    }
}

void Emulator::Push(uint32_t value)
{
    registers[14] -= 4;
    WriteWord(registers[14], value);
}

uint32_t Emulator::ReadWord(uint32_t addr)
{
    uint32_t value = 0;
    for (int i = 0; i < 4; i++)
    {
        auto it = memory.find(addr + i);
        uint8_t b = (it == memory.end()) ? 0 : it->second;
        value |= (uint32_t)b << (8 * i);
    }
    return value;
}

void Emulator::WriteWord(uint32_t addr, uint32_t value)
{
    for (int i = 0; i < 4; i++)
        memory[addr + i] = (value >> (8 * i)) & 0xFF;
}

uint32_t Emulator::GetRegisterValue(const uint8_t &index)
{
    if (index > 15)
        throw std::exception();

    return registers[index];
}

void Emulator::SetRegisterValue(const uint8_t &index, const uint32_t &value)
{
    if (index > 15)
        throw std::exception();

    registers[index] = value;
}
