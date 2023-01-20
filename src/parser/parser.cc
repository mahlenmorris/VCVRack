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





#include "parser.hh"


// Unqualified %code blocks.
#line 36 "parser.yy"

#include "driver.hh"

#line 50 "parser.cc"


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
#line 142 "parser.cc"

  /// Build a parser object.
  Parser::Parser (Driver& drv_yyarg, void* yyscanner_yyarg, yy::location& loc_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      yy_lac_established_ (false),
      drv (drv_yyarg),
      yyscanner (yyscanner_yyarg),
      loc (loc_yyarg)
  {}

  Parser::~Parser ()
  {}

  Parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/



  // by_state.
  Parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  Parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  Parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  Parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  Parser::symbol_kind_type
  Parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  Parser::stack_symbol_type::stack_symbol_type ()
  {}

  Parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_block: // block
      case symbol_kind::S_main_block: // main_block
        value.YY_MOVE_OR_COPY< Block > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_blocks: // blocks
        value.YY_MOVE_OR_COPY< Blocks > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_exp: // exp
        value.YY_MOVE_OR_COPY< Expression > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expression_list: // expression_list
        value.YY_MOVE_OR_COPY< ExpressionList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_statement: // statement
      case symbol_kind::S_array_assignment: // array_assignment
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_clear_statement: // clear_statement
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_elseif_clause: // elseif_clause
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_reset_statement: // reset_statement
      case symbol_kind::S_wait_statement: // wait_statement
        value.YY_MOVE_OR_COPY< Line > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_zero_or_more_statements: // zero_or_more_statements
      case symbol_kind::S_one_or_more_statements: // one_or_more_statements
      case symbol_kind::S_elseif_group: // elseif_group
        value.YY_MOVE_OR_COPY< Statements > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.YY_MOVE_OR_COPY< float > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_ALL: // "all"
      case symbol_kind::S_ALSO: // "also"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_CLEAR: // "clear"
      case symbol_kind::S_CONNECTED: // "connected"
      case symbol_kind::S_CONTINUE: // "continue"
      case symbol_kind::S_ELSE: // "else"
      case symbol_kind::S_ELSEIF: // "elseif"
      case symbol_kind::S_END: // "end"
      case symbol_kind::S_EXIT: // "exit"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_FOR: // "for"
      case symbol_kind::S_IF: // "if"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_NEXT: // "next"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_RESET: // "reset"
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
      case symbol_kind::S_TRIGGER: // "trigger"
      case symbol_kind::S_WAIT: // "wait"
      case symbol_kind::S_WHEN: // "when"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_LBRACE: // "{"
      case symbol_kind::S_RBRACE: // "}"
      case symbol_kind::S_LBRACKET: // "["
      case symbol_kind::S_RBRACKET: // "]"
      case symbol_kind::S_COMMA: // ","
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_NOTE: // "note"
      case symbol_kind::S_IN_PORT: // "in_port"
      case symbol_kind::S_OUT_PORT: // "out_port"
      case symbol_kind::S_ZEROARGFUNC: // "zeroargfunc"
      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
      case symbol_kind::S_COMPARISON: // "comparison"
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  Parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_block: // block
      case symbol_kind::S_main_block: // main_block
        value.move< Block > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_blocks: // blocks
        value.move< Blocks > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_exp: // exp
        value.move< Expression > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expression_list: // expression_list
        value.move< ExpressionList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_statement: // statement
      case symbol_kind::S_array_assignment: // array_assignment
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_clear_statement: // clear_statement
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_elseif_clause: // elseif_clause
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_reset_statement: // reset_statement
      case symbol_kind::S_wait_statement: // wait_statement
        value.move< Line > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_zero_or_more_statements: // zero_or_more_statements
      case symbol_kind::S_one_or_more_statements: // one_or_more_statements
      case symbol_kind::S_elseif_group: // elseif_group
        value.move< Statements > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.move< float > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_ALL: // "all"
      case symbol_kind::S_ALSO: // "also"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_CLEAR: // "clear"
      case symbol_kind::S_CONNECTED: // "connected"
      case symbol_kind::S_CONTINUE: // "continue"
      case symbol_kind::S_ELSE: // "else"
      case symbol_kind::S_ELSEIF: // "elseif"
      case symbol_kind::S_END: // "end"
      case symbol_kind::S_EXIT: // "exit"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_FOR: // "for"
      case symbol_kind::S_IF: // "if"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_NEXT: // "next"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_RESET: // "reset"
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
      case symbol_kind::S_TRIGGER: // "trigger"
      case symbol_kind::S_WAIT: // "wait"
      case symbol_kind::S_WHEN: // "when"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_LBRACE: // "{"
      case symbol_kind::S_RBRACE: // "}"
      case symbol_kind::S_LBRACKET: // "["
      case symbol_kind::S_RBRACKET: // "]"
      case symbol_kind::S_COMMA: // ","
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_NOTE: // "note"
      case symbol_kind::S_IN_PORT: // "in_port"
      case symbol_kind::S_OUT_PORT: // "out_port"
      case symbol_kind::S_ZEROARGFUNC: // "zeroargfunc"
      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
      case symbol_kind::S_COMPARISON: // "comparison"
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_block: // block
      case symbol_kind::S_main_block: // main_block
        value.copy< Block > (that.value);
        break;

      case symbol_kind::S_blocks: // blocks
        value.copy< Blocks > (that.value);
        break;

      case symbol_kind::S_exp: // exp
        value.copy< Expression > (that.value);
        break;

      case symbol_kind::S_expression_list: // expression_list
        value.copy< ExpressionList > (that.value);
        break;

      case symbol_kind::S_statement: // statement
      case symbol_kind::S_array_assignment: // array_assignment
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_clear_statement: // clear_statement
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_elseif_clause: // elseif_clause
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_reset_statement: // reset_statement
      case symbol_kind::S_wait_statement: // wait_statement
        value.copy< Line > (that.value);
        break;

      case symbol_kind::S_zero_or_more_statements: // zero_or_more_statements
      case symbol_kind::S_one_or_more_statements: // one_or_more_statements
      case symbol_kind::S_elseif_group: // elseif_group
        value.copy< Statements > (that.value);
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.copy< float > (that.value);
        break;

      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_ALL: // "all"
      case symbol_kind::S_ALSO: // "also"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_CLEAR: // "clear"
      case symbol_kind::S_CONNECTED: // "connected"
      case symbol_kind::S_CONTINUE: // "continue"
      case symbol_kind::S_ELSE: // "else"
      case symbol_kind::S_ELSEIF: // "elseif"
      case symbol_kind::S_END: // "end"
      case symbol_kind::S_EXIT: // "exit"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_FOR: // "for"
      case symbol_kind::S_IF: // "if"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_NEXT: // "next"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_RESET: // "reset"
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
      case symbol_kind::S_TRIGGER: // "trigger"
      case symbol_kind::S_WAIT: // "wait"
      case symbol_kind::S_WHEN: // "when"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_LBRACE: // "{"
      case symbol_kind::S_RBRACE: // "}"
      case symbol_kind::S_LBRACKET: // "["
      case symbol_kind::S_RBRACKET: // "]"
      case symbol_kind::S_COMMA: // ","
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_NOTE: // "note"
      case symbol_kind::S_IN_PORT: // "in_port"
      case symbol_kind::S_OUT_PORT: // "out_port"
      case symbol_kind::S_ZEROARGFUNC: // "zeroargfunc"
      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
      case symbol_kind::S_COMPARISON: // "comparison"
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_block: // block
      case symbol_kind::S_main_block: // main_block
        value.move< Block > (that.value);
        break;

      case symbol_kind::S_blocks: // blocks
        value.move< Blocks > (that.value);
        break;

      case symbol_kind::S_exp: // exp
        value.move< Expression > (that.value);
        break;

      case symbol_kind::S_expression_list: // expression_list
        value.move< ExpressionList > (that.value);
        break;

      case symbol_kind::S_statement: // statement
      case symbol_kind::S_array_assignment: // array_assignment
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_clear_statement: // clear_statement
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_elseif_clause: // elseif_clause
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_reset_statement: // reset_statement
      case symbol_kind::S_wait_statement: // wait_statement
        value.move< Line > (that.value);
        break;

      case symbol_kind::S_zero_or_more_statements: // zero_or_more_statements
      case symbol_kind::S_one_or_more_statements: // one_or_more_statements
      case symbol_kind::S_elseif_group: // elseif_group
        value.move< Statements > (that.value);
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.move< float > (that.value);
        break;

      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_ALL: // "all"
      case symbol_kind::S_ALSO: // "also"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_CLEAR: // "clear"
      case symbol_kind::S_CONNECTED: // "connected"
      case symbol_kind::S_CONTINUE: // "continue"
      case symbol_kind::S_ELSE: // "else"
      case symbol_kind::S_ELSEIF: // "elseif"
      case symbol_kind::S_END: // "end"
      case symbol_kind::S_EXIT: // "exit"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_FOR: // "for"
      case symbol_kind::S_IF: // "if"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_NEXT: // "next"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_RESET: // "reset"
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
      case symbol_kind::S_TRIGGER: // "trigger"
      case symbol_kind::S_WAIT: // "wait"
      case symbol_kind::S_WHEN: // "when"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_LBRACE: // "{"
      case symbol_kind::S_RBRACE: // "}"
      case symbol_kind::S_LBRACKET: // "["
      case symbol_kind::S_RBRACKET: // "]"
      case symbol_kind::S_COMMA: // ","
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_NOTE: // "note"
      case symbol_kind::S_IN_PORT: // "in_port"
      case symbol_kind::S_OUT_PORT: // "out_port"
      case symbol_kind::S_ZEROARGFUNC: // "zeroargfunc"
      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
      case symbol_kind::S_COMPARISON: // "comparison"
        value.move< std::string > (that.value);
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
  Parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  Parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
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
        switch (yykind)
    {
      case symbol_kind::S_ABS: // "abs"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 680 "parser.cc"
        break;

      case symbol_kind::S_ALL: // "all"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 686 "parser.cc"
        break;

      case symbol_kind::S_ALSO: // "also"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 692 "parser.cc"
        break;

      case symbol_kind::S_AND: // "and"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 698 "parser.cc"
        break;

      case symbol_kind::S_ASSIGN: // "="
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 704 "parser.cc"
        break;

      case symbol_kind::S_CEILING: // "ceiling"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 710 "parser.cc"
        break;

      case symbol_kind::S_CLEAR: // "clear"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 716 "parser.cc"
        break;

      case symbol_kind::S_CONNECTED: // "connected"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 722 "parser.cc"
        break;

      case symbol_kind::S_CONTINUE: // "continue"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 728 "parser.cc"
        break;

      case symbol_kind::S_ELSE: // "else"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 734 "parser.cc"
        break;

      case symbol_kind::S_ELSEIF: // "elseif"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 740 "parser.cc"
        break;

      case symbol_kind::S_END: // "end"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 746 "parser.cc"
        break;

      case symbol_kind::S_EXIT: // "exit"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 752 "parser.cc"
        break;

      case symbol_kind::S_FLOOR: // "floor"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 758 "parser.cc"
        break;

      case symbol_kind::S_FOR: // "for"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 764 "parser.cc"
        break;

      case symbol_kind::S_IF: // "if"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 770 "parser.cc"
        break;

      case symbol_kind::S_MAX: // "max"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 776 "parser.cc"
        break;

      case symbol_kind::S_MIN: // "min"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 782 "parser.cc"
        break;

      case symbol_kind::S_NEXT: // "next"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 788 "parser.cc"
        break;

      case symbol_kind::S_NOT: // "not"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 794 "parser.cc"
        break;

      case symbol_kind::S_OR: // "or"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 800 "parser.cc"
        break;

      case symbol_kind::S_POW: // "pow"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 806 "parser.cc"
        break;

      case symbol_kind::S_RESET: // "reset"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 812 "parser.cc"
        break;

      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 818 "parser.cc"
        break;

      case symbol_kind::S_SIGN: // "sign"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 824 "parser.cc"
        break;

      case symbol_kind::S_SIN: // "sin"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 830 "parser.cc"
        break;

      case symbol_kind::S_START: // "start"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 836 "parser.cc"
        break;

      case symbol_kind::S_STEP: // "step"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 842 "parser.cc"
        break;

      case symbol_kind::S_THEN: // "then"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 848 "parser.cc"
        break;

      case symbol_kind::S_TO: // "to"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 854 "parser.cc"
        break;

      case symbol_kind::S_TRIGGER: // "trigger"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 860 "parser.cc"
        break;

      case symbol_kind::S_WAIT: // "wait"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 866 "parser.cc"
        break;

      case symbol_kind::S_WHEN: // "when"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 872 "parser.cc"
        break;

      case symbol_kind::S_MINUS: // "-"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 878 "parser.cc"
        break;

      case symbol_kind::S_PLUS: // "+"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 884 "parser.cc"
        break;

      case symbol_kind::S_STAR: // "*"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 890 "parser.cc"
        break;

      case symbol_kind::S_SLASH: // "/"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 896 "parser.cc"
        break;

      case symbol_kind::S_LPAREN: // "("
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 902 "parser.cc"
        break;

      case symbol_kind::S_RPAREN: // ")"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 908 "parser.cc"
        break;

      case symbol_kind::S_LBRACE: // "{"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 914 "parser.cc"
        break;

      case symbol_kind::S_RBRACE: // "}"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 920 "parser.cc"
        break;

      case symbol_kind::S_LBRACKET: // "["
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 926 "parser.cc"
        break;

      case symbol_kind::S_RBRACKET: // "]"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 932 "parser.cc"
        break;

      case symbol_kind::S_COMMA: // ","
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 938 "parser.cc"
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 944 "parser.cc"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < float > (); }
#line 950 "parser.cc"
        break;

      case symbol_kind::S_NOTE: // "note"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 956 "parser.cc"
        break;

      case symbol_kind::S_IN_PORT: // "in_port"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 962 "parser.cc"
        break;

      case symbol_kind::S_OUT_PORT: // "out_port"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 968 "parser.cc"
        break;

      case symbol_kind::S_ZEROARGFUNC: // "zeroargfunc"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 974 "parser.cc"
        break;

      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 980 "parser.cc"
        break;

      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 986 "parser.cc"
        break;

      case symbol_kind::S_COMPARISON: // "comparison"
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 992 "parser.cc"
        break;

      case symbol_kind::S_blocks: // blocks
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Blocks > (); }
#line 998 "parser.cc"
        break;

      case symbol_kind::S_block: // block
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Block > (); }
#line 1004 "parser.cc"
        break;

      case symbol_kind::S_main_block: // main_block
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Block > (); }
#line 1010 "parser.cc"
        break;

      case symbol_kind::S_zero_or_more_statements: // zero_or_more_statements
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1016 "parser.cc"
        break;

      case symbol_kind::S_one_or_more_statements: // one_or_more_statements
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1022 "parser.cc"
        break;

      case symbol_kind::S_statement: // statement
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1028 "parser.cc"
        break;

      case symbol_kind::S_array_assignment: // array_assignment
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1034 "parser.cc"
        break;

      case symbol_kind::S_assignment: // assignment
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1040 "parser.cc"
        break;

      case symbol_kind::S_clear_statement: // clear_statement
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1046 "parser.cc"
        break;

      case symbol_kind::S_continue_statement: // continue_statement
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1052 "parser.cc"
        break;

      case symbol_kind::S_exit_statement: // exit_statement
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1058 "parser.cc"
        break;

      case symbol_kind::S_for_statement: // for_statement
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1064 "parser.cc"
        break;

      case symbol_kind::S_elseif_group: // elseif_group
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1070 "parser.cc"
        break;

      case symbol_kind::S_elseif_clause: // elseif_clause
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1076 "parser.cc"
        break;

      case symbol_kind::S_if_statement: // if_statement
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1082 "parser.cc"
        break;

      case symbol_kind::S_reset_statement: // reset_statement
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1088 "parser.cc"
        break;

      case symbol_kind::S_wait_statement: // wait_statement
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1094 "parser.cc"
        break;

      case symbol_kind::S_expression_list: // expression_list
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < ExpressionList > (); }
#line 1100 "parser.cc"
        break;

      case symbol_kind::S_exp: // exp
