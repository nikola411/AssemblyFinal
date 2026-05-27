#ifndef LINKER_HPP
#define LINKER_HPP

#include <vector>
#include <string>
#include <map>
#include <inttypes.h>

class Linker
{
public:
    Linker();
    ~Linker();

    void SetInput(const std::string& input);
    void SetOutput(const std::string& output);
    void SetPlace(const std::string& sectionName, const std::string& address);
    void SetMode(const uint8_t mode);

    void ConsumeInput();
    void ResolveSymbols();
    void ResolveRelocations();
    void MergeSections();
    void CreateOutput();

private:
    // varijable vezane za unos argumenata, uticu na rad linkera
    uint8_t modes;
    std::vector<std::string> inputFiles;
    std::string outputFile;
    std::map<std::string, uint32_t> sectionPositions;

    // varijable vezane za rad linkera
};

#endif