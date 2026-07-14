// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "parser.hpp"


// Unqualified %code blocks.
#line 34 "./parser.yy"

    #include <memory>
    #include <vector>
    #include <string>

    #include "ParserTypes.hpp"
    #include "Utility.hpp"
    #include "Driver.hpp"

#line 56 "./parser/parser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {
#line 148 "./parser/parser.cpp"

  /// Build a parser object.
  parser::parser (Driver& drv_yyarg, AssemblyAdapter& assembly_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      yy_lac_established_ (false),
      drv (drv_yyarg),
      assembly (assembly_yyarg)
  {}

  parser::~parser ()
  {}

  parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/



  // by_state.
  parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  parser::symbol_kind_type
  parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  parser::stack_symbol_type::stack_symbol_type ()
  {}

  parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_NT_ConditionalJumpOperands: // NT_ConditionalJumpOperands
        value.YY_MOVE_OR_COPY< ConditionalJumpOperands > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NT_JumpOperand: // NT_JumpOperand
      case symbol_kind::S_NT_Operand: // NT_Operand
        value.YY_MOVE_OR_COPY< ParserOperand > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NT_DirectiveIdentifier: // NT_DirectiveIdentifier
      case symbol_kind::S_NT_ConditionalJumpIdentifier: // NT_ConditionalJumpIdentifier
      case symbol_kind::S_NT_UnconditionalJumpIdentifier: // NT_UnconditionalJumpIdentifier
      case symbol_kind::S_NT_DataInstructionIdentifier: // NT_DataInstructionIdentifier
      case symbol_kind::S_NT_StackInstructionIdentifier: // NT_StackInstructionIdentifier
        value.YY_MOVE_OR_COPY< eAssemblyIdentifier > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_LABEL: // LABEL
      case symbol_kind::S_SYMBOL: // SYMBOL
      case symbol_kind::S_LITERAL: // LITERAL
      case symbol_kind::S_GPR: // GPR
      case symbol_kind::S_CSR: // CSR
      case symbol_kind::S_NT_DirectiveWithList: // NT_DirectiveWithList
      case symbol_kind::S_NT_DirectiveSingleArgument: // NT_DirectiveSingleArgument
      case symbol_kind::S_NT_DirectiveWithSymbolList: // NT_DirectiveWithSymbolList
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NT_DataInstructionOperands: // NT_DataInstructionOperands
      case symbol_kind::S_NT_SymbolList: // NT_SymbolList
      case symbol_kind::S_NT_LiteralList: // NT_LiteralList
        value.YY_MOVE_OR_COPY< std::vector<ParserOperand> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_NT_ConditionalJumpOperands: // NT_ConditionalJumpOperands
        value.move< ConditionalJumpOperands > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NT_JumpOperand: // NT_JumpOperand
      case symbol_kind::S_NT_Operand: // NT_Operand
        value.move< ParserOperand > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NT_DirectiveIdentifier: // NT_DirectiveIdentifier
      case symbol_kind::S_NT_ConditionalJumpIdentifier: // NT_ConditionalJumpIdentifier
      case symbol_kind::S_NT_UnconditionalJumpIdentifier: // NT_UnconditionalJumpIdentifier
      case symbol_kind::S_NT_DataInstructionIdentifier: // NT_DataInstructionIdentifier
      case symbol_kind::S_NT_StackInstructionIdentifier: // NT_StackInstructionIdentifier
        value.move< eAssemblyIdentifier > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_LABEL: // LABEL
      case symbol_kind::S_SYMBOL: // SYMBOL
      case symbol_kind::S_LITERAL: // LITERAL
      case symbol_kind::S_GPR: // GPR
      case symbol_kind::S_CSR: // CSR
      case symbol_kind::S_NT_DirectiveWithList: // NT_DirectiveWithList
      case symbol_kind::S_NT_DirectiveSingleArgument: // NT_DirectiveSingleArgument
      case symbol_kind::S_NT_DirectiveWithSymbolList: // NT_DirectiveWithSymbolList
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NT_DataInstructionOperands: // NT_DataInstructionOperands
      case symbol_kind::S_NT_SymbolList: // NT_SymbolList
      case symbol_kind::S_NT_LiteralList: // NT_LiteralList
        value.move< std::vector<ParserOperand> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_NT_ConditionalJumpOperands: // NT_ConditionalJumpOperands
        value.copy< ConditionalJumpOperands > (that.value);
        break;

      case symbol_kind::S_NT_JumpOperand: // NT_JumpOperand
      case symbol_kind::S_NT_Operand: // NT_Operand
        value.copy< ParserOperand > (that.value);
        break;

      case symbol_kind::S_NT_DirectiveIdentifier: // NT_DirectiveIdentifier
      case symbol_kind::S_NT_ConditionalJumpIdentifier: // NT_ConditionalJumpIdentifier
      case symbol_kind::S_NT_UnconditionalJumpIdentifier: // NT_UnconditionalJumpIdentifier
      case symbol_kind::S_NT_DataInstructionIdentifier: // NT_DataInstructionIdentifier
      case symbol_kind::S_NT_StackInstructionIdentifier: // NT_StackInstructionIdentifier
        value.copy< eAssemblyIdentifier > (that.value);
        break;

      case symbol_kind::S_LABEL: // LABEL
      case symbol_kind::S_SYMBOL: // SYMBOL
      case symbol_kind::S_LITERAL: // LITERAL
      case symbol_kind::S_GPR: // GPR
      case symbol_kind::S_CSR: // CSR
      case symbol_kind::S_NT_DirectiveWithList: // NT_DirectiveWithList
      case symbol_kind::S_NT_DirectiveSingleArgument: // NT_DirectiveSingleArgument
      case symbol_kind::S_NT_DirectiveWithSymbolList: // NT_DirectiveWithSymbolList
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_NT_DataInstructionOperands: // NT_DataInstructionOperands
      case symbol_kind::S_NT_SymbolList: // NT_SymbolList
      case symbol_kind::S_NT_LiteralList: // NT_LiteralList
        value.copy< std::vector<ParserOperand> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_NT_ConditionalJumpOperands: // NT_ConditionalJumpOperands
        value.move< ConditionalJumpOperands > (that.value);
        break;

      case symbol_kind::S_NT_JumpOperand: // NT_JumpOperand
      case symbol_kind::S_NT_Operand: // NT_Operand
        value.move< ParserOperand > (that.value);
        break;

      case symbol_kind::S_NT_DirectiveIdentifier: // NT_DirectiveIdentifier
      case symbol_kind::S_NT_ConditionalJumpIdentifier: // NT_ConditionalJumpIdentifier
      case symbol_kind::S_NT_UnconditionalJumpIdentifier: // NT_UnconditionalJumpIdentifier
      case symbol_kind::S_NT_DataInstructionIdentifier: // NT_DataInstructionIdentifier
      case symbol_kind::S_NT_StackInstructionIdentifier: // NT_StackInstructionIdentifier
        value.move< eAssemblyIdentifier > (that.value);
        break;

      case symbol_kind::S_LABEL: // LABEL
      case symbol_kind::S_SYMBOL: // SYMBOL
      case symbol_kind::S_LITERAL: // LITERAL
      case symbol_kind::S_GPR: // GPR
      case symbol_kind::S_CSR: // CSR
      case symbol_kind::S_NT_DirectiveWithList: // NT_DirectiveWithList
      case symbol_kind::S_NT_DirectiveSingleArgument: // NT_DirectiveSingleArgument
      case symbol_kind::S_NT_DirectiveWithSymbolList: // NT_DirectiveWithSymbolList
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_NT_DataInstructionOperands: // NT_DataInstructionOperands
      case symbol_kind::S_NT_SymbolList: // NT_SymbolList
      case symbol_kind::S_NT_LiteralList: // NT_LiteralList
        value.move< std::vector<ParserOperand> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  parser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::operator() ()
  {
    return parse ();
  }

  int
  parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // Discard the LAC context in case there still is one left from a
    // previous invocation.
    yy_lac_discard_ ("init");

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (drv, assembly));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        if (!yy_lac_establish_ (yyla.kind ()))
          goto yyerrlab;
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        if (!yy_lac_establish_ (yyla.kind ()))
          goto yyerrlab;

        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    yy_lac_discard_ ("shift");
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_NT_ConditionalJumpOperands: // NT_ConditionalJumpOperands
        yylhs.value.emplace< ConditionalJumpOperands > ();
        break;

      case symbol_kind::S_NT_JumpOperand: // NT_JumpOperand
      case symbol_kind::S_NT_Operand: // NT_Operand
        yylhs.value.emplace< ParserOperand > ();
        break;

      case symbol_kind::S_NT_DirectiveIdentifier: // NT_DirectiveIdentifier
      case symbol_kind::S_NT_ConditionalJumpIdentifier: // NT_ConditionalJumpIdentifier
      case symbol_kind::S_NT_UnconditionalJumpIdentifier: // NT_UnconditionalJumpIdentifier
      case symbol_kind::S_NT_DataInstructionIdentifier: // NT_DataInstructionIdentifier
      case symbol_kind::S_NT_StackInstructionIdentifier: // NT_StackInstructionIdentifier
        yylhs.value.emplace< eAssemblyIdentifier > ();
        break;

      case symbol_kind::S_LABEL: // LABEL
      case symbol_kind::S_SYMBOL: // SYMBOL
      case symbol_kind::S_LITERAL: // LITERAL
      case symbol_kind::S_GPR: // GPR
      case symbol_kind::S_CSR: // CSR
      case symbol_kind::S_NT_DirectiveWithList: // NT_DirectiveWithList
      case symbol_kind::S_NT_DirectiveSingleArgument: // NT_DirectiveSingleArgument
      case symbol_kind::S_NT_DirectiveWithSymbolList: // NT_DirectiveWithSymbolList
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_NT_DataInstructionOperands: // NT_DataInstructionOperands
      case symbol_kind::S_NT_SymbolList: // NT_SymbolList
      case symbol_kind::S_NT_LiteralList: // NT_LiteralList
        yylhs.value.emplace< std::vector<ParserOperand> > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // NT_Program: NT_Program NT_Line
