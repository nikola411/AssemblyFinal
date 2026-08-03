#include "Emulator.hpp"

#include "Terminal.hpp"
#include "Timer.hpp"

#include <string>
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: emulator <input>\n";
        return 1;
    }
    Emulator emulator;

    auto& memory = emulator.GetMemoryRef();
    Timer timer(memory);
    Terminal terminal(memory);

    auto timThread = std::thread(&Timer::DoWork, timer);

    emulator.Load(argv[1]);
    emulator.Execute();
    emulator.Finish();

    std::cout << "\n";
    return 0;
}