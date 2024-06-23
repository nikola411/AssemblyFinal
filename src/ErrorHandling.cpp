#include "ErrorHandling.hpp"

std::string UndefinedSymbolMessage(const std::string& name)
{
    std::string output = AsmResultToString[ASM_RESULT_UNDEFINED_SYMBOL];
    output += "\n";
    output += name;
    output += " ";
    output += "\n";

    return output;
}