#line 107 "./parser.yy"
    {
        try
        {
            assembly.FinishInstruction();
        }
        catch (AssemblyException& e)
        {
            yy::parser::error(yystack_[0].location, std::string(e.GetErrorMessage()));
            throw FatalException();
        }
    }
#line 734 "./parser/parser.cpp"
    break;

  case 3: // NT_Program: NT_Line
#line 119 "./parser.yy"
    {
        try
        {
            assembly.FinishInstruction();
        }
        catch (AssemblyException& e)
        {
            yy::parser::error(yystack_[0].location, std::string(e.GetErrorMessage()));
            throw FatalException();
        }
    }
#line 750 "./parser/parser.cpp"
    break;

  case 14: // NT_LabelAndComment: LABEL
#line 147 "./parser.yy"
    {
        assembly.SetInstruction(eAssemblyIdentifier::LBL, eAssemblyIdentifierType::LABEL);
        assembly.SetOperand(yystack_[0].value.as < std::string > (), eOperandType::SYM);
    }
#line 759 "./parser/parser.cpp"
    break;

  case 18: // NT_Directive: END
#line 158 "./parser.yy"
    {
        assembly.SetInstruction(eAssemblyIdentifier::END, eAssemblyIdentifierType::DIRECTIVE);

    }
#line 768 "./parser/parser.cpp"
    break;

  case 19: // NT_DirectiveWithList: WORD NT_LiteralList
