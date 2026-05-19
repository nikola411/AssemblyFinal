#include "Emulator.hpp"

#include <string>
#include <iostream>

Emulator::Emulator()
{
    for (int i = 0; i < 16; i++)
    {
        registers[i] = 0;
    }

    cause = 0;
    status = 0;
    handler = 0;

    memory = std::vector<uint32_t>(MEMORY_SIZE, 0);
}

void Emulator::Load(std::string input)
{
}

void Emulator::Execute()
{
}

void Emulator::Finish()
{
    for (int i = 0; i < 16; i++)
    {
        std::cout << "r" << i << "=" << registers[i] << " ";
    }
}
