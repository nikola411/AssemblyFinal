#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <memory>
#include <vector>
#include <termios.h>

#include "DataTypes.hpp"

class Terminal
{
public:
    Terminal(std::unordered_map<uint32_t, uint8_t>& memory);
    ~Terminal();
    void WorkLoop();


private:
    //[0xFFFFFF00-0xFFFFFF03]
    std::vector<BytePtr> term_in;
    //[0xFFFFFF04-0xFFFFFF07]
    std::vector<BytePtr> term_out;

    struct termios old_tio, raw_tio;
};

#endif