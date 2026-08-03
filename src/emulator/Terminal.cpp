#include "Terminal.hpp"

#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <unordered_map>

Terminal::Terminal(std::unordered_map<uint32_t, uint8_t>& memory)
{
    //nas terminal kontroler init
    for (int i = 0; i < 4; i++)
    {
        memory[0xFFFFFF00 + i] = 0;
        memory[0xFFFFFF04 + i] = 0;

        term_in[i] = std::make_shared<uint8_t>(memory[0xFFFFFF00 + i]);
        term_out[i] = std::make_shared<uint8_t>(memory[0xFFFFFF04 + i]);
    }

    // termios init
    tcgetattr(STDIN_FILENO, &old_tio);
    raw_tio = old_tio;
    raw_tio.c_lflag &= ~(ICANON | ECHO);
    raw_tio.c_cc[VMIN]  = 0;
    raw_tio.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw_tio);
}

Terminal::~Terminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}

void Terminal::WorkLoop()
{
    uint8_t c;
    if (read(STDIN_FILENO, &c, 1) == 1)
    {
        std::lock_guard<std::mutex> lock(memoryMutex);
        *term_in[0] = c;
    }

    uint8_t out = 0;
    {
        std::lock_guard<std::mutex> lock(memoryMutex);
        out = *term_out[0];
    }

    if (out != 0)
    {
        putchar(out);
        fflush(stdout);

        {
            std::lock_guard<std::mutex> lock(memoryMutex);
            *term_out[0] = 0;
        }
    }
}