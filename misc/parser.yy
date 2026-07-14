%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.5.1"
%language "C++"
%defines "./parser/parser.hpp"
%output  "./parser/parser.cpp"

%defines
%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires
{
    #include "AssemblyAdapter.hpp"

    class Driver;
    class AssemblyAdapter;

    using std::string;
    using std::vector;
    using std::pair;
}

%param { Driver& drv }
%param { AssemblyAdapter& assembly }
%locations

%define parse.trace
%define parse.error verbose
%define parse.lac full

%code
{
    #include <memory>
    #include <vector>
    #include <string>

    #include "ParserTypes.hpp"
    #include "Utility.hpp"
    #include "Driver.hpp"
}

%define api.token.prefix {TOK_}

//--------------------NONTERMINALS--------------------
%nterm NT_Program;
%nterm NT_Line;

%type <std::vector<ParserOperand>> NT_SymbolList;
%type <std::vector<ParserOperand>> NT_LiteralList;

%nterm NT_Directive;
%type <std::string> NT_DirectiveWithSymbolList;
%type <std::string> NT_DirectiveWithList;
%type <std::string> NT_DirectiveSingleArgument;
%type <eAssemblyIdentifier> NT_DirectiveIdentifier;

%nterm NT_ProcessorInstruction;

%nterm NT_BranchInstruction;
%type <eAssemblyIdentifier> NT_ConditionalJumpIdentifier;
%type <eAssemblyIdentifier> NT_UnconditionalJumpIdentifier;
%type <ConditionalJumpOperands> NT_ConditionalJumpOperands;

%nterm NT_StackInstruction;
%type <eAssemblyIdentifier> NT_StackInstructionIdentifier;

%nterm NT_DataInstruction;
%type <eAssemblyIdentifier> NT_DataInstructionIdentifier;
%type <std::vector<ParserOperand>> NT_DataInstructionOperands;

%nterm NT_MemoryInstruction;
%nterm NT_SpecialInstruction;

%nterm NT_LabelAndComment;

%nterm <ParserOperand> NT_Operand;
%nterm <ParserOperand> NT_JumpOperand;

//--------------------TERMINALS-----------------------
%token GLOBAL EXTERN SECTION WORD SKIP ASCII EQU END;
%token HALT INT IRET RET;
%token JMP CALL;
%token PUSH POP;
%token BEQ BNE BGT;
%token XCHG ADD SUB MUL DIV NOT AND OR XOR SHL SHR;
%token LD ST;
%token CSRRD CSRWR;
%token <std::string> LABEL;
%token <std::string> SYMBOL;
%token <std::string> LITERAL;
%token <std::string> GPR;
%token <std::string> CSR;

%token DOLLAR "$" R_BRACKET "]" L_BRACKET "[";
%token L_PAREN "(" R_PAREN ")";
%token PERCENT "%" PLUS "+" MINUS "-";
%token COMMA "," COLON ":";
%token COMMENT;
%token EOF 0 "end of file";

%%

NT_Program:
    NT_Program NT_Line
    {
        try
        {
            assembly.FinishInstruction();
        }
        catch (AssemblyException& e)
        {
            yy::parser::error(@2, std::string(e.GetErrorMessage()));
            throw FatalException();
        }
    }
    | NT_Line
    {
        try
        {
            assembly.FinishInstruction();
        }
        catch (AssemblyException& e)
        {
            yy::parser::error(@1, std::string(e.GetErrorMessage()));
            throw FatalException();
        }
    }
    | EOF
    ;

NT_Line:
    NT_Directive
    | NT_ProcessorInstruction
    | NT_BranchInstruction
    | NT_StackInstruction
    | NT_DataInstruction
    | NT_MemoryInstruction
    | NT_SpecialInstruction
    | NT_LabelAndComment
    ;

NT_LabelAndComment:
    COMMENT
    | LABEL
    {
        assembly.SetInstruction(eAssemblyIdentifier::LBL, eAssemblyIdentifierType::LABEL);
        assembly.SetOperand($1, eOperandType::SYM);
    }
    ;
//-------------------DIRECTIVES-------------------
NT_Directive:
    NT_DirectiveWithSymbolList
    | NT_DirectiveWithList
    | NT_DirectiveSingleArgument
    | END
    {
        assembly.SetInstruction(eAssemblyIdentifier::END, eAssemblyIdentifierType::DIRECTIVE);

    }
    ;

NT_DirectiveWithList:
    WORD NT_LiteralList
    {
        assembly.SetInstruction(eAssemblyIdentifier::WORD, eAssemblyIdentifierType::DIRECTIVE);
        assembly.SetMultipleOperands($2);

    }
    | WORD NT_SymbolList
    {
        assembly.SetInstruction(eAssemblyIdentifier::WORD, eAssemblyIdentifierType::DIRECTIVE);
        assembly.SetMultipleOperands($2);
    }
    ;

