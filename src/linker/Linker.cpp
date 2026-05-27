#include "Linker.hpp"

#include <iostream>
#include <fstream>

Linker::Linker() {}
Linker::~Linker() {}

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
    sectionPositions[sectionName] = address;
}

void Linker::SetMode(const uint8_t mode)
{
    modes |= mode;
}

void Linker::ConsumeInput()
{
    for (const auto& path: inputFiles)
    {
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line))
        {
            // obradi line
        }
    }
}

void Linker::ResolveSymbols() {}
void Linker::ResolveRelocations() {}
void Linker::MergeSections() {}
void Linker::CreateOutput() {}
