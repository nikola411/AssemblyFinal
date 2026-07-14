#include "Linker.hpp"

#include <iostream>
#include <fstream>

#include "Helpers.hpp"

Linker::Linker() {}
Linker::~Linker() {}

/*

Linker treba da pokupi ulazne fajlove koje dobije i spoji u jedan izlazni fajl.
Taj izlazni fajl moze da bude hex ili relocatable.
Ako je izlaz hex, fajl mora da bude moguce izvrsiti (izvrsi elf format).
Na ulaz dolaze elf fajlovi koje je obradio asembler, a linker treba da ih ucita.

*/

void Linker::SetInput(const std::string& input)
{
    inputFiles.push_back(input);
}

void Linker::SetOutput(const std::string& output)
{
    outputFile = output;
}

void Linker::SetPlace(const std::string& sectionName, const std::string& address)
{
    //sectionPositions[sectionName] = address;
}

void Linker::SetMode(const uint8_t mode)
{
    modes |= mode;
}

void Linker::ConsumeInput()
{
    for (const auto& path: inputFiles)
    {
        auto raw = ReadBinaryFile(path);
        Elf elf = {};
        elf.LoadBinary(raw);

        SymbolTable symt;
        SectionTable sect;
        RelocationTable relt;

        elf.UnloadLinkable(symt, sect, relt);

        absFiles.push_back(AbstractFile{symt, sect, relt});
    }
}

void Linker::ResolveSymbols()
{

}

void Linker::ResolveRelocations()
{

}

void Linker::MergeSections()
{

}

void Linker::CreateOutput()
{
    
}
