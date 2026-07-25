#ifndef PARSER_TYPES_HPP
#define PARSER_TYPES_HPP

#include <inttypes.h>
#include <string>

typedef uint8_t BYTE;
typedef uint32_t ADDRESS;


enum eAssemblyIdentifier
{
    // directive
    GLOBAL, EXTERN, WORD, SECTION, SKIP, LBL, END,
    // branch
    JMP, CALL,
    BEQ, BNE, BGT,
    // processor
    HALT, INT, IRET, RET,
    // stack
    PUSH, POP,
    // data
    XCHG, ADD, SUB, MUL, DIV, NOT, AND, OR, XOR, SHL, SHR,
    // memory
    LD, ST,
    // special
    CSRRD, CSRWR,
    // equ, ascii
    EQU
};

/*
    Podela instrukcija: 
    DIRECTIVE - asemblerske direktive
    LABEL - labele
    OPERAND - sve instrukcije koje koriste operande
    REGISTER - instrukcije koje koriste samo registre
*/
enum eAssemblyIdentifierType
{
    DIRECTIVE = 0x01,
    LABEL = 0x02,
    DATA = 0x04,
    BRANCH = 0x08,
    REGISTER = 0x10,
    NONE = 0x20
};

enum eOperandType
{
    NONE_TYPE = 0,
    GPR,
    CSR,
    SYM,
    LTR,
    COMPOUND,
};

enum eAddressingType
{
    ADDR_NONE = 0,
    ADDR_DIRECT,
    ADDR_MEMORY,
    ADDR_MEMORY_OFFSET
};

enum eRelocationType : uint32_t
{
    REL12_ABS = 1, // upisivanje 12bit vrednosti
    REL12_PC = 2, // upisivanje 12bit pomeraja do bazena literala
    REL32_ABS = 3  // upisivanje 32bit vrenodnosti
};

enum eGPR
{
    R0 = 0x00, R1 = 0x01, R2 = 0x02, R3 = 0x03,
    R4 = 0x04, R5 = 0x05, R6 = 0x06, R7 = 0x07,
    R8 = 0x08, R9 = 0x09, R10 = 0x0A, R11 = 0x0B,
    R12 = 0x0C, R13 = 0x0D, R14 = 0x0E, R15 = 0x0F
};

enum eCSR
{
    STATUS = 0x00, HANDLER = 0x01, CAUSE = 0x02
};

struct ParserOperand
{
/// @brief Parser data
    std::string value;
    eOperandType type;

    std::string offset;
    eOperandType offsetType;

    eAddressingType addressingType;

    ParserOperand() = default;
    ParserOperand(std::string value, eOperandType type) : value(value), type(type) {};
    ParserOperand(std::string value, eOperandType type, std::string offset, eOperandType offsetType, eAddressingType addressingType) :
        value(value), type(type), offset(offset), offsetType(offsetType), addressingType(addressingType)
    {}

/// @brief ASM data
    uint16_t asmValue = 0;
    uint16_t asmOffset = 0;
    bool isBigValue = false; // da li je asmValue pool offset (instrukcija treba SIZE_BIG enkodiranje)
};

struct ConditionalJumpOperands
{
    std::string gpr1;
    std::string gpr2;
    ParserOperand operand;
};

#endif