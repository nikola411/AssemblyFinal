#include "Timer.hpp"

#include <chrono>
#include <thread>
#include <unordered_map>

Timer::Timer(std::unordered_map<uint32_t, uint8_t>& memory)
{
    for (int i = 0; i < 4; i++)
    {
        memory[0xFFFFFF10 + i] = 0;
        tim_cfg[i] = std::make_shared<uint8_t>(memory[0xFFFFFF10 + i]);
    }
}

void Timer::WorkLoop()
{
    while (started)
    {
        TimerConfigValues value = (TimerConfigValues)(*tim_cfg[0]);
        int sleepDuration = TimerCycleMap[value];
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));

    }
}

void Timer::SetConfiguration(const TimerConfigValues& config)
{
    std::lock_guard<std::mutex> lock(memoryMutex);
    *(tim_cfg[0]) = config;
}