NT_DirectiveSingleArgument:
    SECTION SYMBOL
    {
        assembly.SetInstruction(eAssemblyIdentifier::SECTION, eAssemblyIdentifierType::DIRECTIVE);
        assembly.SetOperand($2, eOperandType::SYM);
    }
    | SKIP LITERAL
    {
        assembly.SetInstruction(eAssemblyIdentifier::SKIP, eAssemblyIdentifierType::DIRECTIVE);
        assembly.SetOperand($2, eOperandType::LTR);
    }
    ;

NT_DirectiveWithSymbolList:
    NT_DirectiveIdentifier NT_SymbolList
    {
        assembly.SetInstruction($1, eAssemblyIdentifierType::DIRECTIVE);
        assembly.SetMultipleOperands($2);
    }
    ;

NT_DirectiveIdentifier:
    GLOBAL { $$ = eAssemblyIdentifier::GLOBAL; }
    | EXTERN { $$ = eAssemblyIdentifier::EXTERN; }
    ;
//---------------------DIRECTIVES-END----------------
//---------------------PROCESSOR-INSTR---------------
NT_ProcessorInstruction:
    HALT { assembly.SetInstruction(eAssemblyIdentifier::HALT, eAssemblyIdentifierType::PROCESSOR); }
    | INT { assembly.SetInstruction(eAssemblyIdentifier::INT, eAssemblyIdentifierType::PROCESSOR); }
    | IRET { assembly.SetInstruction(eAssemblyIdentifier::IRET, eAssemblyIdentifierType::PROCESSOR); }
    | RET { assembly.SetInstruction(eAssemblyIdentifier::RET, eAssemblyIdentifierType::PROCESSOR); }
    ;
//---------------------PROCESSOR-END-----------------
//---------------------BRANCH------------------------
NT_BranchInstruction:
    NT_ConditionalJumpIdentifier NT_ConditionalJumpOperands
    {
        assembly.SetInstruction($1, eAssemblyIdentifierType::BRANCH);
        assembly.SetOperand($2.gpr1, eOperandType::GPR);
        assembly.SetOperand($2.gpr2, eOperandType::GPR);
        assembly.SetOperand($2.operand);
    }
    | NT_UnconditionalJumpIdentifier NT_JumpOperand
    {
        assembly.SetInstruction($1, eAssemblyIdentifierType::BRANCH);
        assembly.SetOperand($2);
    }
    ;

NT_ConditionalJumpIdentifier:
    BEQ { $$ = eAssemblyIdentifier::BEQ; }
    | BNE { $$ = eAssemblyIdentifier::BNE; }
    | BGT { $$ = eAssemblyIdentifier::BGT; }
    ;

NT_ConditionalJumpOperands:
    "%" GPR COMMA "%" GPR COMMA NT_JumpOperand
    {
        $$.gpr1 = $2;
        $$.gpr2 = $5;
        $$.operand = $7;
    }
    ;

NT_UnconditionalJumpIdentifier:
    JMP { $$ = eAssemblyIdentifier::JMP; }
    | CALL { $$ = eAssemblyIdentifier::CALL; }
    ;
//---------------------BRANCH-END--------------------
//---------------------DATA--------------------------
NT_DataInstruction:
    NT_DataInstructionIdentifier NT_DataInstructionOperands
    {
        if ($1 == eAssemblyIdentifier::NOT && $2.size() > 1)
        {
            error(@1, "Wrong number of arguments for instruction not.\n");
            return 1;
        }

        assembly.SetInstruction($1, eAssemblyIdentifierType::DATA);
        assembly.SetMultipleOperands($2);
    }
    ;

NT_DataInstructionIdentifier:
    XCHG { $$ = eAssemblyIdentifier::XCHG; }
    | ADD { $$ = eAssemblyIdentifier::ADD; }
    | SUB { $$ = eAssemblyIdentifier::SUB; }
    | MUL { $$ = eAssemblyIdentifier::MUL; }
    | DIV { $$ = eAssemblyIdentifier::DIV; }
    | NOT { $$ = eAssemblyIdentifier::NOT; }
    | AND { $$ = eAssemblyIdentifier::AND; }
    | OR  { $$ = eAssemblyIdentifier::OR;  }
    | XOR { $$ = eAssemblyIdentifier::XOR; }
    | SHL { $$ = eAssemblyIdentifier::SHL; }
    | SHR { $$ = eAssemblyIdentifier::SHR; }
    ;

NT_DataInstructionOperands:
    "%" GPR "," "%" GPR
    {
        $$.push_back(ParserOperand($2, eOperandType::GPR));
        $$.push_back(ParserOperand($5, eOperandType::GPR));
    }
    | "%" GPR { $$.push_back(ParserOperand($2, eOperandType::GPR)); }
    ;
