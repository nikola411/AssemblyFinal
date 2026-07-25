#ifndef ERROR_HANDLING_HPP
#define ERROR_HANDLING_HPP

#include <exception>
#include <map>
#include <string>
#include <vector>

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
    ASM_RESULT_SYMBOL_ALREADY_DEFINED,
    ASM_RESULT_POOL_OUT_OF_REACH,
    ASM_RESULT_WRONG_VALUE,
    ASM_RESULT_WRONG_INSTRUCTION_IDENTIFIER,
    ASM_RESULT_WRONG_ADDRESSING_TYPE,
    ASM_REUSLT_WRONG_PAYLOAD_TYPE,
    ASM_RESULT_WRONG_VALUE_SIZE,
    ASM_RESULT_INDEX_OUT_OF_BOUNDS,
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
    { ASM_RESULT_UNDEFINED_SYMBOL, "Undefined symbols."},
    { ASM_RESULT_SYMBOL_ALREADY_DEFINED, "Symbol already defined." },
    { ASM_RESULT_POOL_OUT_OF_REACH, "Literal pool where symbol value is located is out of reach." },
    { ASM_RESULT_WRONG_INSTRUCTION_IDENTIFIER, "Assembly instruction identifier is not defined in the assembly instructions map." },
    { ASM_RESULT_WRONG_ADDRESSING_TYPE, "Addressing type provided is not supported for the wanted assembly instruction." },
    { ASM_REUSLT_WRONG_PAYLOAD_TYPE, "Assembly instruction for provided addressing type is not supporting given operand type." },
    { ASM_RESULT_WRONG_VALUE_SIZE, "Assembly instruction for provided operand type is not supporting given operand size." },
    { ASM_RESULT_INDEX_OUT_OF_BOUNDS, "Index out of bounds." },
    { ASM_RESULT_WRONG_VALUE, "Wrong value error." },
};


namespace ExceptionMessage
{
    static const std::string UndefinedSymbol = "Symbol {} is undefined.";
    static const std::string LiteralOffsetTooBig = "Literal {} offset is too big.";
    static const std::string EndNotEncountered = ".end directive was not encountered.";
    static const std::string SymbolAlreadyDefined = "Symbol {} definition was encountered before.";
    static const std::string InstructionNotSupported = "Instruction {} is not supported.";
    static const std::string SymbolOffsetTooBig = "Symbol {} offset is too big.";
    static const std::string ExportingUndefinedSymbol = "Symbol {} is being exported but is not defined.";
    static const std::string ImportingDefinedSymbol = "Symbol {} is defined and being imported.";
    static const std::string OutputFileNotOpen = "Output file {} could not be opened.";
    static const std::string SymbolNotConstant = "Symbol {} is not a constant.";
    static const std::string SymbolValueTooBig = "Symbol {} value is too big.";
}

struct AssemblyErrorMetadata
{
    AsmResult statusCode = ASM_RESULT_SUCCESS;
    std::string value;
};

std::string UndefinedSymbolMessage(const std::string& name);

class FatalException
{
public:
    FatalException() = default;
    ~FatalException() = default;
};


class AssemblyException : public std::exception
{
public:
    AssemblyException() {};
    AssemblyException(std::string arg) : message(arg) {};
    AssemblyException(std::string arg, std::vector<std::string> modifiers) : message(arg), mods(modifiers) {}

    const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW final
    {
        if (mods.size() != 0)
        {
            auto newMessage = Format(message, mods);
            return newMessage.c_str();
        }
            

        return message.c_str();
    };

    std::string GetErrorMessage() const 
    {
        if (mods.size() != 0)
        {
            auto newMessage = Format(message, mods);
            return newMessage;
        }
            
        return message;
    }

private:
    static std::string Format(std::string original, std::vector<std::string> modifiers);

    std::vector<std::string> mods;
    std::string message;
};

#endif