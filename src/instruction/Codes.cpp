#include "Codes.hpp"

#define DO_NOTHING {}

std::map<eInstructionIdentifier, std::map<eOperandType, std::map<eAddressingType, std::vector<InstructionPopulationMetadata>>>>
CodesMap::InstructionCodesMap = {};

eInstructionIdentifier CodesMap::currentIdentifier = (eInstructionIdentifier)0;
eOperandType CodesMap::currentOperandType = eOperandType::NONE_TYPE;
eAddressingType CodesMap::currentAddressingType = eAddressingType::ADDR_NONE;
bool CodesMap::Initialized = false;
std::vector<InstructionPopulationMetadata> CodesMap::currentEntry = {};

void CodesMap::AddInstructionPair(int code, std::vector<std::pair<ptrInstructionMethod, eValueToUse>> methods)
{
    auto codePopulationVector = std::vector<CodePopulation::s_ptr>();

    if (methods.size() == 0)
        codePopulationVector.push_back(nullptr);

    for (auto method : methods)
    {
        codePopulationVector.push_back(std::make_shared<CodePopulation>(method.first, method.second));
    }

    Instruction instruction;
    instruction.SetData(code);
    currentEntry.push_back(std::pair<Instruction, std::vector<CodePopulation::s_ptr>>(instruction, codePopulationVector));
}

void CodesMap::SetMapEntry(eInstructionIdentifier identifier, eOperandType operand, eAddressingType addressing)
{
    currentIdentifier = identifier;
    currentOperandType = operand;
    currentAddressingType = addressing;

    InstructionCodesMap[identifier];
    InstructionCodesMap[identifier][operand];

    currentEntry = InstructionCodesMap[identifier][operand][addressing];
}

void CodesMap::AddMapEntry()
{
    InstructionCodesMap[currentIdentifier][currentOperandType][currentAddressingType] = currentEntry;
}

