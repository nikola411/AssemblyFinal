#ifndef TIMER_HPP
#define TIMER_HPP

#include <vector>
#include <memory>
#include <map>

#include "inttypes.h"
#include "DataTypes.hpp"

enum TimerConfigValues
{
    ms500 = 0,
    ms1000 = 1,
    ms1500 = 2,
    ms2000 = 3,
    ms5000 = 4,
    s10 = 5,
    s30 = 6,
    s60 = 7,
};

inline std::map<TimerConfigValues, int> TimerCycleMap =
{
    { ms500, 500 },
    { ms1000, 1000 },
    { ms1500, 1.5 * 1000 },
    { ms2000, 2 * 1000 },
    { ms5000, 5 * 1000 },
    { s10, 10 * 1000 },
    { s30, 30 * 1000 },
    { s60, 60 * 1000 }
};

class Timer
{
public:
    Timer(std::unordered_map<uint32_t, uint8_t>& memory);
    void WorkLoop();
    void SetConfiguration(const TimerConfigValues& config);

    // 0xFFFFFF10-0xFFFFFF13
    std::vector<std::shared_ptr<uint8_t>> tim_cfg; // memory mapped
    bool started = false;
};

#endif