#line 166 "./parser.yy"
    {
        assembly.SetInstruction(eAssemblyIdentifier::WORD, eAssemblyIdentifierType::DIRECTIVE);
        assembly.SetMultipleOperands(yystack_[0].value.as < std::vector<ParserOperand> > ());

    }
#line 778 "./parser/parser.cpp"
    break;

  case 20: // NT_DirectiveWithList: WORD NT_SymbolList
#line 172 "./parser.yy"
    {
        assembly.SetInstruction(eAssemblyIdentifier::WORD, eAssemblyIdentifierType::DIRECTIVE);
        assembly.SetMultipleOperands(yystack_[0].value.as < std::vector<ParserOperand> > ());
    }
#line 787 "./parser/parser.cpp"
    break;

  case 21: // NT_DirectiveSingleArgument: SECTION SYMBOL
#line 180 "./parser.yy"
    {
        assembly.SetInstruction(eAssemblyIdentifier::SECTION, eAssemblyIdentifierType::DIRECTIVE);
        assembly.SetOperand(yystack_[0].value.as < std::string > (), eOperandType::SYM);
    }
#line 796 "./parser/parser.cpp"
    break;

  case 22: // NT_DirectiveSingleArgument: SKIP LITERAL
#line 185 "./parser.yy"
    {
        assembly.SetInstruction(eAssemblyIdentifier::SKIP, eAssemblyIdentifierType::DIRECTIVE);
        assembly.SetOperand(yystack_[0].value.as < std::string > (), eOperandType::LTR);
    }
#line 805 "./parser/parser.cpp"
    break;

  case 23: // NT_DirectiveWithSymbolList: NT_DirectiveIdentifier NT_SymbolList
#line 193 "./parser.yy"
    {
        assembly.SetInstruction(yystack_[1].value.as < eAssemblyIdentifier > (), eAssemblyIdentifierType::DIRECTIVE);
        assembly.SetMultipleOperands(yystack_[0].value.as < std::vector<ParserOperand> > ());
    }
#line 814 "./parser/parser.cpp"
    break;

  case 24: // NT_DirectiveIdentifier: GLOBAL
#line 200 "./parser.yy"
           { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::GLOBAL; }
#line 820 "./parser/parser.cpp"
    break;

  case 25: // NT_DirectiveIdentifier: EXTERN
#line 201 "./parser.yy"
             { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::EXTERN; }
#line 826 "./parser/parser.cpp"
    break;

  case 26: // NT_ProcessorInstruction: HALT
#line 206 "./parser.yy"
         { assembly.SetInstruction(eAssemblyIdentifier::HALT, eAssemblyIdentifierType::PROCESSOR); }
#line 832 "./parser/parser.cpp"
    break;

  case 27: // NT_ProcessorInstruction: INT
#line 207 "./parser.yy"
          { assembly.SetInstruction(eAssemblyIdentifier::INT, eAssemblyIdentifierType::PROCESSOR); }
#line 838 "./parser/parser.cpp"
    break;

  case 28: // NT_ProcessorInstruction: IRET
#line 208 "./parser.yy"
           { assembly.SetInstruction(eAssemblyIdentifier::IRET, eAssemblyIdentifierType::PROCESSOR); }
#line 844 "./parser/parser.cpp"
    break;

  case 29: // NT_ProcessorInstruction: RET
#line 209 "./parser.yy"
          { assembly.SetInstruction(eAssemblyIdentifier::RET, eAssemblyIdentifierType::PROCESSOR); }
#line 850 "./parser/parser.cpp"
    break;

  case 30: // NT_BranchInstruction: NT_ConditionalJumpIdentifier NT_ConditionalJumpOperands
#line 215 "./parser.yy"
    {
        assembly.SetInstruction(yystack_[1].value.as < eAssemblyIdentifier > (), eAssemblyIdentifierType::BRANCH);
        assembly.SetOperand(yystack_[0].value.as < ConditionalJumpOperands > ().gpr1, eOperandType::GPR);
        assembly.SetOperand(yystack_[0].value.as < ConditionalJumpOperands > ().gpr2, eOperandType::GPR);
        assembly.SetOperand(yystack_[0].value.as < ConditionalJumpOperands > ().operand);
    }
#line 861 "./parser/parser.cpp"
    break;

  case 31: // NT_BranchInstruction: NT_UnconditionalJumpIdentifier NT_JumpOperand
#line 222 "./parser.yy"
    {
        assembly.SetInstruction(yystack_[1].value.as < eAssemblyIdentifier > (), eAssemblyIdentifierType::BRANCH);
        assembly.SetOperand(yystack_[0].value.as < ParserOperand > ());
    }
#line 870 "./parser/parser.cpp"
    break;

  case 32: // NT_ConditionalJumpIdentifier: BEQ