void CodesMap::PopulateMap()
{
    if (Initialized)
        return;
    
    { // program control methods population
        SetMapEntry(eInstructionIdentifier::HALT, eOperandType::NONE_TYPE, eAddressingType::ADDR_NONE);
        AddInstructionPair(0x00000000, DO_NOTHING);
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::INT, eOperandType::NONE_TYPE, eAddressingType::ADDR_NONE);
        AddInstructionPair(0x10000000, DO_NOTHING);
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::IRET, eOperandType::NONE_TYPE, eAddressingType::ADDR_NONE);
        //AddInstructionPair(0x93FE0004, DO_NOTHING);
        AddInstructionPair(0x93FE0004, DO_NOTHING);
        AddInstructionPair(0x970E0004, DO_NOTHING); // pop status
        AddMapEntry();

        // fix for dumb project spec
        // SetMapEntry(eInstructionIdentifier::IRET, eOperandType::NONE_TYPE, eAddressingType::ADDR_NONE);
        // AddInstructionPair(0x970E0004, DO_NOTHING); // pop status
        // AddInstructionPair(0x93FE0004, DO_NOTHING);
        // AddMapEntry();

        SetMapEntry(eInstructionIdentifier::RET, eOperandType::NONE_TYPE, eAddressingType::ADDR_NONE);
        AddInstructionPair(0x93FE0004, DO_NOTHING);
        AddMapEntry();
    }

    { // LD map population
        /*
            ld operand, %gpr ; gpr <= operand
        */
        SetMapEntry(eInstructionIdentifier::LD, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x91000000,
            {
               { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
               { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::LD, eOperandType::GPR, eAddressingType::ADDR_MEMORY);
        AddInstructionPair(0x92000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::LD, eOperandType::GPR, eAddressingType::ADDR_MEMORY_OFFSET);
        AddInstructionPair(0x92000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetDisplacement, eValueToUse::FIRST_OFFSET }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::LD, eOperandType::SYM, eAddressingType::ADDR_MEMORY);
        AddInstructionPair(0x920F0000, // F for pc because of PC rel addressing
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetDisplacement, eValueToUse::FIRST_OPERAND }
            });
        AddInstructionPair(0x92000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::LD, eOperandType::LTR, eAddressingType::ADDR_MEMORY);
        AddInstructionPair(0x920F0000, // F for pc because of PC rel addressing
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetDisplacement, eValueToUse::FIRST_OPERAND }
            });
        AddInstructionPair(0x92000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::LD, eOperandType::SYM, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x920F0000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetDisplacement, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::LD, eOperandType::LTR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x920F0000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetDisplacement, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();
    }

    { // ST Map population
        /*
     initialized   */
        SetMapEntry(eInstructionIdentifier::ST, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x91000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::ST, eOperandType::GPR, eAddressingType::ADDR_MEMORY);
        AddInstructionPair(0x80000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::ST, eOperandType::GPR, eAddressingType::ADDR_MEMORY_OFFSET);
        AddInstructionPair(0x80000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetDisplacement, eValueToUse::SECOND_OFFSET }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::ST, eOperandType::SYM, eAddressingType::ADDR_MEMORY);
        AddInstructionPair(0x82F00000,
            {
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetDisplacement, eValueToUse::SECOND_OPERAND },
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::ST, eOperandType::LTR, eAddressingType::ADDR_MEMORY);
        AddInstructionPair(0x82F00000,
            {
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetDisplacement, eValueToUse::SECOND_OPERAND },
            });
        AddMapEntry();
    }

    { // PUSH and POP map population
        SetMapEntry(eInstructionIdentifier::PUSH, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x81E00FFC, { { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND } });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::POP, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x930E0004, { { &Instruction::SetRegisterA, eValueToUse::FIRST_OPERAND } });
        AddMapEntry();
    }

    { // Branch methods map population
        /*
            push pc; pc <= operand;
        */
        SetMapEntry(eInstructionIdentifier::CALL, eOperandType::SYM, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x21F00000,
            {
                { &Instruction::SetDisplacement, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::CALL, eOperandType::LTR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x21F00000,
            {
                { &Instruction::SetDisplacement, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();
        /*
            pc <= operand;
        */
        SetMapEntry(eInstructionIdentifier::JMP, eOperandType::SYM, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x38F00000,
            {
                { &Instruction::SetDisplacement, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::JMP, eOperandType::LTR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x38F00000,
            {
                { &Instruction::SetDisplacement, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();
        /*
            if (gpr1 == gpr2) pc <= operand;
        */
        SetMapEntry(eInstructionIdentifier::BEQ, eOperandType::SYM, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x39F00000,
            {
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetDisplacement,   eValueToUse::THIRD_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::BEQ, eOperandType::LTR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x39F00000,
            {
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetDisplacement,   eValueToUse::THIRD_OPERAND }
            });
        AddMapEntry();
        /*
            if (gpr1 != gpr2) pc <= operand;
        */
        SetMapEntry(eInstructionIdentifier::BNE, eOperandType::SYM, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x3AF00000,
            {
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetDisplacement,   eValueToUse::THIRD_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::BNE, eOperandType::LTR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x3AF00000,
            {
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetDisplacement,   eValueToUse::THIRD_OPERAND }
            });
        AddMapEntry();
        /*
            if (gpr1 signed > gpr2) pc <= operand;
        */
        SetMapEntry(eInstructionIdentifier::BGT, eOperandType::SYM, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x3BF00000,
            {
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetDisplacement,   eValueToUse::THIRD_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::BGT, eOperandType::LTR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x3BF00000,
            {
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetDisplacement,   eValueToUse::THIRD_OPERAND }
            });
        AddMapEntry();
    }

    { // XCHG and arithmetic methods map population
        SetMapEntry(eInstructionIdentifier::XCHG, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x40000000,
            {
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::SECOND_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::ADD, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x50000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::SUB, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x51000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::MUL, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x52000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::DIV, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x53000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();
    }

    { // Logical methods map population
        SetMapEntry(eInstructionIdentifier::NOT, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x60000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::FIRST_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND },
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::AND, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x61000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::OR, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x62000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::XOR, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x63000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();
    }

    { // SHL SHR methods map population
        SetMapEntry(eInstructionIdentifier::SHL, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x70000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::SHR, eOperandType::GPR, eAddressingType::ADDR_DIRECT);
        AddInstructionPair(0x71000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterC, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();
    }

    { // CSRRD and CSRWR
        SetMapEntry(eInstructionIdentifier::CSRRD, eOperandType::CSR, eAddressingType::ADDR_DIRECT); // gpr <= csr
        AddInstructionPair(0x90000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();

        SetMapEntry(eInstructionIdentifier::CSRWR, eOperandType::CSR, eAddressingType::ADDR_DIRECT); // csr <= gpr
        AddInstructionPair(0x95000000,
            {
                { &Instruction::SetRegisterA, eValueToUse::SECOND_OPERAND },
                { &Instruction::SetRegisterB, eValueToUse::FIRST_OPERAND }
            });
        AddMapEntry();
    }

    Initialized = true;
}

int CodesMap::GetInstructionCount(eInstructionIdentifier identifier, eOperandType operand, eAddressingType addressing)
{
    auto instructionVector = InstructionCodesMap[identifier][operand][addressing];
    return instructionVector.size();
}

std::vector<InstructionPopulationMetadata>
CodesMap::GetInstructionCodes(eInstructionIdentifier identifier, eOperandType operand, eAddressingType addressing)
{
    return InstructionCodesMap[identifier][operand][addressing];
}