#line 117 "parser.yy"
                 { yyo << yysym.value.template as < Expression > (); }
#line 1106 "parser.cc"
        break;

      default:
        break;
    }
        yyo << ')';
      }
  }
#endif

  void
  Parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  Parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  Parser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  Parser::state_type
  Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  Parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  Parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::operator() ()
  {
    return parse ();
  }

  int
  Parser::parse ()
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
            symbol_type yylookahead (yylex (yyscanner, loc));
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
      case symbol_kind::S_block: // block
      case symbol_kind::S_main_block: // main_block
        yylhs.value.emplace< Block > ();
        break;

      case symbol_kind::S_blocks: // blocks
        yylhs.value.emplace< Blocks > ();
        break;

      case symbol_kind::S_exp: // exp
        yylhs.value.emplace< Expression > ();
        break;

      case symbol_kind::S_expression_list: // expression_list
        yylhs.value.emplace< ExpressionList > ();
        break;

      case symbol_kind::S_statement: // statement
      case symbol_kind::S_array_assignment: // array_assignment
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_clear_statement: // clear_statement
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_elseif_clause: // elseif_clause
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_reset_statement: // reset_statement
      case symbol_kind::S_wait_statement: // wait_statement
        yylhs.value.emplace< Line > ();
        break;

      case symbol_kind::S_zero_or_more_statements: // zero_or_more_statements
      case symbol_kind::S_one_or_more_statements: // one_or_more_statements
      case symbol_kind::S_elseif_group: // elseif_group
        yylhs.value.emplace< Statements > ();
        break;

      case symbol_kind::S_NUMBER: // "number"
        yylhs.value.emplace< float > ();
        break;

      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_ALL: // "all"
      case symbol_kind::S_ALSO: // "also"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_CLEAR: // "clear"
      case symbol_kind::S_CONNECTED: // "connected"
      case symbol_kind::S_CONTINUE: // "continue"
      case symbol_kind::S_ELSE: // "else"
      case symbol_kind::S_ELSEIF: // "elseif"
      case symbol_kind::S_END: // "end"
      case symbol_kind::S_EXIT: // "exit"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_FOR: // "for"
      case symbol_kind::S_IF: // "if"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_NEXT: // "next"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_RESET: // "reset"
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
      case symbol_kind::S_TRIGGER: // "trigger"
      case symbol_kind::S_WAIT: // "wait"
      case symbol_kind::S_WHEN: // "when"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_LBRACE: // "{"
      case symbol_kind::S_RBRACE: // "}"
      case symbol_kind::S_LBRACKET: // "["
      case symbol_kind::S_RBRACKET: // "]"
      case symbol_kind::S_COMMA: // ","
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_NOTE: // "note"
      case symbol_kind::S_IN_PORT: // "in_port"
      case symbol_kind::S_OUT_PORT: // "out_port"
      case symbol_kind::S_ZEROARGFUNC: // "zeroargfunc"
      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
      case symbol_kind::S_COMPARISON: // "comparison"
        yylhs.value.emplace< std::string > ();
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
  case 2: // program: blocks $end
