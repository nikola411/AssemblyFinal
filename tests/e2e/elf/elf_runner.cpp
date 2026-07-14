#include "Elf.hpp"
#include "ElfHelpers.hpp"
#include "DataTypes.hpp"

#include "01-unload-linkable/test.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

// Usage: elf_runner -o <output_file> <test_input_file>
// Dispatches based on the parent directory name of <test_input_file>.
int main(int argc, char* argv[])
{
    std::string output_path;
    std::string input_path;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc)
            output_path = argv[++i];
        else
            input_path = arg;
    }

    if (output_path.empty() || input_path.empty())
    {
        std::cerr << "Usage: elf_runner -o <output> <test.elf>\n";
        return 1;
    }

    std::string test_name = std::filesystem::path(input_path).parent_path().filename().string();

    std::string result;
    if (test_name == "01-unload-linkable")
        result = test_01::run();
    else
    {
        std::cerr << "Unknown test: " << test_name << "\n";
        return 1;
    }

    std::ofstream out(output_path);
    if (!out)
    {
        std::cerr << "Cannot open output: " << output_path << "\n";
        return 1;
    }
    out << result;
    return 0;
}
