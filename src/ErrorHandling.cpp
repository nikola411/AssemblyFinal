#include "ErrorHandling.hpp"
#include <regex>

std::string UndefinedSymbolMessage(const std::string& name)
{
    std::string output = AsmResultToString[ASM_RESULT_UNDEFINED_SYMBOL];
    output += "\n";
    output += name;
    output += " ";
    output += "\n";

    return output;
}

std::string AssemblyException::Format(std::string original, std::vector<std::string> modifiers)
{
    const std::regex pattern("(\\{\\})");

    for (const auto& replace : modifiers)
    {
        original = regex_replace(original, pattern, replace, std::regex_constants::format_first_only);
    }

    return original;
}
