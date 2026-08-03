#ifndef RELOCATION_HPP
#define RELOCATION_HPP

#include "ParserTypes.hpp"
#include "AssemblyLine.hpp"

#include <string>
#include <inttypes.h>
#include <memory>
#include <vector>


struct Relocation
{
    std::string symbolName;
    uint32_t offset;
    eRelocationType type;
    int32_t addend;

    typedef std::shared_ptr<Relocation> s_ptr;
};

using RelocationTable = std::vector<Relocation::s_ptr>;

std::string RelocationTableToString(const RelocationTable& table);

enum ForwardReferenceType
{
    FREF32_ABS = 1,
    FREF12_PC = 2,
};
struct ForwardReference
{
    std::string symbolName;
    std::string sectionName;
    uint32_t offset;
    std::shared_ptr<AssemblyLine> instruction;
    uint32_t addend;
    ForwardReferenceType type;

    typedef std::shared_ptr<ForwardReference> s_ptr;
};

using ForwardReferenceTable = std::vector<ForwardReference::s_ptr>;


#endif