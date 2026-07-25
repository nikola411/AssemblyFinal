#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <string>
#include <inttypes.h>
#include <memory>
#include <vector>

struct Symbol
{
    std::string name;
    std::string section;

    uint32_t offset = 0;
    uint16_t value = 0; // pool entry
    bool defined = false;
    bool isGlobal = false;
    bool isExtern = false;
    bool isConstant = false;

    Symbol(){};
    Symbol(std::string name, std::string section, uint32_t offset, uint16_t value = 0, bool defined = false, bool isGlobal = false, bool isExtern = false, bool isConstant = false)
        : name(name), section(section), offset(offset), value(value), defined(defined), isGlobal(isGlobal), isExtern(isExtern), isConstant(isConstant) {};

    typedef std::shared_ptr<Symbol> s_ptr;
};
typedef std::vector<Symbol::s_ptr> SymbolTable;

Symbol::s_ptr GetSymbol(const SymbolTable& table, const std::string& label);
std::string SymbolTableToString(const SymbolTable& table);

#endif