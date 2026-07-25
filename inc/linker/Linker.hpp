#ifndef LINKER_HPP
#define LINKER_HPP

#include "Elf.hpp"
#include "Helpers.hpp"

#include <vector>
#include <string>
#include <map>
#include <inttypes.h>

class Linker
{
public:
    Linker();
    ~Linker();

    void DoWork();

    void SetInput(const std::string& input);
    void SetOutput(const std::string& output);
    void SetPlace(const std::string& sectionName, const std::string& address);
    void SetMode(const uint8_t mode);

    void ConsumeInput();
    void ResolveSymbols();
    void ResolveRelocations();
    void MergeSections();
    void PlaceSections();
    void CreateOutput();

private:
    // varijable vezane za unos argumenata, uticu na rad linkera
    uint8_t modes;
    std::vector<std::string> inputFiles;
    std::string outputFile;
    std::map<std::string, uint32_t> sectionPositions;

    // varijable vezane za rad linkera

    std::vector<Elf> rawFiles;
    std::vector<AbstractFile> absFiles;

    // izlazni fajl

    std::vector<std::pair<Symbol::s_ptr, int>> symt;
    SectionTable sect;
};

#endif