#line 229 "./parser.yy"
        { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::BEQ; }
#line 876 "./parser/parser.cpp"
    break;

  case 33: // NT_ConditionalJumpIdentifier: BNE
#line 230 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::BNE; }
#line 882 "./parser/parser.cpp"
    break;

  case 34: // NT_ConditionalJumpIdentifier: BGT
#line 231 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::BGT; }
#line 888 "./parser/parser.cpp"
    break;

  case 35: // NT_ConditionalJumpOperands: "%" GPR "," "%" GPR "," NT_JumpOperand
#line 236 "./parser.yy"
    {
        yylhs.value.as < ConditionalJumpOperands > ().gpr1 = yystack_[5].value.as < std::string > ();
        yylhs.value.as < ConditionalJumpOperands > ().gpr2 = yystack_[2].value.as < std::string > ();
        yylhs.value.as < ConditionalJumpOperands > ().operand = yystack_[0].value.as < ParserOperand > ();
    }
#line 898 "./parser/parser.cpp"
    break;

  case 36: // NT_UnconditionalJumpIdentifier: JMP
#line 244 "./parser.yy"
        { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::JMP; }
#line 904 "./parser/parser.cpp"
    break;

  case 37: // NT_UnconditionalJumpIdentifier: CALL
#line 245 "./parser.yy"
           { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::CALL; }
#line 910 "./parser/parser.cpp"
    break;

  case 38: // NT_DataInstruction: NT_DataInstructionIdentifier NT_DataInstructionOperands
#line 251 "./parser.yy"
    {
        if (yystack_[1].value.as < eAssemblyIdentifier > () == eAssemblyIdentifier::NOT && yystack_[0].value.as < std::vector<ParserOperand> > ().size() > 1)
        {
            error(yystack_[1].location, "Wrong number of arguments for instruction not.\n");
            return 1;
        }

        assembly.SetInstruction(yystack_[1].value.as < eAssemblyIdentifier > (), eAssemblyIdentifierType::DATA);
        assembly.SetMultipleOperands(yystack_[0].value.as < std::vector<ParserOperand> > ());
    }
#line 925 "./parser/parser.cpp"
    break;

  case 39: // NT_DataInstructionIdentifier: XCHG
#line 264 "./parser.yy"
         { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::XCHG; }
#line 931 "./parser/parser.cpp"
    break;

  case 40: // NT_DataInstructionIdentifier: ADD
#line 265 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::ADD; }
#line 937 "./parser/parser.cpp"
    break;

  case 41: // NT_DataInstructionIdentifier: SUB
#line 266 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::SUB; }
#line 943 "./parser/parser.cpp"
    break;

  case 42: // NT_DataInstructionIdentifier: MUL
#line 267 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::MUL; }
#line 949 "./parser/parser.cpp"
    break;

  case 43: // NT_DataInstructionIdentifier: DIV
#line 268 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::DIV; }
#line 955 "./parser/parser.cpp"
    break;

  case 44: // NT_DataInstructionIdentifier: NOT
#line 269 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::NOT; }
#line 961 "./parser/parser.cpp"
    break;

  case 45: // NT_DataInstructionIdentifier: AND
#line 270 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::AND; }
#line 967 "./parser/parser.cpp"
    break;

  case 46: // NT_DataInstructionIdentifier: OR
#line 271 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::OR;  }
#line 973 "./parser/parser.cpp"
    break;

  case 47: // NT_DataInstructionIdentifier: XOR
#line 272 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::XOR; }
#line 979 "./parser/parser.cpp"
    break;

  case 48: // NT_DataInstructionIdentifier: SHL
#line 273 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::SHL; }
#line 985 "./parser/parser.cpp"
    break;

  case 49: // NT_DataInstructionIdentifier: SHR
#line 274 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::SHR; }
#line 991 "./parser/parser.cpp"
    break;

  case 50: // NT_DataInstructionOperands: "%" GPR "," "%" GPR
#line 279 "./parser.yy"
    {
        yylhs.value.as < std::vector<ParserOperand> > ().push_back(ParserOperand(yystack_[3].value.as < std::string > (), eOperandType::GPR));
        yylhs.value.as < std::vector<ParserOperand> > ().push_back(ParserOperand(yystack_[0].value.as < std::string > (), eOperandType::GPR));
    }
#line 1000 "./parser/parser.cpp"
    break;

  case 51: // NT_DataInstructionOperands: "%" GPR
#line 283 "./parser.yy"
              { yylhs.value.as < std::vector<ParserOperand> > ().push_back(ParserOperand(yystack_[0].value.as < std::string > (), eOperandType::GPR)); }
#line 1006 "./parser/parser.cpp"
    break;

  case 52: // NT_MemoryInstruction: LD NT_Operand "," "%" GPR
#line 289 "./parser.yy"
    {
        assembly.SetInstruction(eAssemblyIdentifier::LD, eAssemblyIdentifierType::MEMORY);
        assembly.SetOperand(yystack_[3].value.as < ParserOperand > ());
        assembly.SetOperand(yystack_[0].value.as < std::string > (), eOperandType::GPR);
    }
#line 1016 "./parser/parser.cpp"
    break;

  case 53: // NT_MemoryInstruction: ST "%" GPR "," NT_Operand