#line 123 "parser.yy"
                                  { drv.blocks = yystack_[1].value.as < Blocks > ().block_list; }
#line 1465 "parser.cc"
    break;

  case 3: // blocks: main_block
#line 126 "parser.yy"
                                  { yylhs.value.as < Blocks > () = Blocks(yystack_[0].value.as < Block > ()); }
#line 1471 "parser.cc"
    break;

  case 4: // blocks: block
#line 127 "parser.yy"
                                  { yylhs.value.as < Blocks > () = Blocks(yystack_[0].value.as < Block > ()); }
#line 1477 "parser.cc"
    break;

  case 5: // blocks: blocks block
#line 128 "parser.yy"
                                  { yylhs.value.as < Blocks > () = yystack_[1].value.as < Blocks > ().Add(yystack_[0].value.as < Block > ()); }
#line 1483 "parser.cc"
    break;

  case 6: // block: "also" one_or_more_statements "end" "also"
#line 131 "parser.yy"
                                              { yylhs.value.as < Block > () = Block::MainBlock(yystack_[2].value.as < Statements > ()); }
#line 1489 "parser.cc"
    break;

  case 7: // block: "when" exp one_or_more_statements "end" "when"
#line 132 "parser.yy"
                                                 { yylhs.value.as < Block > () = Block::WhenExpBlock(yystack_[3].value.as < Expression > (), yystack_[2].value.as < Statements > ()); }
