#ifndef HELPERS_HPP
#define HELPERS_HPP

#include "Symbol.hpp"
#include "Section.hpp"

#include <string>
#include <vector>
#include <memory>

enum States
{
    READ_NEXT = 0,
    SET_HEX,
    SET_RELOC,
    SET_PLACE,
    SET_INPUT,
    SET_OUTPUT,
    FINISH,
    ERROR
};

enum Modes
{
    HEX = 1,
    RELOC = 2
};

struct AbstractFile
{
    SymbolTable symTable;
    SectionTable sections;
};


class Linker;
int ParseArguments(std::shared_ptr<Linker> linker, int argc, char* argv[]);

namespace ErrorHandling
{
    class SymbolResolutionException : public std::exception
    {
    public:
        explicit SymbolResolutionException(const std::string& msg) : message("SymbolResolutionException: " + msg) {}
        const char* what() const noexcept override { return message.c_str(); }
    private:
        std::string message;
    };

    class RelocationResolutionException : public std::exception
    {
    public:
        explicit RelocationResolutionException(const std::string& msg) : message("RelocationResolutionException: " + msg) {}
        const char* what() const noexcept override { return message.c_str(); }
    private:
        std::string message;
    };

    class MergeSectionsException : public std::exception
    {
    public:
        explicit MergeSectionsException(const std::string& msg) : message("MergeSectionsException: " + msg) {}
        const char* what() const noexcept override { return message.c_str(); }
    private:
        std::string message;
    };

    class OutputGenerationException : public std::exception
    {
    public:
        explicit OutputGenerationException(const std::string& msg) : message("OutputGenerationException: " + msg) {}
        const char* what() const noexcept override { return message.c_str(); }
    private:
        std::string message;
    };
}




#endif