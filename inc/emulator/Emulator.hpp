#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <vector>
#include <string>
#include <stdint.h>
#include <cmath>

const uint32_t MEMORY_SIZE = pow(2, 32);
const uint32_t START_ADDRESS = 0x40000000;
const uint32_t MEMORY_MAPPED_REGISTERS = 0xFFFFFF00;

class Emulator
{
public:
    Emulator();
    ~Emulator() {};

    void Load(std::string input);
    void Execute();
    void Finish();

private:
    uint32_t registers[16];
    uint32_t status, handler, cause;

    std::vector<uint32_t> memory;
};

#endif