#line 1495 "parser.cc"
    break;

  case 8: // main_block: one_or_more_statements
#line 135 "parser.yy"
                                  { yylhs.value.as < Block > () = Block::MainBlock(yystack_[0].value.as < Statements > ()); }
#line 1501 "parser.cc"
    break;

  case 9: // zero_or_more_statements: %empty
#line 138 "parser.yy"
                                     {}
#line 1507 "parser.cc"
    break;

  case 10: // zero_or_more_statements: zero_or_more_statements statement
#line 139 "parser.yy"
                                     { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1513 "parser.cc"
    break;

  case 11: // one_or_more_statements: statement
#line 142 "parser.yy"
                                   { yylhs.value.as < Statements > ()  = Statements::FirstStatement(yystack_[0].value.as < Line > ()); }
#line 1519 "parser.cc"
    break;

  case 12: // one_or_more_statements: one_or_more_statements statement
#line 143 "parser.yy"
                                   { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1525 "parser.cc"
    break;

  case 13: // statement: array_assignment
#line 146 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1531 "parser.cc"
    break;

  case 14: // statement: assignment
#line 147 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1537 "parser.cc"
    break;

  case 15: // statement: clear_statement
#line 148 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1543 "parser.cc"
    break;

  case 16: // statement: continue_statement
#line 149 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1549 "parser.cc"
    break;

  case 17: // statement: exit_statement
#line 150 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1555 "parser.cc"
    break;

  case 18: // statement: for_statement
#line 151 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1561 "parser.cc"
    break;

  case 19: // statement: if_statement
#line 152 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1567 "parser.cc"
    break;

  case 20: // statement: reset_statement
#line 153 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1573 "parser.cc"
    break;

  case 21: // statement: wait_statement
#line 154 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1579 "parser.cc"
    break;

  case 22: // array_assignment: "identifier" "[" exp "]" "=" exp
#line 157 "parser.yy"
                                    { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1585 "parser.cc"
    break;

  case 23: // array_assignment: "identifier" "[" exp "]" "=" "{" expression_list "}"
#line 158 "parser.yy"
                                                        { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[7].value.as < std::string > (), yystack_[5].value.as < Expression > (), yystack_[1].value.as < ExpressionList > (), &drv); }
#line 1591 "parser.cc"
    break;

  case 24: // assignment: "identifier" "=" exp
#line 161 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1597 "parser.cc"
    break;

  case 25: // assignment: "in_port" "=" exp
#line 162 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1603 "parser.cc"
    break;

  case 26: // assignment: "out_port" "=" exp
#line 163 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1609 "parser.cc"
    break;

  case 27: // clear_statement: "clear" "all"
#line 166 "parser.yy"
                        { yylhs.value.as < Line > () = Line::ClearAll(); }
#line 1615 "parser.cc"
    break;

  case 28: // continue_statement: "continue" "for"
#line 169 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1621 "parser.cc"
    break;

  case 29: // continue_statement: "continue" "all"
#line 170 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1627 "parser.cc"
    break;

  case 30: // exit_statement: "exit" "for"
#line 173 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1633 "parser.cc"
    break;

  case 31: // exit_statement: "exit" "all"
#line 174 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1639 "parser.cc"
    break;

  case 32: // for_statement: "for" assignment "to" exp zero_or_more_statements "next"
#line 177 "parser.yy"
                                                            { yylhs.value.as < Line > () = Line::ForNext(yystack_[4].value.as < Line > (), yystack_[2].value.as < Expression > (), drv.factory.Number(1.0), yystack_[1].value.as < Statements > (), &drv); }
#line 1645 "parser.cc"
    break;

  case 33: // for_statement: "for" assignment "to" exp "step" exp zero_or_more_statements "next"
#line 178 "parser.yy"
                                                                      { yylhs.value.as < Line > () = Line::ForNext(yystack_[6].value.as < Line > (), yystack_[4].value.as < Expression > (), yystack_[2].value.as < Expression > (), yystack_[1].value.as < Statements > (), &drv); }
#line 1651 "parser.cc"
    break;

  case 34: // elseif_group: %empty
#line 181 "parser.yy"
                                  {}
#line 1657 "parser.cc"
    break;

  case 35: // elseif_group: elseif_group elseif_clause
#line 182 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1663 "parser.cc"
    break;

  case 36: // elseif_clause: "elseif" exp "then" zero_or_more_statements
#line 185 "parser.yy"
                                               { yylhs.value.as < Line > () = Line::ElseIf(yystack_[2].value.as < Expression > (), yystack_[0].value.as < Statements > ()); }
#line 1669 "parser.cc"
    break;

  case 37: // if_statement: "if" exp "then" zero_or_more_statements elseif_group "end" "if"
#line 188 "parser.yy"
                                                                        { yylhs.value.as < Line > () = Line::IfThen(yystack_[5].value.as < Expression > (), yystack_[3].value.as < Statements > (), yystack_[2].value.as < Statements > ()); }
#line 1675 "parser.cc"
    break;

  case 38: // if_statement: "if" exp "then" zero_or_more_statements elseif_group "else" zero_or_more_statements "end" "if"
#line 189 "parser.yy"
                                                                                                  { yylhs.value.as < Line > () = Line::IfThenElse(yystack_[7].value.as < Expression > (), yystack_[5].value.as < Statements > (), yystack_[2].value.as < Statements > (), yystack_[4].value.as < Statements > ()); }
#line 1681 "parser.cc"
    break;

  case 39: // reset_statement: "reset"
#line 192 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Reset(); }
#line 1687 "parser.cc"
    break;

  case 40: // wait_statement: "wait" exp
#line 195 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Wait(yystack_[0].value.as < Expression > ()); }
#line 1693 "parser.cc"
    break;

  case 41: // expression_list: exp
#line 205 "parser.yy"
                              { yylhs.value.as < ExpressionList > () = ExpressionList(yystack_[0].value.as < Expression > ()); }
#line 1699 "parser.cc"
    break;

  case 42: // expression_list: expression_list "," exp
#line 206 "parser.yy"
                              { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 1705 "parser.cc"
    break;

  case 43: // exp: "number"
#line 209 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Number((float) yystack_[0].value.as < float > ()); }
#line 1711 "parser.cc"
    break;

  case 44: // exp: "note"
#line 210 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Note(yystack_[0].value.as < std::string > ()); }
#line 1717 "parser.cc"
    break;

  case 45: // exp: "-" "number"
#line 211 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.Number(-1 * (float) yystack_[0].value.as < float > ());}
#line 1723 "parser.cc"
    break;

  case 46: // exp: "not" exp
#line 212 "parser.yy"
                      { yylhs.value.as < Expression > () = drv.factory.Not(yystack_[0].value.as < Expression > ());}
#line 1729 "parser.cc"
    break;

  case 47: // exp: "in_port"
#line 213 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1735 "parser.cc"
    break;

  case 48: // exp: "out_port"
#line 214 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1741 "parser.cc"
    break;

  case 49: // exp: "identifier"
#line 215 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1747 "parser.cc"
    break;

  case 50: // exp: "identifier" "[" exp "]"
#line 216 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.ArrayVariable(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv); }
#line 1753 "parser.cc"
    break;

  case 51: // exp: exp "+" exp
#line 217 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1759 "parser.cc"
    break;

  case 52: // exp: exp "-" exp
#line 218 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1765 "parser.cc"
    break;

  case 53: // exp: exp "*" exp
#line 219 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1771 "parser.cc"
    break;

  case 54: // exp: exp "/" exp
#line 220 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1777 "parser.cc"
    break;

  case 55: // exp: exp "comparison" exp
#line 221 "parser.yy"
                       { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1783 "parser.cc"
    break;

  case 56: // exp: exp "and" exp
#line 222 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1789 "parser.cc"
    break;

  case 57: // exp: exp "or" exp
#line 223 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1795 "parser.cc"
    break;

  case 58: // exp: "zeroargfunc" "(" ")"
#line 224 "parser.yy"
                        { yylhs.value.as < Expression > () = drv.factory.ZeroArgFunc(yystack_[2].value.as < std::string > ()); }
#line 1801 "parser.cc"
    break;

  case 59: // exp: "connected" "(" "in_port" ")"
#line 225 "parser.yy"
                                {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 1807 "parser.cc"
    break;

  case 60: // exp: "connected" "(" "out_port" ")"
#line 226 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 1813 "parser.cc"
    break;

  case 61: // exp: "trigger" "(" "in_port" ")"
#line 227 "parser.yy"
                                {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 1819 "parser.cc"
    break;

  case 62: // exp: "oneargfunc" "(" exp ")"
#line 228 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.OneArgFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > ()); }
#line 1825 "parser.cc"
    break;

  case 63: // exp: "twoargfunc" "(" exp "," exp ")"
#line 229 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.TwoArgFunc(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > ()); }
#line 1831 "parser.cc"
    break;

  case 64: // exp: "(" exp ")"
#line 230 "parser.yy"
                { yylhs.value.as < Expression > () = yystack_[1].value.as < Expression > (); }
#line 1837 "parser.cc"
    break;


#line 1841 "parser.cc"

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
  Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  const char *
  Parser::symbol_name (symbol_kind_type yysymbol)
  {
    static const char *const yy_sname[] =
    {
    "end of file", "error", "invalid token", "abs", "all", "also", "and",
  "=", "ceiling", "clear", "connected", "continue", "else", "elseif",
  "end", "exit", "floor", "for", "if", "max", "min", "next", "not", "or",
  "pow", "reset", "sample_rate", "sign", "sin", "start", "step", "then",
  "to", "trigger", "wait", "when", "-", "+", "*", "/", "(", ")", "{", "}",
  "[", "]", ",", "identifier", "number", "note", "in_port", "out_port",
  "zeroargfunc", "oneargfunc", "twoargfunc", "comparison", "NEG",
  "$accept", "program", "blocks", "block", "main_block",
  "zero_or_more_statements", "one_or_more_statements", "statement",
  "array_assignment", "assignment", "clear_statement",
  "continue_statement", "exit_statement", "for_statement", "elseif_group",
  "elseif_clause", "if_statement", "reset_statement", "wait_statement",
  "expression_list", "exp", YY_NULLPTR
    };
    return yy_sname[yysymbol];
  }



  // Parser::context.
  Parser::context::context (const Parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  Parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
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
  Parser::yy_lac_check_ (symbol_kind_type yytoken) const
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
  Parser::yy_lac_establish_ (symbol_kind_type yytoken)
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
  Parser::yy_lac_discard_ (const char* event)
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
  Parser::yy_syntax_error_arguments_ (const context& yyctx,
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
  Parser::yysyntax_error_ (const context& yyctx) const
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


  const signed char Parser::yypact_ninf_ = -45;

  const signed char Parser::yytable_ninf_ = -1;

  const short
  Parser::yypact_[] =
  {
     241,   392,    22,    10,    17,   -28,   264,   -45,   264,   264,
      -2,    42,    43,    24,    13,   -45,   -45,   392,   -45,   -45,
     -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,   310,   -45,
     -45,   -45,   -45,   -45,    44,   -15,   -10,   264,    16,    29,
     264,    21,   -45,   -45,   -45,   -45,    31,    32,    39,     2,
      37,    72,   264,   264,   264,   264,   -45,   -45,   -45,   -45,
      75,   264,   -44,   -45,    34,   -45,    93,   264,    45,   264,
     264,   264,   264,   -45,   264,   264,   264,   264,   264,   322,
      37,   114,    37,    37,   -45,   119,    47,    50,    52,   -45,
     139,   -45,   159,   165,   -27,   179,   392,   -23,   -23,   -45,
     -45,   194,    59,    78,   264,   334,   -45,   -45,   -45,   -45,
     -45,   264,   -45,    23,   -45,   231,    37,   -45,   185,   -45,
     264,    80,   -45,   264,    37,   365,   -45,   380,   206,   -45,
      12,    37,   -45,    82,   -45,   -45,   264,   -45,   392,    37
  };

  const signed char
  Parser::yydefact_[] =
  {
       0,     0,     0,     0,     0,     0,     0,    39,     0,     0,
       0,     0,     0,     0,     0,     4,     3,     8,    11,    13,
      14,    15,    16,    17,    18,    19,    20,    21,     0,    27,
      29,    28,    31,    30,     0,     0,     0,     0,     0,     0,
       0,    49,    43,    44,    47,    48,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     1,     2,     5,    12,
       0,     0,     0,    46,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     9,     0,     0,     0,     0,     0,     0,
      24,     0,    25,    26,     6,     9,     0,     0,     0,    64,
       0,    58,     0,     0,    56,    57,    34,    52,    51,    53,
      54,    55,     0,     0,     0,     0,    59,    60,    61,    50,
      62,     0,    10,     0,     7,     0,     9,    32,     0,     9,
       0,     0,    35,     0,    22,     0,    63,     0,     0,    37,
       0,    41,    33,     0,     9,    23,     0,    38,    36,    42
  };

  const signed char
  Parser::yypgoto_[] =
  {
     -45,   -45,   -45,    87,   -45,    20,     1,     3,   -45,    97,
     -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,
      -8
  };

  const unsigned char
  Parser::yydefgoto_[] =
  {
       0,    13,    14,    15,    16,    96,    17,   112,    19,    20,
      21,    22,    23,    24,   113,   122,    25,    26,    27,   130,
      49
  };

  const unsigned char
  Parser::yytable_[] =
  {
      50,    51,    28,    18,    18,    52,    86,    87,    71,    74,
      75,    76,    77,    57,    30,    76,    77,    61,     1,    34,
      59,    32,    11,    12,    56,    72,    29,    31,    78,    63,
      62,    59,    66,    73,    33,   119,   120,   121,    74,    75,
      76,    77,    53,    71,    80,    81,    82,    83,     9,    54,
      55,    52,    79,    85,    18,   135,    64,    78,   136,    90,
      72,    92,    93,    94,    95,    67,    97,    98,    99,   100,
     101,    68,    69,    74,    75,    76,    77,    65,    71,    70,
      84,     2,    59,     3,    88,   115,    91,     4,   106,     5,
       6,   107,    78,   108,   114,    72,   116,     7,   129,    71,
     137,    58,    35,   118,     0,   105,     8,   124,    74,    75,
      76,    77,   128,     0,     0,   131,    72,     0,     0,    10,
      71,     0,    11,    12,     0,    71,     0,    78,   139,    74,
      75,    76,    77,     0,    89,     0,   125,    72,     0,   127,
       0,     0,    72,     0,     0,    71,     0,     0,    78,   104,
      74,    75,    76,    77,   138,    74,    75,    76,    77,   103,
       0,     0,    72,     0,     0,    71,     0,     0,     0,    78,
       0,    71,     0,     0,    78,    74,    75,    76,    77,     0,
       0,     0,    72,     0,   109,    71,     0,     0,    72,     0,
       0,    71,     0,     0,    78,    74,    75,    76,    77,     0,
     110,    74,    75,    76,    77,     0,     0,     0,    72,     0,
       0,   111,    71,     0,    78,    74,    75,    76,    77,     0,
      78,    74,    75,    76,    77,     0,   126,     0,     0,    72,
      74,    75,    76,    77,    78,     0,     0,   134,     0,     0,
      78,    36,    74,    75,    76,    77,     1,     0,     0,     0,
       2,     0,     3,    37,     0,     0,     4,     0,     5,     6,
       0,    78,     0,     0,    38,     0,     7,    39,     0,     0,
       0,    40,     0,   123,    36,     8,     9,     0,    41,    42,
      43,    44,    45,    46,    47,    48,    37,     0,    10,     0,
       0,    11,    12,     0,     0,     0,     0,    38,     0,     0,
      39,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,    41,    42,    43,    44,    45,    46,    47,    48,     2,
       0,     3,     0,     0,    60,     4,     0,     5,     6,     0,
       0,     2,     0,     3,     0,     7,   102,     4,     0,     5,
       6,     0,     0,     2,     8,     3,     0,     7,     0,     4,
       0,     5,     6,     0,     0,   117,     8,    10,     0,     7,
      11,    12,     0,     0,     0,     0,     0,     0,     8,    10,
       0,     0,    11,    12,     2,     0,     3,     0,     0,     0,
       4,    10,     5,     6,    11,    12,   132,     0,     0,     2,
       7,     3,     0,     0,   133,     4,     0,     5,     6,     8,
       0,     2,     0,     3,     0,     7,     0,     4,     0,     5,
       6,     0,    10,     0,     8,    11,    12,     7,     0,     0,
       0,     0,     0,     0,     0,     0,     8,    10,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,    10,
       0,     0,    11,    12
  };

  const short
  Parser::yycheck_[] =
  {
       8,     9,     1,     0,     1,     7,    50,    51,     6,    36,
      37,    38,    39,     0,     4,    38,    39,    32,     5,    47,
      17,     4,    50,    51,     0,    23,     4,    17,    55,    37,
      40,    28,    40,    31,    17,    12,    13,    14,    36,    37,
      38,    39,    44,     6,    52,    53,    54,    55,    35,     7,
       7,     7,    51,    61,    51,    43,    40,    55,    46,    67,
      23,    69,    70,    71,    72,    44,    74,    75,    76,    77,
      78,    40,    40,    36,    37,    38,    39,    48,     6,    40,
       5,     9,    79,    11,    50,     7,    41,    15,    41,    17,
      18,    41,    55,    41,    35,    23,   104,    25,    18,     6,
      18,    14,     5,   111,    -1,    85,    34,   115,    36,    37,
      38,    39,   120,    -1,    -1,   123,    23,    -1,    -1,    47,
       6,    -1,    50,    51,    -1,     6,    -1,    55,   136,    36,
      37,    38,    39,    -1,    41,    -1,   116,    23,    -1,   119,
      -1,    -1,    23,    -1,    -1,     6,    -1,    -1,    55,    30,
      36,    37,    38,    39,   134,    36,    37,    38,    39,    45,
      -1,    -1,    23,    -1,    -1,     6,    -1,    -1,    -1,    55,
      -1,     6,    -1,    -1,    55,    36,    37,    38,    39,    -1,
      -1,    -1,    23,    -1,    45,     6,    -1,    -1,    23,    -1,
      -1,     6,    -1,    -1,    55,    36,    37,    38,    39,    -1,
      41,    36,    37,    38,    39,    -1,    -1,    -1,    23,    -1,
      -1,    46,     6,    -1,    55,    36,    37,    38,    39,    -1,
      55,    36,    37,    38,    39,    -1,    41,    -1,    -1,    23,
      36,    37,    38,    39,    55,    -1,    -1,    31,    -1,    -1,
      55,    10,    36,    37,    38,    39,     5,    -1,    -1,    -1,
       9,    -1,    11,    22,    -1,    -1,    15,    -1,    17,    18,
      -1,    55,    -1,    -1,    33,    -1,    25,    36,    -1,    -1,
      -1,    40,    -1,    42,    10,    34,    35,    -1,    47,    48,
      49,    50,    51,    52,    53,    54,    22,    -1,    47,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    33,    -1,    -1,
      36,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    48,    49,    50,    51,    52,    53,    54,     9,
      -1,    11,    -1,    -1,    14,    15,    -1,    17,    18,    -1,
      -1,     9,    -1,    11,    -1,    25,    14,    15,    -1,    17,
      18,    -1,    -1,     9,    34,    11,    -1,    25,    -1,    15,
      -1,    17,    18,    -1,    -1,    21,    34,    47,    -1,    25,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    34,    47,
      -1,    -1,    50,    51,     9,    -1,    11,    -1,    -1,    -1,
      15,    47,    17,    18,    50,    51,    21,    -1,    -1,     9,
      25,    11,    -1,    -1,    14,    15,    -1,    17,    18,    34,
      -1,     9,    -1,    11,    -1,    25,    -1,    15,    -1,    17,
      18,    -1,    47,    -1,    34,    50,    51,    25,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    34,    47,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    50,    51
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     5,     9,    11,    15,    17,    18,    25,    34,    35,
      47,    50,    51,    58,    59,    60,    61,    63,    64,    65,
      66,    67,    68,    69,    70,    73,    74,    75,    63,     4,
       4,    17,     4,    17,    47,    66,    10,    22,    33,    36,
      40,    47,    48,    49,    50,    51,    52,    53,    54,    77,
      77,    77,     7,    44,     7,     7,     0,     0,    60,    64,
      14,    32,    40,    77,    40,    48,    77,    44,    40,    40,
      40,     6,    23,    31,    36,    37,    38,    39,    55,    63,
      77,    77,    77,    77,     5,    77,    50,    51,    50,    41,
      77,    41,    77,    77,    77,    77,    62,    77,    77,    77,
      77,    77,    14,    45,    30,    62,    41,    41,    41,    45,
      41,    46,    64,    71,    35,     7,    77,    21,    77,    12,
      13,    14,    72,    42,    77,    62,    41,    62,    77,    18,
      76,    77,    21,    14,    31,    43,    46,    18,    62,    77
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    57,    58,    59,    59,    59,    60,    60,    61,    62,
      62,    63,    63,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    65,    65,    66,    66,    66,    67,    68,    68,
      69,    69,    70,    70,    71,    71,    72,    73,    73,    74,
      75,    76,    76,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     2,     1,     1,     2,     4,     5,     1,     0,
       2,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     6,     8,     3,     3,     3,     2,     2,     2,
       2,     2,     6,     8,     0,     2,     4,     7,     9,     1,
       2,     1,     3,     1,     1,     2,     2,     1,     1,     1,
       4,     3,     3,     3,     3,     3,     3,     3,     3,     4,
       4,     4,     4,     6,     3
  };




#if YYDEBUG
  const unsigned char
  Parser::yyrline_[] =
  {
       0,   123,   123,   126,   127,   128,   131,   132,   135,   138,
     139,   142,   143,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   157,   158,   161,   162,   163,   166,   169,   170,
     173,   174,   177,   178,   181,   182,   185,   188,   189,   192,
     195,   205,   206,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230
  };

  void
  Parser::yy_stack_print_ () const
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
  Parser::yy_reduce_print_ (int yyrule) const
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
#line 2552 "parser.cc"

#line 231 "parser.yy"


void
yy::Parser::error (const location_type& l, const std::string& m)
{
  drv.errors.push_back(Error(l.begin.line, l.begin.column, m));
}