#line 295 "./parser.yy"
    {
        assembly.SetInstruction(eAssemblyIdentifier::ST, eAssemblyIdentifierType::MEMORY);
        assembly.SetOperand(yystack_[2].value.as < std::string > (), eOperandType::GPR);
        assembly.SetOperand(yystack_[0].value.as < ParserOperand > ());
    }
#line 1026 "./parser/parser.cpp"
    break;

  case 54: // NT_SpecialInstruction: CSRRD "%" CSR "," "%" GPR
#line 305 "./parser.yy"
    {
        assembly.SetInstruction(eAssemblyIdentifier::CSRRD, eAssemblyIdentifierType::SPECIAL);
        assembly.SetOperand(yystack_[3].value.as < std::string > (), eOperandType::CSR);
        assembly.SetOperand(yystack_[0].value.as < std::string > (), eOperandType::GPR);
    }
#line 1036 "./parser/parser.cpp"
    break;

  case 55: // NT_SpecialInstruction: CSRWR "%" GPR "," "%" CSR
#line 311 "./parser.yy"
    {
        assembly.SetInstruction(eAssemblyIdentifier::CSRWR, eAssemblyIdentifierType::SPECIAL);
        assembly.SetOperand(yystack_[3].value.as < std::string > (), eOperandType::GPR);
        assembly.SetOperand(yystack_[0].value.as < std::string > (), eOperandType::CSR);
    }
#line 1046 "./parser/parser.cpp"
    break;

  case 56: // NT_StackInstruction: NT_StackInstructionIdentifier "%" GPR
#line 321 "./parser.yy"
    {
        assembly.SetInstruction(yystack_[2].value.as < eAssemblyIdentifier > (), eAssemblyIdentifierType::STACK);
        assembly.SetOperand(yystack_[0].value.as < std::string > (), eOperandType::GPR);
    }
#line 1055 "./parser/parser.cpp"
    break;

  case 57: // NT_StackInstructionIdentifier: PUSH
#line 328 "./parser.yy"
         { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::PUSH; }
#line 1061 "./parser/parser.cpp"
    break;

  case 58: // NT_StackInstructionIdentifier: POP
#line 329 "./parser.yy"
          { yylhs.value.as < eAssemblyIdentifier > () = eAssemblyIdentifier::POP; }
#line 1067 "./parser/parser.cpp"
    break;

  case 59: // NT_SymbolList: NT_SymbolList "," SYMBOL
#line 334 "./parser.yy"
                               { yystack_[2].value.as < std::vector<ParserOperand> > ().push_back(ParserOperand(yystack_[0].value.as < std::string > (), eOperandType::SYM)); yylhs.value.as < std::vector<ParserOperand> > () = yystack_[2].value.as < std::vector<ParserOperand> > (); }
#line 1073 "./parser/parser.cpp"
    break;

  case 60: // NT_SymbolList: SYMBOL
#line 335 "./parser.yy"
             { yylhs.value.as < std::vector<ParserOperand> > ().push_back(ParserOperand(yystack_[0].value.as < std::string > (), eOperandType::SYM)); }
#line 1079 "./parser/parser.cpp"
    break;

  case 61: // NT_LiteralList: NT_LiteralList "," LITERAL
#line 339 "./parser.yy"
                                 { yystack_[2].value.as < std::vector<ParserOperand> > ().push_back(ParserOperand(yystack_[0].value.as < std::string > (), eOperandType::LTR)); yylhs.value.as < std::vector<ParserOperand> > () = yystack_[2].value.as < std::vector<ParserOperand> > (); }
#line 1085 "./parser/parser.cpp"
    break;

  case 62: // NT_LiteralList: LITERAL
#line 340 "./parser.yy"
              { yylhs.value.as < std::vector<ParserOperand> > ().push_back(ParserOperand(yystack_[0].value.as < std::string > (), eOperandType::LTR)); }
#line 1091 "./parser/parser.cpp"
    break;

  case 63: // NT_JumpOperand: SYMBOL
#line 346 "./parser.yy"
              { yylhs.value.as < ParserOperand > ().value = yystack_[0].value.as < std::string > (); yylhs.value.as < ParserOperand > ().type = eOperandType::SYM; yylhs.value.as < ParserOperand > ().addressingType = eAddressingType::ADDR_DIRECT; }
#line 1097 "./parser/parser.cpp"
    break;

  case 64: // NT_JumpOperand: LITERAL
#line 347 "./parser.yy"
              { yylhs.value.as < ParserOperand > ().value = yystack_[0].value.as < std::string > (); yylhs.value.as < ParserOperand > ().type = eOperandType::LTR; yylhs.value.as < ParserOperand > ().addressingType = eAddressingType::ADDR_DIRECT; }
#line 1103 "./parser/parser.cpp"
    break;

  case 65: // NT_Operand: "$" LITERAL
#line 351 "./parser.yy"
                { yylhs.value.as < ParserOperand > ().value = yystack_[0].value.as < std::string > (); yylhs.value.as < ParserOperand > ().type = eOperandType::LTR; yylhs.value.as < ParserOperand > ().addressingType = eAddressingType::ADDR_DIRECT; }
#line 1109 "./parser/parser.cpp"
    break;

  case 66: // NT_Operand: "$" SYMBOL
#line 352 "./parser.yy"
                 { yylhs.value.as < ParserOperand > ().value = yystack_[0].value.as < std::string > (); yylhs.value.as < ParserOperand > ().type = eOperandType::SYM; yylhs.value.as < ParserOperand > ().addressingType = eAddressingType::ADDR_DIRECT; }
