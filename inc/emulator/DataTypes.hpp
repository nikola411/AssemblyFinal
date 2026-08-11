#ifndef DATATYPES_HPP
#define DATATYPES_HPP

#include <memory>
#include <cmath>

using BytePtr = std::shared_ptr<uint8_t>;
using MemoryMappedRegister = std::vector<BytePtr>;

const uint32_t MEMORY_SIZE = pow(2, 32);
const uint32_t START_ADDRESS = 0x40000000;
const uint32_t MEMORY_MAPPED_REGISTERS = 0xFFFFFF00;


#include <mutex>

inline std::mutex memoryMutex;
inline std::mutex registerMutex;
namespace Interrupts
{
    // deklarisemo extern da najavimo postojanje, definicija u cpp fajlu i globalna
    inline std::atomic<bool> timerInterrupt = { false };
    inline std::atomic<bool> terminalInterrupt = { false };
};



#endif