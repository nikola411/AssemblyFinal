#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <string>
#include <map>
#include "parser.hpp"
#include "location.hh"

#include "AssemblyAdapter.hpp"

#define YY_DECL \
    yy::parser::symbol_type yylex (Driver& drv, AssemblyAdapter& asem)
YY_DECL;

class Driver
{
public:
    Driver(bool, AssemblyAdapter*);

    void scan_begin();
    void scan_end();
    int parse(const std::string& f);

    yy::location location;

private:
    std::string file;
    bool trace_parsing;

    AssemblyAdapter* assembly;
};

#endif