//---------------------DATA-END----------------------
//---------------------MEMORY------------------------
NT_MemoryInstruction:
    LD NT_Operand "," "%" GPR
    {
        assembly.SetInstruction(eAssemblyIdentifier::LD, eAssemblyIdentifierType::MEMORY);
        assembly.SetOperand($2);
        assembly.SetOperand($5, eOperandType::GPR);
    }
    | ST "%" GPR "," NT_Operand
    {
        assembly.SetInstruction(eAssemblyIdentifier::ST, eAssemblyIdentifierType::MEMORY);
        assembly.SetOperand($3, eOperandType::GPR);
        assembly.SetOperand($5);
    }
    ;
//---------------------MEMORY-END--------------------
//---------------------SPECIAL-----------------------
NT_SpecialInstruction:
    CSRRD "%" CSR "," "%" GPR
    {
        assembly.SetInstruction(eAssemblyIdentifier::CSRRD, eAssemblyIdentifierType::SPECIAL);
        assembly.SetOperand($3, eOperandType::CSR);
        assembly.SetOperand($6, eOperandType::GPR);
    }
    | CSRWR "%" GPR "," "%" CSR
    {
        assembly.SetInstruction(eAssemblyIdentifier::CSRWR, eAssemblyIdentifierType::SPECIAL);
        assembly.SetOperand($3, eOperandType::GPR);
        assembly.SetOperand($6, eOperandType::CSR);
    }
    ;
//---------------------SPECIAL-END-------------------
//---------------------STACK-------------------------
NT_StackInstruction:
    NT_StackInstructionIdentifier "%" GPR
    {
        assembly.SetInstruction($1, eAssemblyIdentifierType::STACK);
        assembly.SetOperand($3, eOperandType::GPR);
    }
    ;

NT_StackInstructionIdentifier:
    PUSH { $$ = eAssemblyIdentifier::PUSH; }
    | POP { $$ = eAssemblyIdentifier::POP; }
    ;
//---------------------STACK-END---------------------
//---------------------LISTS-------------------------
NT_SymbolList:
    NT_SymbolList COMMA SYMBOL { $1.push_back(ParserOperand($3, eOperandType::SYM)); $$ = $1; }
    | SYMBOL { $$.push_back(ParserOperand($1, eOperandType::SYM)); }
    ;

NT_LiteralList:
    NT_LiteralList COMMA LITERAL { $1.push_back(ParserOperand($3, eOperandType::LTR)); $$ = $1; }
    | LITERAL { $$.push_back(ParserOperand($1, eOperandType::LTR)); }
    ;

//---------------------LISTS-END---------------------
//---------------------OPERANDS----------------------
NT_JumpOperand:
    SYMBOL    { $$.value = $1; $$.type = eOperandType::SYM; $$.addressingType = eAddressingType::ADDR_DIRECT; }
    | LITERAL { $$.value = $1; $$.type = eOperandType::LTR; $$.addressingType = eAddressingType::ADDR_DIRECT; }
    ;

NT_Operand:
    "$" LITERAL { $$.value = $2; $$.type = eOperandType::LTR; $$.addressingType = eAddressingType::ADDR_DIRECT; }
    | "$" SYMBOL { $$.value = $2; $$.type = eOperandType::SYM; $$.addressingType = eAddressingType::ADDR_DIRECT; }
    | LITERAL { $$.value = $1; $$.type = eOperandType::LTR; $$.addressingType = eAddressingType::ADDR_MEMORY; }
    | SYMBOL { $$.value = $1; $$.type = eOperandType::SYM; $$.addressingType = eAddressingType::ADDR_MEMORY; }
    | "%" GPR { $$.value = $2; $$.type = eOperandType::GPR; $$.addressingType = eAddressingType::ADDR_DIRECT; }
    | "[" "%" GPR "]" { $$.value = $3; $$.type = eOperandType::GPR; $$.addressingType = eAddressingType::ADDR_MEMORY; }
    | "[" "%" GPR "+" LITERAL "]" { $$.value = $3; $$.offset = $5; $$.offsetType = eOperandType::LTR; $$.type = eOperandType::GPR; $$.addressingType = eAddressingType::ADDR_MEMORY_OFFSET; }
    | "[" "%" GPR "+" SYMBOL "]"  { $$.value = $3; $$.offset = $5; $$.offsetType = eOperandType::SYM; $$.type = eOperandType::GPR; $$.addressingType = eAddressingType::ADDR_MEMORY_OFFSET; }
    ;
//---------------------OPERANDS-END------------------

%%

void yy::parser::error (const location_type& l, const std::string& m)
{
    std::cerr << "\033[31m ERROR:" << l << ": " << m << '\n';
}