#line 1115 "./parser/parser.cpp"
    break;

  case 67: // NT_Operand: LITERAL
#line 353 "./parser.yy"
              { yylhs.value.as < ParserOperand > ().value = yystack_[0].value.as < std::string > (); yylhs.value.as < ParserOperand > ().type = eOperandType::LTR; yylhs.value.as < ParserOperand > ().addressingType = eAddressingType::ADDR_MEMORY; }
#line 1121 "./parser/parser.cpp"
    break;

  case 68: // NT_Operand: SYMBOL
#line 354 "./parser.yy"
             { yylhs.value.as < ParserOperand > ().value = yystack_[0].value.as < std::string > (); yylhs.value.as < ParserOperand > ().type = eOperandType::SYM; yylhs.value.as < ParserOperand > ().addressingType = eAddressingType::ADDR_MEMORY; }
#line 1127 "./parser/parser.cpp"
    break;

  case 69: // NT_Operand: "%" GPR
#line 355 "./parser.yy"
              { yylhs.value.as < ParserOperand > ().value = yystack_[0].value.as < std::string > (); yylhs.value.as < ParserOperand > ().type = eOperandType::GPR; yylhs.value.as < ParserOperand > ().addressingType = eAddressingType::ADDR_DIRECT; }
#line 1133 "./parser/parser.cpp"
    break;

  case 70: // NT_Operand: "[" "%" GPR "]"
#line 356 "./parser.yy"
                      { yylhs.value.as < ParserOperand > ().value = yystack_[1].value.as < std::string > (); yylhs.value.as < ParserOperand > ().type = eOperandType::GPR; yylhs.value.as < ParserOperand > ().addressingType = eAddressingType::ADDR_MEMORY; }
#line 1139 "./parser/parser.cpp"
    break;

  case 71: // NT_Operand: "[" "%" GPR "+" LITERAL "]"
#line 357 "./parser.yy"
                                  { yylhs.value.as < ParserOperand > ().value = yystack_[3].value.as < std::string > (); yylhs.value.as < ParserOperand > ().offset = yystack_[1].value.as < std::string > (); yylhs.value.as < ParserOperand > ().offsetType = eOperandType::LTR; yylhs.value.as < ParserOperand > ().type = eOperandType::GPR; yylhs.value.as < ParserOperand > ().addressingType = eAddressingType::ADDR_MEMORY_OFFSET; }
#line 1145 "./parser/parser.cpp"
    break;

  case 72: // NT_Operand: "[" "%" GPR "+" SYMBOL "]"
#line 358 "./parser.yy"
                                  { yylhs.value.as < ParserOperand > ().value = yystack_[3].value.as < std::string > (); yylhs.value.as < ParserOperand > ().offset = yystack_[1].value.as < std::string > (); yylhs.value.as < ParserOperand > ().offsetType = eOperandType::SYM; yylhs.value.as < ParserOperand > ().type = eOperandType::GPR; yylhs.value.as < ParserOperand > ().addressingType = eAddressingType::ADDR_MEMORY_OFFSET; }
#line 1151 "./parser/parser.cpp"
    break;


