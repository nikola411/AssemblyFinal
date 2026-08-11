#include "Terminal.hpp"

#include "DataTypes.hpp"

#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <unordered_map>

Terminal::Terminal(std::unordered_map<uint32_t, uint8_t>& memory)
{
    //nas terminal kontroler init
    term_in.resize(4);
    term_out.resize(4);

    for (int i = 0; i < 4; i++)
    {
        memory[0xFFFFFF00 + i] = 0;
        memory[0xFFFFFF04 + i] = 0;

        // pokazujemo na sam bajt u mapi, zato prazan deleter — memoriju ne
        // posedujemo. Pokazivaci na elemente unordered_map ostaju validni i
        // posle rehash-a, a emulator nikad ne brise elemente.
        term_in[i] = BytePtr(&memory[0xFFFFFF00 + i], [](uint8_t*) {});
        term_out[i] = BytePtr(&memory[0xFFFFFF04 + i], [](uint8_t*) {});
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
        Interrupts::terminalInterrupt.exchange(true);
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