#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <string>
#include <memory>
#include <vector>

#define DEBUG 0
#define LOG(func) \
    if (DEBUG) \
        std::cout << "[" <<__FILE__ << ":" << __LINE__ << "]- "<< func <<"\n";


typedef uint8_t BYTE;
typedef uint32_t ADDRESS;

enum eInstructionIdentifier
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
    CSRRD, CSRWR
};

enum eInstructionType
{
    DIRECTIVE = 0x00,
    LABEL = 0x01,
    BRANCH = 0x02,

    PROCESSOR = 0x03,
    STACK = 0x04,
    DATA = 0x05,
    MEMORY = 0x06,
    SPECIAL = 0x07,
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

enum eRelocationType
{
    REL_NONE,
    REL_LOCAL,
    REL_EXTERN,
    REL_PC_RELATIVE,
    REL_FORWARD_REFERENCING,
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

// Symbol Table definitions
struct Symbol
{
    std::string name;
    std::string section;

    uint32_t offset = 0;
    uint16_t value; // pool entry
    bool defined = false;
    bool isGlobal = false;
    bool isExtern = false;

    Symbol(){};
    Symbol(std::string name, std::string section, uint32_t offset, uint16_t value = 0, bool defined = false, bool isGlobal = false, bool isExtern = false)
        : name(name), section(section), offset(offset), value(value), defined(defined), isGlobal(isGlobal), isExtern(isExtern) {};

    typedef std::shared_ptr<Symbol> s_ptr;
};
typedef std::vector<Symbol::s_ptr> SymbolTable;

Symbol::s_ptr GetSymbol(const SymbolTable& table, const std::string& label);
std::string SymbolTableToString(const SymbolTable& table);

// Symbol table definitions end
// Section definitions

struct Section
{
    std::string name;
    std::vector<BYTE> data;
    uint32_t locationCounter = 0;
    std::vector<BYTE> literalPool;

    void AppendData(const std::vector<BYTE>& data);
    void WriteData(const uint32_t& offset, const std::vector<BYTE>& data);
    void WriteInstructionDisplacement(const uint32_t& offset, const uint16_t& toWrite);

    ADDRESS InsertLiteralInPool(uint32_t value);
    int64_t IsLiteralPresentInPool(const uint32_t value) const;

    static uint32_t AddressToPoolEntry(ADDRESS address);
    static ADDRESS PoolEntryToAddress(uint32_t entry);

    typedef std::shared_ptr<Section> s_ptr;
};
typedef std::vector<Section::s_ptr> SectionTable;

std::string SectionTableToString(const SectionTable& table);

// Section definitions end
// Relocations definitions

struct Relocation
{
    std::string symbolName;
    std::string sectionName;
    uint32_t offset;
    eRelocationType type;

    typedef std::shared_ptr<Relocation> s_ptr;
};

using RelocationTable = std::vector<Relocation::s_ptr>;
using ForwardRefferenceTable = std::vector<Relocation::s_ptr>;

std::string RelocationTableToString(const RelocationTable& table);

// Relocations definitions end

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
};

struct ConditionalJumpOperands
{
    std::string gpr1;
    std::string gpr2;
    ParserOperand operand;
};


// General utility

eGPR GPRStringToEnum(std::string reg);
eCSR CSRStringToEnum(std::string csr);
std::vector<uint8_t> IntToByteArray(const uint32_t& value);
std::vector<uint8_t> ShortToByteArray(const uint16_t& value);
uint32_t LiteralStringToInt(const std::string& value);
bool Compare(const std::vector<uint8_t>& first, const std::vector<uint8_t>& second);

#endif