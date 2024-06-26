#include "Driver.hpp"
#include "AssemblyAdapter.hpp"

#include <filesystem>
#include <iostream>
#include <regex>
#include <string>

std::string GetTestName(std::string& testPath)
{
    std::string firstIter;
    std::regex_replace(std::back_inserter(firstIter), testPath.begin(), testPath.end(), std::regex("_"), " ");
    std::string result;
    std::regex_replace(std::back_inserter(result), firstIter.begin(), firstIter.end(), std::regex("-"), ": ");
    char last = '-';
    for (auto& c : result)
    {
        if (last == ' ')
        {
            c = toupper(c);
            last = c;
        }
        else
            last = c;
    }

    return result;
}

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

int main()
{
    std::cout << "Running the tests.\n";

    int successful = 0;
    int run = 0;

    std::regex test_number_regex("./tests/[0-9]+");
    std::vector<std::string> tests = {};

    for (const auto& dirEntry : std::filesystem::directory_iterator("./tests"))
    {
        if (std::regex_match(dirEntry.path().c_str(), std::regex("./tests/[0-9]+-.*"), std::regex_constants::match_any))
        {
            ++run;
            tests.push_back(std::string(dirEntry.path().c_str()));
        }
    }

    for (auto& test : tests)
    {
        std::string cut = test.substr(8, test.size());
        std::cout << "Running test: " << GetTestName(cut) << "\n";
        std::string input = test + "/test.asm";
        std::string output = test + "/out.asm";

        AssemblyAdapter adapter;
        Driver driver(false, &adapter);
        try
        {
            driver.parse(input);
            adapter.Backpatch();
            adapter.GenerateOutput(test + "/result");
        }
        catch(const AssemblyException& e)
        {
            std::cerr << e.GetErrorMessage() << '\n';
        }
    }

    std::cout << "Run finished.\n";
    std::cout << run - successful << "\\" << run << " tests failed\n";
}