#line 1155 "./parser/parser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      yy_lac_discard_ ("error recovery");
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  // parser::context.
  parser::context::context (const parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

#if YYDEBUG
    // Execute LAC once. We don't care if it is successful, we
    // only do it for the sake of debugging output.
    if (!yyparser_.yy_lac_established_)
      yyparser_.yy_lac_check_ (yyla_.kind ());
#endif

    for (int yyx = 0; yyx < YYNTOKENS; ++yyx)
      {
        symbol_kind_type yysym = YY_CAST (symbol_kind_type, yyx);
        if (yysym != symbol_kind::S_YYerror
            && yysym != symbol_kind::S_YYUNDEF
            && yyparser_.yy_lac_check_ (yysym))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = yysym;
          }
      }
    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }




  bool
  parser::yy_lac_check_ (symbol_kind_type yytoken) const
  {
    // Logically, the yylac_stack's lifetime is confined to this function.
    // Clear it, to get rid of potential left-overs from previous call.
    yylac_stack_.clear ();
    // Reduce until we encounter a shift and thereby accept the token.
#if YYDEBUG
    YYCDEBUG << "LAC: checking lookahead " << symbol_name (yytoken) << ':';
#endif
    std::ptrdiff_t lac_top = 0;
    while (true)
      {
        state_type top_state = (yylac_stack_.empty ()
                                ? yystack_[lac_top].state
                                : yylac_stack_.back ());
        int yyrule = yypact_[+top_state];
        if (yy_pact_value_is_default_ (yyrule)
            || (yyrule += yytoken) < 0 || yylast_ < yyrule
            || yycheck_[yyrule] != yytoken)
          {
            // Use the default action.
            yyrule = yydefact_[+top_state];
            if (yyrule == 0)
              {
                YYCDEBUG << " Err\n";
                return false;
              }
          }
        else
          {
            // Use the action from yytable.
            yyrule = yytable_[yyrule];
            if (yy_table_value_is_error_ (yyrule))
              {
                YYCDEBUG << " Err\n";
                return false;
              }
            if (0 < yyrule)
              {
                YYCDEBUG << " S" << yyrule << '\n';
                return true;
              }
            yyrule = -yyrule;
          }
        // By now we know we have to simulate a reduce.
        YYCDEBUG << " R" << yyrule - 1;
        // Pop the corresponding number of values from the stack.
        {
          std::ptrdiff_t yylen = yyr2_[yyrule];
          // First pop from the LAC stack as many tokens as possible.
          std::ptrdiff_t lac_size = std::ptrdiff_t (yylac_stack_.size ());
          if (yylen < lac_size)
            {
              yylac_stack_.resize (std::size_t (lac_size - yylen));
              yylen = 0;
            }
          else if (lac_size)
            {
              yylac_stack_.clear ();
              yylen -= lac_size;
            }
          // Only afterwards look at the main stack.
          // We simulate popping elements by incrementing lac_top.
          lac_top += yylen;
        }
        // Keep top_state in sync with the updated stack.
        top_state = (yylac_stack_.empty ()
                     ? yystack_[lac_top].state
                     : yylac_stack_.back ());
        // Push the resulting state of the reduction.
        state_type state = yy_lr_goto_state_ (top_state, yyr1_[yyrule]);
        YYCDEBUG << " G" << int (state);
        yylac_stack_.push_back (state);
      }
  }

  // Establish the initial context if no initial context currently exists.
  bool
  parser::yy_lac_establish_ (symbol_kind_type yytoken)
  {
    /* Establish the initial context for the current lookahead if no initial
       context is currently established.

       We define a context as a snapshot of the parser stacks.  We define
       the initial context for a lookahead as the context in which the
       parser initially examines that lookahead in order to select a
       syntactic action.  Thus, if the lookahead eventually proves
       syntactically unacceptable (possibly in a later context reached via a
       series of reductions), the initial context can be used to determine
       the exact set of tokens that would be syntactically acceptable in the
       lookahead's place.  Moreover, it is the context after which any
       further semantic actions would be erroneous because they would be
       determined by a syntactically unacceptable token.

       yy_lac_establish_ should be invoked when a reduction is about to be
       performed in an inconsistent state (which, for the purposes of LAC,
       includes consistent states that don't know they're consistent because
       their default reductions have been disabled).

       For parse.lac=full, the implementation of yy_lac_establish_ is as
       follows.  If no initial context is currently established for the
       current lookahead, then check if that lookahead can eventually be
       shifted if syntactic actions continue from the current context.  */
    if (yy_lac_established_)
      return true;
    else
      {
#if YYDEBUG
        YYCDEBUG << "LAC: initial context established for "
                 << symbol_name (yytoken) << '\n';
#endif
        yy_lac_established_ = true;
        return yy_lac_check_ (yytoken);
      }
  }

  // Discard any previous initial lookahead context.
  void
  parser::yy_lac_discard_ (const char* event)
  {
   /* Discard any previous initial lookahead context because of Event,
      which may be a lookahead change or an invalidation of the currently
      established initial context for the current lookahead.

      The most common example of a lookahead change is a shift.  An example
      of both cases is syntax error recovery.  That is, a syntax error
      occurs when the lookahead is syntactically erroneous for the
      currently established initial context, so error recovery manipulates
      the parser stacks to try to find a new initial context in which the
      current lookahead is syntactically acceptable.  If it fails to find
      such a context, it discards the lookahead.  */
    if (yy_lac_established_)
      {
        YYCDEBUG << "LAC: initial context discarded due to "
                 << event << '\n';
        yy_lac_established_ = false;
      }
  }


  int
  parser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
         In the first two cases, it might appear that the current syntax
         error should have been detected in the previous state when
         yy_lac_check was invoked.  However, at that time, there might
         have been a different syntax error that discarded a different
         initial context during error recovery, leaving behind the
         current lookahead.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser::yypact_ninf_ = -38;

  const signed char parser::yytable_ninf_ = -1;

  const signed char
  parser::yypact_[] =
  {
       0,   -38,   -38,   -38,     4,   -37,    -1,   -38,   -38,   -38,
     -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,
     -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,
      44,    -2,    34,    40,   -38,   -38,    43,   -38,   -38,   -38,
     -38,   -38,   -38,    13,   -38,   -38,    42,   -30,   -38,    45,
     -38,   -38,   -38,    47,   -38,   -38,   -38,    48,    49,   -38,
     -38,   -38,     2,    50,    53,    51,    56,    59,    62,   -38,
     -38,    48,    63,   -38,   -38,   -38,   -38,    64,   -38,    65,
      52,    67,   -38,   -38,    68,   -38,    60,    61,    66,    69,
      70,    71,   -38,   -38,   -38,    -4,    72,    44,    75,    76,
      77,    78,   -38,    46,   -38,   -38,    73,    74,    86,    87,
      85,    88,   -38,   -38,    79,   -38,   -38,   -38,   -30,   -38
  };

  const signed char
  parser::yydefact_[] =
  {
       0,     4,    24,    25,     0,     0,     0,    18,    26,    27,
      28,    29,    36,    37,    57,    58,    32,    33,    34,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,     0,     0,     0,    14,    13,     0,     3,    12,     5,
      16,    17,    15,     0,     6,     7,     0,     0,     9,     0,
      10,    11,     8,     0,    21,    60,    62,    20,    19,    22,
      68,    67,     0,     0,     0,     0,     0,     0,     0,     1,
       2,    23,     0,    30,    63,    64,    31,     0,    38,     0,
       0,     0,    66,    65,     0,    69,     0,     0,     0,     0,
       0,    51,    56,    59,    61,     0,     0,     0,     0,     0,
       0,     0,    70,     0,    52,    53,     0,     0,     0,     0,
       0,     0,    54,    55,     0,    50,    72,    71,     0,    35
  };

  const signed char
  parser::yypgoto_[] =
  {
     -38,   -38,    81,   -38,   -38,   -38,   -38,   -38,   -38,   -38,
     -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,
     -38,    89,   -38,    -9,    17
  };

  const signed char
  parser::yydefgoto_[] =
  {
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    73,    47,    48,    49,    78,    50,    51,    52,
      53,    57,    58,    76,    65
  };

  const signed char
  parser::yytable_[] =
  {
       1,    55,    56,     2,     3,     4,     5,     6,    74,    75,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    59,   102,
      82,    83,    54,    69,   103,    66,     2,     3,     4,     5,
       6,    55,    35,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    67,    60,    61,   110,   111,    62,    68,    63,    72,
      93,    64,    77,    85,    79,    35,    87,    84,    80,    81,
      88,    86,    89,    90,    91,    92,    94,    96,    95,   119,
       0,    97,   104,   112,   105,   113,    98,    70,     0,    99,
     100,   101,   106,   107,   108,   109,   114,   115,   116,   118,
       0,   117,    71
  };

  const signed char
  parser::yycheck_[] =
  {
       0,    38,    39,     3,     4,     5,     6,     7,    38,    39,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    39,    43,
      38,    39,    38,     0,    48,    47,     3,     4,     5,     6,
       7,    38,    52,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    47,    38,    39,    38,    39,    42,    47,    44,    47,
      38,    47,    47,    40,    47,    52,    40,    47,    50,    50,
      41,    50,    40,    40,    40,    40,    39,    47,    40,   118,
      -1,    50,    40,    40,    97,    41,    50,    36,    -1,    50,
      50,    50,    47,    47,    47,    47,    40,    40,    43,    50,
      -1,    43,    43
  };

  const signed char
  parser::yystos_[] =
  {
       0,     0,     3,     4,     5,     6,     7,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    52,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    66,    67,    68,
      70,    71,    72,    73,    38,    38,    39,    74,    75,    39,
      38,    39,    42,    44,    47,    77,    47,    47,    47,     0,
      55,    74,    47,    65,    38,    39,    76,    47,    69,    47,
      50,    50,    38,    39,    47,    40,    50,    40,    41,    40,
      40,    40,    40,    38,    39,    40,    47,    50,    50,    50,
      50,    50,    43,    48,    40,    77,    47,    47,    47,    47,
      38,    39,    40,    41,    40,    40,    43,    43,    50,    76
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    53,    54,    54,    54,    55,    55,    55,    55,    55,
      55,    55,    55,    56,    56,    57,    57,    57,    57,    58,
      58,    59,    59,    60,    61,    61,    62,    62,    62,    62,
      63,    63,    64,    64,    64,    65,    66,    66,    67,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      69,    69,    70,    70,    71,    71,    72,    73,    73,    74,
      74,    75,    75,    76,    76,    77,    77,    77,    77,    77,
      77,    77,    77
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     1,
       2,     2,     1,     1,     1,     7,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       5,     2,     5,     5,     6,     6,     3,     1,     1,     3,
       1,     3,     1,     1,     1,     2,     2,     1,     1,     2,
       4,     6,     6
  };


