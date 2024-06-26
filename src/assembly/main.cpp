#include <iostream>
#include <string>

#include "Driver.hpp"
#include "Utility.hpp"

#include <bitset>
#include <memory>
#include <vector>

using std::string;

int main(int argc, char* argv[])
{
    //PARSE ARGUMENTS
    std::cout << "Parsing arguments.\n";
    if (argc > 5 || argc < 4)
    {
        std::cout << "\033[31mERROR: Wrong number of arguments! \n";
        return -1;
    }

    string flag = string(argv[1]);
    string output_file = string(argv[2]);
    string input_file = string(argv[3]);

    bool debug = false;
    if (argc > 4)
    {
        debug = bool(argv[4]);
    }

    std::cout << "Debug enabled: " << debug << "\n";
    if (flag != "-o")
    {
        std::cout << "\033[31mERROR: No output flag specified! \n";
        return -1;
    }
    std::cout << "Starting to read the input file.\n";
    // FINISH PARSING THE ARGUMENTS
    // START ASSEMBLY
    try
    {
        auto adapter = new AssemblyAdapter();
        Driver driver(debug, adapter);
        driver.parse(input_file);
        adapter->Backpatch();       
        adapter->GenerateOutput(output_file);
        
        delete adapter;
    }
    catch(const FatalException& e)
    {
        std::cerr << "\033[31m" << "Aborting execution.\n";
    }
    catch(const AssemblyException& e)
    {
       std::cerr << "\033[31m" << e.GetErrorMessage() << '\n';
    }
    catch(const std::exception& e)
    {
        std::cerr << "\033[31mERROR: " <<  e.what() << '\n';
    }
    
    return 0;
}