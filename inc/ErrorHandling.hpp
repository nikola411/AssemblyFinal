#ifndef ERROR_HANDLING_HPP
#define ERROR_HANDLING_HPP

#include <exception>
#include <map>
#include <string>

enum AsmResult
{
    ASM_RESULT_SUCCESS = 0,
    ASM_RESULT_FAIL,
    ASM_RESULT_LITERAL_OFFSET_TOO_BIG,
    ASM_RESULT_SYMBOL_OFFSET_VALUE_TOO_BIG,
    ASM_RESULT_CODE_MAP_EMPTY,
    ASM_RESULT_CODE_MAP_INVALID_DATA,
    ASM_RESULT_NOT_SUPPORTED,
    ASM_RESULT_END_ENCOUNTERED,
    ASM_RESULT_END_NOT_ENCOUNTERED,
    ASM_RESULT_UNDEFINED_SYMBOL,
    ASM_RESULT_SYMBOL_ALREADY_DEFINED
};

static std::map<AsmResult, std::string> AsmResultToString =
{
    { ASM_RESULT_SUCCESS, "Success" },
    { ASM_RESULT_FAIL, "Fail" },
    { ASM_RESULT_LITERAL_OFFSET_TOO_BIG, "Literal offset too big." },
    { ASM_RESULT_SYMBOL_OFFSET_VALUE_TOO_BIG, "Symbol offset too big." },
    { ASM_RESULT_CODE_MAP_EMPTY, "Codes Map is empty." },
    { ASM_RESULT_CODE_MAP_INVALID_DATA, "Data in Codes Map is invalid." },
    { ASM_RESULT_NOT_SUPPORTED, "Instruction is not supported. " },
    { ASM_RESULT_END_ENCOUNTERED, "End directive encountered." },
    { ASM_RESULT_END_NOT_ENCOUNTERED, "End directive not encountered." },
    { ASM_RESULT_UNDEFINED_SYMBOL, "Undefined symbol."},
    { ASM_RESULT_SYMBOL_ALREADY_DEFINED, "Symbol already defined." },
};

struct AssemblyErrorMetadata
{
    AsmResult statusCode = ASM_RESULT_SUCCESS;
    std::string value;
};

class AssemblyException : public std::exception
{
public:
    AssemblyException() {};
    // AssemblyException(std::string arg) : message(arg) {};
    // AssemblyException(std::string arg, std::vector<std::string> more) : message(arg), more(more) {}

    const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW final
    {
        // std::string error("");
        // error += message;

        // for (const auto& part : more)
        //     error += part;

        // return error.c_str();

        return "Aborting execution.";
    };

private:

    std::string ExpandMore();
    std::string message;
    std::vector<std::string> more;
};

#endif