#if YYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "GLOBAL", "EXTERN",
  "SECTION", "WORD", "SKIP", "ASCII", "EQU", "END", "HALT", "INT", "IRET",
  "RET", "JMP", "CALL", "PUSH", "POP", "BEQ", "BNE", "BGT", "XCHG", "ADD",
  "SUB", "MUL", "DIV", "NOT", "AND", "OR", "XOR", "SHL", "SHR", "LD", "ST",
  "CSRRD", "CSRWR", "LABEL", "SYMBOL", "LITERAL", "GPR", "CSR", "\"$\"",
  "\"]\"", "\"[\"", "\"(\"", "\")\"", "\"%\"", "\"+\"", "\"-\"", "\",\"",
  "\":\"", "COMMENT", "$accept", "NT_Program", "NT_Line",
  "NT_LabelAndComment", "NT_Directive", "NT_DirectiveWithList",
  "NT_DirectiveSingleArgument", "NT_DirectiveWithSymbolList",
  "NT_DirectiveIdentifier", "NT_ProcessorInstruction",
  "NT_BranchInstruction", "NT_ConditionalJumpIdentifier",
  "NT_ConditionalJumpOperands", "NT_UnconditionalJumpIdentifier",
  "NT_DataInstruction", "NT_DataInstructionIdentifier",
  "NT_DataInstructionOperands", "NT_MemoryInstruction",
  "NT_SpecialInstruction", "NT_StackInstruction",
  "NT_StackInstructionIdentifier", "NT_SymbolList", "NT_LiteralList",
  "NT_JumpOperand", "NT_Operand", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   106,   106,   118,   130,   134,   135,   136,   137,   138,
     139,   140,   141,   145,   146,   154,   155,   156,   157,   165,
     171,   179,   184,   192,   200,   201,   206,   207,   208,   209,
     214,   221,   229,   230,   231,   235,   244,   245,   250,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     278,   283,   288,   294,   304,   310,   320,   328,   329,   334,
     335,   339,   340,   346,   347,   351,   352,   353,   354,   355,
     356,   357,   358
  };

  void
  parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


} // yy
#line 1850 "./parser/parser.cpp"

#line 362 "./parser.yy"


void yy::parser::error (const location_type& l, const std::string& m)
{
    std::cerr << "\033[31m ERROR:" << l << ": " << m << '\n';
}
