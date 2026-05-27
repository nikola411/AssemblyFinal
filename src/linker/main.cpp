#include "Linker.hpp"

#include "Helpers.hpp"

#include <memory>
#include <iostream>

int main(int argc, char* argv[])
{
    auto linker = std::make_shared<Linker>();
    ParseArguments(linker, argc, argv);

    try
    {
        linker->ResolveSymbols();
        linker->ResolveRelocations();
        linker->MergeSections();
        linker->CreateOutput();
    }
    catch (const ErrorHandling::SymbolResolutionException& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (const ErrorHandling::RelocationResolutionException& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (const ErrorHandling::MergeSectionsException& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (const ErrorHandling::OutputGenerationException& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}