#include "Symbol.hpp"

Symbol::s_ptr GetSymbol(const SymbolTable& table, const std::string& label)
{
    for (const auto& entry : table)
        if (entry->name == label)
            return entry;

    return nullptr;
}