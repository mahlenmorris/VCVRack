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
      case symbol_kind::S_string_exp: // string_exp
        value.YY_MOVE_OR_COPY< Expression > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expression_list: // expression_list
      case symbol_kind::S_string_list: // string_list
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
      case symbol_kind::S_print_statement: // print_statement
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
      case symbol_kind::S_DEBUG: // "debug"
      case symbol_kind::S_ELSE: // "else"
      case symbol_kind::S_ELSEIF: // "elseif"
      case symbol_kind::S_END: // "end"
      case symbol_kind::S_EXIT: // "exit"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_FOR: // "for"
      case symbol_kind::S_IF: // "if"
      case symbol_kind::S_LOG2: // "log2"
      case symbol_kind::S_LOGE: // "loge"
      case symbol_kind::S_LOG10: // "log10"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_NEXT: // "next"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_PRINT: // "print"
      case symbol_kind::S_RESET: // "reset"
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TIME: // "time"
      case symbol_kind::S_TIME_MILLIS: // "time_millis"
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
      case symbol_kind::S_DOLLAR: // "$"
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
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
      case symbol_kind::S_string_exp: // string_exp
        value.move< Expression > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expression_list: // expression_list
      case symbol_kind::S_string_list: // string_list
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
      case symbol_kind::S_print_statement: // print_statement
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
      case symbol_kind::S_DEBUG: // "debug"
      case symbol_kind::S_ELSE: // "else"
      case symbol_kind::S_ELSEIF: // "elseif"
      case symbol_kind::S_END: // "end"
      case symbol_kind::S_EXIT: // "exit"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_FOR: // "for"
      case symbol_kind::S_IF: // "if"
      case symbol_kind::S_LOG2: // "log2"
      case symbol_kind::S_LOGE: // "loge"
      case symbol_kind::S_LOG10: // "log10"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_NEXT: // "next"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_PRINT: // "print"
      case symbol_kind::S_RESET: // "reset"
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TIME: // "time"
      case symbol_kind::S_TIME_MILLIS: // "time_millis"
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
      case symbol_kind::S_DOLLAR: // "$"
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
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
      case symbol_kind::S_string_exp: // string_exp
        value.copy< Expression > (that.value);
        break;

      case symbol_kind::S_expression_list: // expression_list
      case symbol_kind::S_string_list: // string_list
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
      case symbol_kind::S_print_statement: // print_statement
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
      case symbol_kind::S_DEBUG: // "debug"
      case symbol_kind::S_ELSE: // "else"
      case symbol_kind::S_ELSEIF: // "elseif"
      case symbol_kind::S_END: // "end"
      case symbol_kind::S_EXIT: // "exit"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_FOR: // "for"
      case symbol_kind::S_IF: // "if"
      case symbol_kind::S_LOG2: // "log2"
      case symbol_kind::S_LOGE: // "loge"
      case symbol_kind::S_LOG10: // "log10"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_NEXT: // "next"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_PRINT: // "print"
      case symbol_kind::S_RESET: // "reset"
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TIME: // "time"
      case symbol_kind::S_TIME_MILLIS: // "time_millis"
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
      case symbol_kind::S_DOLLAR: // "$"
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
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
      case symbol_kind::S_string_exp: // string_exp
        value.move< Expression > (that.value);
        break;

      case symbol_kind::S_expression_list: // expression_list
      case symbol_kind::S_string_list: // string_list
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
      case symbol_kind::S_print_statement: // print_statement
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
      case symbol_kind::S_DEBUG: // "debug"
      case symbol_kind::S_ELSE: // "else"
      case symbol_kind::S_ELSEIF: // "elseif"
      case symbol_kind::S_END: // "end"
      case symbol_kind::S_EXIT: // "exit"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_FOR: // "for"
      case symbol_kind::S_IF: // "if"
      case symbol_kind::S_LOG2: // "log2"
      case symbol_kind::S_LOGE: // "loge"
      case symbol_kind::S_LOG10: // "log10"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_NEXT: // "next"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_PRINT: // "print"
      case symbol_kind::S_RESET: // "reset"
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TIME: // "time"
      case symbol_kind::S_TIME_MILLIS: // "time_millis"
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
      case symbol_kind::S_DOLLAR: // "$"
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
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
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 728 "parser.cc"
        break;

      case symbol_kind::S_ALL: // "all"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 734 "parser.cc"
        break;

      case symbol_kind::S_ALSO: // "also"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 740 "parser.cc"
        break;

      case symbol_kind::S_AND: // "and"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 746 "parser.cc"
        break;

      case symbol_kind::S_ASSIGN: // "="
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 752 "parser.cc"
        break;

      case symbol_kind::S_CEILING: // "ceiling"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 758 "parser.cc"
        break;

      case symbol_kind::S_CLEAR: // "clear"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 764 "parser.cc"
        break;

      case symbol_kind::S_CONNECTED: // "connected"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 770 "parser.cc"
        break;

      case symbol_kind::S_CONTINUE: // "continue"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 776 "parser.cc"
        break;

      case symbol_kind::S_DEBUG: // "debug"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 782 "parser.cc"
        break;

      case symbol_kind::S_ELSE: // "else"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 788 "parser.cc"
        break;

      case symbol_kind::S_ELSEIF: // "elseif"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 794 "parser.cc"
        break;

      case symbol_kind::S_END: // "end"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 800 "parser.cc"
        break;

      case symbol_kind::S_EXIT: // "exit"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 806 "parser.cc"
        break;

      case symbol_kind::S_FLOOR: // "floor"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 812 "parser.cc"
        break;

      case symbol_kind::S_FOR: // "for"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 818 "parser.cc"
        break;

      case symbol_kind::S_IF: // "if"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 824 "parser.cc"
        break;

      case symbol_kind::S_LOG2: // "log2"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 830 "parser.cc"
        break;

      case symbol_kind::S_LOGE: // "loge"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 836 "parser.cc"
        break;

      case symbol_kind::S_LOG10: // "log10"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 842 "parser.cc"
        break;

      case symbol_kind::S_MAX: // "max"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 848 "parser.cc"
        break;

      case symbol_kind::S_MIN: // "min"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 854 "parser.cc"
        break;

      case symbol_kind::S_NEXT: // "next"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 860 "parser.cc"
        break;

      case symbol_kind::S_NOT: // "not"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 866 "parser.cc"
        break;

      case symbol_kind::S_OR: // "or"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 872 "parser.cc"
        break;

      case symbol_kind::S_POW: // "pow"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 878 "parser.cc"
        break;

      case symbol_kind::S_PRINT: // "print"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 884 "parser.cc"
        break;

      case symbol_kind::S_RESET: // "reset"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 890 "parser.cc"
        break;

      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 896 "parser.cc"
        break;

      case symbol_kind::S_SIGN: // "sign"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 902 "parser.cc"
        break;

      case symbol_kind::S_SIN: // "sin"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 908 "parser.cc"
        break;

      case symbol_kind::S_START: // "start"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 914 "parser.cc"
        break;

      case symbol_kind::S_STEP: // "step"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 920 "parser.cc"
        break;

      case symbol_kind::S_THEN: // "then"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 926 "parser.cc"
        break;

      case symbol_kind::S_TIME: // "time"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 932 "parser.cc"
        break;

      case symbol_kind::S_TIME_MILLIS: // "time_millis"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 938 "parser.cc"
        break;

      case symbol_kind::S_TO: // "to"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 944 "parser.cc"
        break;

      case symbol_kind::S_TRIGGER: // "trigger"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 950 "parser.cc"
        break;

      case symbol_kind::S_WAIT: // "wait"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 956 "parser.cc"
        break;

      case symbol_kind::S_WHEN: // "when"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 962 "parser.cc"
        break;

      case symbol_kind::S_MINUS: // "-"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 968 "parser.cc"
        break;

      case symbol_kind::S_PLUS: // "+"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 974 "parser.cc"
        break;

      case symbol_kind::S_STAR: // "*"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 980 "parser.cc"
        break;

      case symbol_kind::S_SLASH: // "/"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 986 "parser.cc"
        break;

      case symbol_kind::S_LPAREN: // "("
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 992 "parser.cc"
        break;

      case symbol_kind::S_RPAREN: // ")"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 998 "parser.cc"
        break;

      case symbol_kind::S_LBRACE: // "{"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1004 "parser.cc"
        break;

      case symbol_kind::S_RBRACE: // "}"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1010 "parser.cc"
        break;

      case symbol_kind::S_LBRACKET: // "["
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1016 "parser.cc"
        break;

      case symbol_kind::S_RBRACKET: // "]"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1022 "parser.cc"
        break;

      case symbol_kind::S_COMMA: // ","
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1028 "parser.cc"
        break;

      case symbol_kind::S_DOLLAR: // "$"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1034 "parser.cc"
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1040 "parser.cc"
        break;

      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1046 "parser.cc"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < float > (); }
#line 1052 "parser.cc"
        break;

      case symbol_kind::S_NOTE: // "note"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1058 "parser.cc"
        break;

      case symbol_kind::S_IN_PORT: // "in_port"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1064 "parser.cc"
        break;

      case symbol_kind::S_OUT_PORT: // "out_port"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1070 "parser.cc"
        break;

      case symbol_kind::S_ZEROARGFUNC: // "zeroargfunc"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1076 "parser.cc"
        break;

      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1082 "parser.cc"
        break;

      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1088 "parser.cc"
        break;

      case symbol_kind::S_COMPARISON: // "comparison"
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1094 "parser.cc"
        break;

      case symbol_kind::S_blocks: // blocks
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Blocks > (); }
#line 1100 "parser.cc"
        break;

      case symbol_kind::S_block: // block
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Block > (); }
#line 1106 "parser.cc"
        break;

      case symbol_kind::S_main_block: // main_block
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Block > (); }
#line 1112 "parser.cc"
        break;

      case symbol_kind::S_zero_or_more_statements: // zero_or_more_statements
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1118 "parser.cc"
        break;

      case symbol_kind::S_one_or_more_statements: // one_or_more_statements
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1124 "parser.cc"
        break;

      case symbol_kind::S_statement: // statement
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1130 "parser.cc"
        break;

      case symbol_kind::S_array_assignment: // array_assignment
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1136 "parser.cc"
        break;

      case symbol_kind::S_assignment: // assignment
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1142 "parser.cc"
        break;

      case symbol_kind::S_clear_statement: // clear_statement
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1148 "parser.cc"
        break;

      case symbol_kind::S_continue_statement: // continue_statement
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1154 "parser.cc"
        break;

      case symbol_kind::S_exit_statement: // exit_statement
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1160 "parser.cc"
        break;

      case symbol_kind::S_for_statement: // for_statement
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1166 "parser.cc"
        break;

      case symbol_kind::S_elseif_group: // elseif_group
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1172 "parser.cc"
        break;

      case symbol_kind::S_elseif_clause: // elseif_clause
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1178 "parser.cc"
        break;

      case symbol_kind::S_if_statement: // if_statement
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1184 "parser.cc"
        break;

      case symbol_kind::S_print_statement: // print_statement
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1190 "parser.cc"
        break;

      case symbol_kind::S_reset_statement: // reset_statement
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1196 "parser.cc"
        break;

      case symbol_kind::S_wait_statement: // wait_statement
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1202 "parser.cc"
        break;

      case symbol_kind::S_expression_list: // expression_list
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < ExpressionList > (); }
#line 1208 "parser.cc"
        break;

      case symbol_kind::S_exp: // exp
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Expression > (); }
#line 1214 "parser.cc"
        break;

      case symbol_kind::S_string_list: // string_list
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < ExpressionList > (); }
#line 1220 "parser.cc"
        break;

      case symbol_kind::S_string_exp: // string_exp
#line 129 "parser.yy"
                 { yyo << yysym.value.template as < Expression > (); }
#line 1226 "parser.cc"
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
      case symbol_kind::S_string_exp: // string_exp
        yylhs.value.emplace< Expression > ();
        break;

      case symbol_kind::S_expression_list: // expression_list
      case symbol_kind::S_string_list: // string_list
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
      case symbol_kind::S_print_statement: // print_statement
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
      case symbol_kind::S_DEBUG: // "debug"
      case symbol_kind::S_ELSE: // "else"
      case symbol_kind::S_ELSEIF: // "elseif"
      case symbol_kind::S_END: // "end"
      case symbol_kind::S_EXIT: // "exit"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_FOR: // "for"
      case symbol_kind::S_IF: // "if"
      case symbol_kind::S_LOG2: // "log2"
      case symbol_kind::S_LOGE: // "loge"
      case symbol_kind::S_LOG10: // "log10"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_NEXT: // "next"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_PRINT: // "print"
      case symbol_kind::S_RESET: // "reset"
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TIME: // "time"
      case symbol_kind::S_TIME_MILLIS: // "time_millis"
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
      case symbol_kind::S_DOLLAR: // "$"
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
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
#line 135 "parser.yy"
                                  { drv.blocks = yystack_[1].value.as < Blocks > ().block_list; }
#line 1597 "parser.cc"
    break;

  case 3: // blocks: main_block
#line 138 "parser.yy"
                                  { yylhs.value.as < Blocks > () = Blocks(yystack_[0].value.as < Block > ()); }
#line 1603 "parser.cc"
    break;

  case 4: // blocks: block
#line 139 "parser.yy"
                                  { yylhs.value.as < Blocks > () = Blocks(yystack_[0].value.as < Block > ()); }
#line 1609 "parser.cc"
    break;

  case 5: // blocks: blocks block
#line 140 "parser.yy"
                                  { yylhs.value.as < Blocks > () = yystack_[1].value.as < Blocks > ().Add(yystack_[0].value.as < Block > ()); }
#line 1615 "parser.cc"
    break;

  case 6: // block: "also" one_or_more_statements "end" "also"
#line 143 "parser.yy"
                                              { yylhs.value.as < Block > () = Block::MainBlock(yystack_[2].value.as < Statements > ()); }
#line 1621 "parser.cc"
    break;

  case 7: // block: "when" exp one_or_more_statements "end" "when"
#line 144 "parser.yy"
                                                 { yylhs.value.as < Block > () = Block::WhenExpBlock(yystack_[3].value.as < Expression > (), yystack_[2].value.as < Statements > ()); }
#line 1627 "parser.cc"
    break;

  case 8: // main_block: one_or_more_statements
#line 147 "parser.yy"
                                  { yylhs.value.as < Block > () = Block::MainBlock(yystack_[0].value.as < Statements > ()); }
#line 1633 "parser.cc"
    break;

  case 9: // zero_or_more_statements: %empty
#line 150 "parser.yy"
                                     {}
#line 1639 "parser.cc"
    break;

  case 10: // zero_or_more_statements: zero_or_more_statements statement
#line 151 "parser.yy"
                                     { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1645 "parser.cc"
    break;

  case 11: // one_or_more_statements: statement
#line 154 "parser.yy"
                                   { yylhs.value.as < Statements > ()  = Statements::FirstStatement(yystack_[0].value.as < Line > ()); }
#line 1651 "parser.cc"
    break;

  case 12: // one_or_more_statements: one_or_more_statements statement
#line 155 "parser.yy"
                                   { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1657 "parser.cc"
    break;

  case 13: // statement: array_assignment
#line 158 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1663 "parser.cc"
    break;

  case 14: // statement: assignment
#line 159 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1669 "parser.cc"
    break;

  case 15: // statement: clear_statement
#line 160 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1675 "parser.cc"
    break;

  case 16: // statement: continue_statement
#line 161 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1681 "parser.cc"
    break;

  case 17: // statement: exit_statement
#line 162 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1687 "parser.cc"
    break;

  case 18: // statement: for_statement
#line 163 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1693 "parser.cc"
    break;

  case 19: // statement: if_statement
#line 164 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1699 "parser.cc"
    break;

  case 20: // statement: print_statement
#line 165 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1705 "parser.cc"
    break;

  case 21: // statement: reset_statement
#line 166 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1711 "parser.cc"
    break;

  case 22: // statement: wait_statement
#line 167 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1717 "parser.cc"
    break;

  case 23: // array_assignment: "identifier" "[" exp "]" "=" exp
#line 170 "parser.yy"
                                    { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1723 "parser.cc"
    break;

  case 24: // array_assignment: "identifier" "[" exp "]" "=" "{" expression_list "}"
#line 171 "parser.yy"
                                                        { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[7].value.as < std::string > (), yystack_[5].value.as < Expression > (), yystack_[1].value.as < ExpressionList > (), &drv); }
#line 1729 "parser.cc"
    break;

  case 25: // array_assignment: "identifier" "$" "[" exp "]" "=" exp
#line 172 "parser.yy"
                                        { yylhs.value.as < Line > () = Line::StringArrayAssignment(yystack_[6].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1735 "parser.cc"
    break;

  case 26: // array_assignment: "identifier" "$" "[" exp "]" "=" string_exp
#line 173 "parser.yy"
                                               { yylhs.value.as < Line > () = Line::StringArrayAssignment(yystack_[6].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1741 "parser.cc"
    break;

  case 27: // array_assignment: "identifier" "$" "[" exp "]" "=" "{" string_list "}"
#line 174 "parser.yy"
                                                        { yylhs.value.as < Line > () = Line::StringArrayAssignment(yystack_[8].value.as < std::string > (), yystack_[5].value.as < Expression > (), yystack_[1].value.as < ExpressionList > (), &drv); }
#line 1747 "parser.cc"
    break;

  case 28: // assignment: "identifier" "=" exp
#line 177 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1753 "parser.cc"
    break;

  case 29: // assignment: "in_port" "=" exp
#line 178 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1759 "parser.cc"
    break;

  case 30: // assignment: "out_port" "=" exp
#line 179 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1765 "parser.cc"
    break;

  case 31: // assignment: "identifier" "$" "=" exp
#line 180 "parser.yy"
                            { yylhs.value.as < Line > () = Line::StringAssignment(yystack_[3].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1771 "parser.cc"
    break;

  case 32: // assignment: "identifier" "$" "=" string_exp
#line 181 "parser.yy"
                                   { yylhs.value.as < Line > () = Line::StringAssignment(yystack_[3].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1777 "parser.cc"
    break;

  case 33: // clear_statement: "clear" "all"
#line 184 "parser.yy"
                        { yylhs.value.as < Line > () = Line::ClearAll(); }
#line 1783 "parser.cc"
    break;

  case 34: // continue_statement: "continue" "for"
#line 187 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1789 "parser.cc"
    break;

  case 35: // continue_statement: "continue" "all"
#line 188 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1795 "parser.cc"
    break;

  case 36: // exit_statement: "exit" "for"
#line 191 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1801 "parser.cc"
    break;

  case 37: // exit_statement: "exit" "all"
#line 192 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1807 "parser.cc"
    break;

  case 38: // for_statement: "for" assignment "to" exp zero_or_more_statements "next"
#line 195 "parser.yy"
                                                            { yylhs.value.as < Line > () = Line::ForNext(yystack_[4].value.as < Line > (), yystack_[2].value.as < Expression > (), drv.factory.Number(1.0), yystack_[1].value.as < Statements > (), &drv); }
#line 1813 "parser.cc"
    break;

  case 39: // for_statement: "for" assignment "to" exp "step" exp zero_or_more_statements "next"
#line 196 "parser.yy"
                                                                      { yylhs.value.as < Line > () = Line::ForNext(yystack_[6].value.as < Line > (), yystack_[4].value.as < Expression > (), yystack_[2].value.as < Expression > (), yystack_[1].value.as < Statements > (), &drv); }
#line 1819 "parser.cc"
    break;

  case 40: // elseif_group: %empty
#line 199 "parser.yy"
                                  {}
#line 1825 "parser.cc"
    break;

  case 41: // elseif_group: elseif_group elseif_clause
#line 200 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1831 "parser.cc"
    break;

  case 42: // elseif_clause: "elseif" exp "then" zero_or_more_statements
#line 203 "parser.yy"
                                               { yylhs.value.as < Line > () = Line::ElseIf(yystack_[2].value.as < Expression > (), yystack_[0].value.as < Statements > ()); }
#line 1837 "parser.cc"
    break;

  case 43: // if_statement: "if" exp "then" zero_or_more_statements elseif_group "end" "if"
#line 206 "parser.yy"
                                                                        { yylhs.value.as < Line > () = Line::IfThen(yystack_[5].value.as < Expression > (), yystack_[3].value.as < Statements > (), yystack_[2].value.as < Statements > ()); }
#line 1843 "parser.cc"
    break;

  case 44: // if_statement: "if" exp "then" zero_or_more_statements elseif_group "else" zero_or_more_statements "end" "if"
#line 207 "parser.yy"
                                                                                                  { yylhs.value.as < Line > () = Line::IfThenElse(yystack_[7].value.as < Expression > (), yystack_[5].value.as < Statements > (), yystack_[2].value.as < Statements > (), yystack_[4].value.as < Statements > ()); }
#line 1849 "parser.cc"
    break;

  case 45: // print_statement: "print" "(" "out_port" "," string_list ")"
#line 210 "parser.yy"
                                              {yylhs.value.as < Line > () = Line::Print(yystack_[3].value.as < std::string > (), yystack_[1].value.as < ExpressionList > (), &drv);}
#line 1855 "parser.cc"
    break;

  case 46: // reset_statement: "reset"
#line 213 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Reset(); }
#line 1861 "parser.cc"
    break;

  case 47: // wait_statement: "wait" exp
#line 216 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Wait(yystack_[0].value.as < Expression > ()); }
#line 1867 "parser.cc"
    break;

  case 48: // expression_list: exp
#line 226 "parser.yy"
                              { yylhs.value.as < ExpressionList > () = ExpressionList(yystack_[0].value.as < Expression > ()); }
#line 1873 "parser.cc"
    break;

  case 49: // expression_list: expression_list "," exp
#line 227 "parser.yy"
                              { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 1879 "parser.cc"
    break;

  case 50: // exp: "number"
#line 230 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Number((float) yystack_[0].value.as < float > ()); }
#line 1885 "parser.cc"
    break;

  case 51: // exp: "note"
#line 231 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Note(yystack_[0].value.as < std::string > ()); }
#line 1891 "parser.cc"
    break;

  case 52: // exp: "-" "number"
#line 232 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.Number(-1 * (float) yystack_[0].value.as < float > ());}
#line 1897 "parser.cc"
    break;

  case 53: // exp: "not" exp
#line 233 "parser.yy"
                      { yylhs.value.as < Expression > () = drv.factory.Not(yystack_[0].value.as < Expression > ());}
#line 1903 "parser.cc"
    break;

  case 54: // exp: "in_port"
#line 234 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1909 "parser.cc"
    break;

  case 55: // exp: "out_port"
#line 235 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1915 "parser.cc"
    break;

  case 56: // exp: "identifier"
#line 236 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1921 "parser.cc"
    break;

  case 57: // exp: "identifier" "[" exp "]"
#line 237 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.ArrayVariable(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv); }
#line 1927 "parser.cc"
    break;

  case 58: // exp: exp "+" exp
#line 238 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1933 "parser.cc"
    break;

  case 59: // exp: exp "-" exp
#line 239 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1939 "parser.cc"
    break;

  case 60: // exp: exp "*" exp
#line 240 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1945 "parser.cc"
    break;

  case 61: // exp: exp "/" exp
#line 241 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1951 "parser.cc"
    break;

  case 62: // exp: exp "comparison" exp
#line 242 "parser.yy"
                       { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1957 "parser.cc"
    break;

  case 63: // exp: exp "and" exp
#line 243 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1963 "parser.cc"
    break;

  case 64: // exp: exp "or" exp
#line 244 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1969 "parser.cc"
    break;

  case 65: // exp: "zeroargfunc" "(" ")"
#line 245 "parser.yy"
                        { yylhs.value.as < Expression > () = drv.factory.ZeroArgFunc(yystack_[2].value.as < std::string > ()); }
#line 1975 "parser.cc"
    break;

  case 66: // exp: "connected" "(" "in_port" ")"
#line 246 "parser.yy"
                                {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 1981 "parser.cc"
    break;

  case 67: // exp: "connected" "(" "out_port" ")"
#line 247 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 1987 "parser.cc"
    break;

  case 68: // exp: "trigger" "(" "in_port" ")"
#line 248 "parser.yy"
                                {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 1993 "parser.cc"
    break;

  case 69: // exp: "oneargfunc" "(" exp ")"
#line 249 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.OneArgFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > ()); }
#line 1999 "parser.cc"
    break;

  case 70: // exp: "twoargfunc" "(" exp "," exp ")"
#line 250 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.TwoArgFunc(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > ()); }
#line 2005 "parser.cc"
    break;

  case 71: // exp: "(" exp ")"
#line 251 "parser.yy"
                { yylhs.value.as < Expression > () = yystack_[1].value.as < Expression > (); }
#line 2011 "parser.cc"
    break;

  case 72: // string_list: string_exp
#line 255 "parser.yy"
                             { ExpressionList foo = ExpressionList::StringList();
                               yylhs.value.as < ExpressionList > () = foo.add(yystack_[0].value.as < Expression > ()); }
#line 2018 "parser.cc"
    break;

  case 73: // string_list: exp
#line 257 "parser.yy"
                             { ExpressionList foo = ExpressionList::StringList();
                               yylhs.value.as < ExpressionList > () = foo.add(yystack_[0].value.as < Expression > ()); }
#line 2025 "parser.cc"
    break;

  case 74: // string_list: string_list "," string_exp
#line 259 "parser.yy"
                             { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 2031 "parser.cc"
    break;

  case 75: // string_list: string_list "," exp
#line 260 "parser.yy"
                             { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 2037 "parser.cc"
    break;

  case 76: // string_exp: "quoted_string"
#line 263 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.Quoted(yystack_[0].value.as < std::string > ()); }
#line 2043 "parser.cc"
    break;

  case 77: // string_exp: "identifier" "$"
#line 264 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.StringVariable(yystack_[1].value.as < std::string > (), &drv); }
#line 2049 "parser.cc"
    break;

  case 78: // string_exp: "identifier" "$" "[" exp "]"
#line 265 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.StringArrayVariable(yystack_[4].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2055 "parser.cc"
    break;

  case 79: // string_exp: "debug" "(" "identifier" ")"
#line 266 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.DebugId(yystack_[1].value.as < std::string > (), &drv); }
#line 2061 "parser.cc"
    break;

  case 80: // string_exp: "debug" "(" "identifier" "$" ")"
#line 267 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.DebugIdString(yystack_[2].value.as < std::string > (), &drv); }
#line 2067 "parser.cc"
    break;

  case 81: // string_exp: "debug" "(" "identifier" "[" "]" "," exp "," exp ")"
#line 268 "parser.yy"
                                                       { yylhs.value.as < Expression > () = drv.factory.DebugId(yystack_[7].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2073 "parser.cc"
    break;

  case 82: // string_exp: "debug" "(" "identifier" "$" "[" "]" "," exp "," exp ")"
#line 269 "parser.yy"
                                                           { yylhs.value.as < Expression > () = drv.factory.DebugIdString(yystack_[8].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2079 "parser.cc"
    break;


#line 2083 "parser.cc"

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
  "=", "ceiling", "clear", "connected", "continue", "debug", "else",
  "elseif", "end", "exit", "floor", "for", "if", "log2", "loge", "log10",
  "max", "min", "next", "not", "or", "pow", "print", "reset",
  "sample_rate", "sign", "sin", "start", "step", "then", "time",
  "time_millis", "to", "trigger", "wait", "when", "-", "+", "*", "/", "(",
  ")", "{", "}", "[", "]", ",", "$", "identifier", "quoted_string",
  "number", "note", "in_port", "out_port", "zeroargfunc", "oneargfunc",
  "twoargfunc", "comparison", "NEG", "$accept", "program", "blocks",
  "block", "main_block", "zero_or_more_statements",
  "one_or_more_statements", "statement", "array_assignment", "assignment",
  "clear_statement", "continue_statement", "exit_statement",
  "for_statement", "elseif_group", "elseif_clause", "if_statement",
  "print_statement", "reset_statement", "wait_statement",
  "expression_list", "exp", "string_list", "string_exp", YY_NULLPTR
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


  const signed char Parser::yypact_ninf_ = -84;

  const signed char Parser::yytable_ninf_ = -1;

  const short
  Parser::yypact_[] =
  {
     312,   561,    20,    11,    12,   -21,   412,   -40,   -84,   412,
     412,     3,    19,    21,    47,     9,   -84,   -84,   561,   -84,
     -84,   -84,   -84,   -84,   -84,   -84,   -84,   -84,   -84,   -84,
     447,   -84,   -84,   -84,   -84,   -84,     4,    24,    -5,   412,
      38,    41,   412,    52,   -84,   -84,   -84,   -84,    57,    59,
      67,    66,    65,   269,    78,   412,   412,    14,   412,   412,
     -84,   -84,   -84,   -84,   121,   124,   412,    31,   -84,    69,
     -84,     0,   412,    84,   412,   412,   412,   412,   -84,   412,
     412,   412,   412,   412,    81,   471,   269,   109,   350,   412,
     269,   269,   -84,   123,    93,   101,   111,   -84,   145,   -84,
     150,   157,   300,    35,   561,    55,    55,   -84,   -84,   102,
     350,   115,   153,   117,   -29,   -84,   269,   -84,   179,   412,
     499,   -84,   -84,   -84,   -84,   -84,   412,   -84,   103,   269,
     -35,   -84,   -84,   388,   107,   119,   164,   269,   -84,   193,
     -84,   412,   156,   -84,   -84,   350,   412,   269,   -31,   412,
     326,   516,   -84,   533,   206,   -84,   269,   -84,   -18,   269,
     -84,   128,   -11,   201,   350,   269,   -84,   -84,   162,   -84,
     -84,   412,   130,   -84,   134,   -84,    42,   -84,   561,   269,
     412,   139,   -84,   228,   412,   412,   234,   256,   412,   -84,
     262,   -84
  };

  const signed char
  Parser::yydefact_[] =
  {
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
       0,     0,     0,     0,     0,     0,     4,     3,     8,    11,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
       0,    33,    35,    34,    37,    36,     0,     0,     0,     0,
       0,     0,     0,    56,    50,    51,    54,    55,     0,     0,
       0,     0,     0,    47,     0,     0,     0,     0,     0,     0,
       1,     2,     5,    12,     0,     0,     0,     0,    53,     0,
      52,     0,     0,     0,     0,     0,     0,     0,     9,     0,
       0,     0,     0,     0,     0,     0,    28,     0,     0,     0,
      29,    30,     6,     9,     0,     0,     0,    71,     0,    65,
       0,     0,    63,    64,    40,    59,    58,    60,    61,    62,
       0,     0,     0,     0,    56,    76,    31,    32,     0,     0,
       0,    66,    67,    68,    57,    69,     0,    10,     0,    73,
       0,    72,     7,     0,     0,    77,     0,     9,    38,     0,
       9,     0,     0,    41,    45,     0,     0,    23,     0,     0,
       0,     0,    70,     0,     0,    43,    75,    74,     0,    48,
      79,     0,     0,     0,     0,    25,    26,    39,     0,     9,
      24,     0,     0,    80,     0,    78,     0,    44,    42,    49,
       0,     0,    27,     0,     0,     0,     0,     0,     0,    81,
       0,    82
  };

  const short
  Parser::yypgoto_[] =
  {
     -84,   -84,   -84,   161,   -84,   -81,     7,     1,   -84,   199,
     -84,   -84,   -84,   -84,   -84,   -84,   -84,   -84,   -84,   -84,
     -84,    -6,    44,   -83
  };

  const unsigned char
  Parser::yydefgoto_[] =
  {
       0,    14,    15,    16,    17,   104,    18,   127,    20,    21,
      22,    23,    24,    25,   128,   143,    26,    27,    28,    29,
     158,   129,   130,   131
  };

  const unsigned char
  Parser::yytable_[] =
  {
      51,    19,    19,    53,    54,   117,    76,    52,    30,    61,
      55,    55,   120,   144,     1,    32,    34,   160,   145,    63,
     161,    88,    72,   162,    31,   135,    58,    77,    59,    33,
      35,    63,   170,    68,    36,   171,    71,   173,    12,    13,
     174,    76,    67,    79,    80,    81,    82,    60,    97,    86,
      87,    10,    90,    91,    56,    19,   151,    57,    65,   153,
      93,    85,   157,    66,    83,    89,    98,   166,   100,   101,
     102,   103,    76,   105,   106,   107,   108,   109,    79,    80,
      81,    82,   116,   118,    76,    69,    63,     2,   178,     3,
      94,    95,   182,    77,     4,   145,     5,     6,    70,    83,
      81,    82,    78,    72,    73,    77,    74,     7,     8,    79,
      80,    81,    82,   137,    75,    76,   140,   141,   142,     9,
     139,    79,    80,    81,    82,    84,    92,   147,    96,    76,
      83,    88,    99,    11,   110,   154,    77,    12,    13,   156,
     159,   121,    83,   163,   165,    79,    80,    81,    82,   122,
      77,    76,    79,    80,    81,    82,    76,   132,   119,   123,
     133,   112,   148,    76,   134,   179,    79,    80,    81,    82,
     149,   150,    77,    83,   183,   155,    62,    77,   186,   187,
     172,   177,   190,   180,    77,    76,   181,    83,    79,    80,
      81,    82,   184,    79,    80,    81,    82,   124,   125,    76,
      79,    80,    81,    82,    37,     0,    77,    76,   176,    83,
     126,     0,    76,     0,    83,     0,     0,     0,     0,     0,
      77,    83,    79,    80,    81,    82,     0,     0,    77,     0,
       0,   136,     0,    77,    76,     0,    79,    80,    81,    82,
      76,   152,   169,    83,    79,    80,    81,    82,     0,    79,
      80,    81,    82,   175,     0,    77,     0,    83,     0,     0,
       0,    77,    76,     0,     0,    83,     0,     0,    76,     0,
      83,    79,    80,    81,    82,    76,     0,    79,    80,    81,
      82,   185,     0,    77,     0,     0,     0,   188,     0,    77,
       0,     0,    83,     0,     0,     0,    77,     0,    83,    79,
      80,    81,    82,     0,   189,    79,    80,    81,    82,     0,
     191,     0,    79,    80,    81,    82,     0,     1,     0,     0,
      83,     2,     0,     3,     0,     0,    83,     0,     4,     0,
       5,     6,     0,    83,     0,     0,    38,     0,   113,     0,
       0,     7,     8,    79,    80,    81,    82,     0,     0,     0,
       0,     0,    39,     9,    10,     0,     0,     0,     0,     0,
      38,     0,   113,     0,    83,     0,    40,    11,     0,    41,
       0,    12,    13,    42,     0,   164,    39,     0,     0,     0,
       0,   114,   115,    44,    45,    46,    47,    48,    49,    50,
      40,     0,     0,    41,     0,     0,     0,    42,    38,     0,
       0,     0,     0,     0,     0,   114,   115,    44,    45,    46,
      47,    48,    49,    50,    39,     0,     0,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,    40,     0,
       0,    41,     0,     0,     0,    42,     0,   146,    39,     0,
       0,     0,     0,    43,     0,    44,    45,    46,    47,    48,
      49,    50,    40,     0,     0,    41,     2,     0,     3,    42,
       0,     0,    64,     4,     0,     5,     6,    43,     0,    44,
      45,    46,    47,    48,    49,    50,     7,     8,     0,     0,
       2,     0,     3,     0,     0,     0,   111,     4,     9,     5,
       6,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       7,     8,    11,     0,     0,     0,    12,    13,     2,     0,
       3,     0,     9,     0,     0,     4,     0,     5,     6,     0,
       0,     0,     0,     0,   138,     2,    11,     3,     7,     8,
      12,    13,     4,     0,     5,     6,     0,     0,     0,     0,
       9,   167,     2,     0,     3,     7,     8,     0,   168,     4,
       0,     5,     6,     0,    11,     0,     0,     9,    12,    13,
       0,     0,     7,     8,     0,     0,     0,     0,     0,     0,
       2,    11,     3,     0,     9,    12,    13,     4,     0,     5,
       6,     0,     0,     0,     0,     0,     0,     0,    11,     0,
       7,     8,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     9,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,     0,     0,     0,
      12,    13
  };

  const short
  Parser::yycheck_[] =
  {
       6,     0,     1,     9,    10,    88,     6,    47,     1,     0,
       7,     7,    93,    48,     5,     4,     4,    48,    53,    18,
      51,     7,    51,    54,     4,    54,     7,    27,     7,    18,
      18,    30,    50,    39,    55,    53,    42,    48,    59,    60,
      51,     6,    47,    43,    44,    45,    46,     0,    48,    55,
      56,    42,    58,    59,    51,    54,   137,    54,    54,   140,
      66,    54,   145,    39,    64,    51,    72,   150,    74,    75,
      76,    77,     6,    79,    80,    81,    82,    83,    43,    44,
      45,    46,    88,    89,     6,    47,    85,     9,   169,    11,
      59,    60,    50,    27,    16,    53,    18,    19,    57,    64,
      45,    46,    36,    51,    47,    27,    47,    29,    30,    43,
      44,    45,    46,   119,    47,     6,    13,    14,    15,    41,
     126,    43,    44,    45,    46,    60,     5,   133,    59,     6,
      64,     7,    48,    55,    53,   141,    27,    59,    60,   145,
     146,    48,    64,   149,   150,    43,    44,    45,    46,    48,
      27,     6,    43,    44,    45,    46,     6,    42,    35,    48,
       7,    52,    55,     6,    47,   171,    43,    44,    45,    46,
      51,     7,    27,    64,   180,    19,    15,    27,   184,   185,
      52,    19,   188,    53,    27,     6,    52,    64,    43,    44,
      45,    46,    53,    43,    44,    45,    46,    52,    48,     6,
      43,    44,    45,    46,     5,    -1,    27,     6,   164,    64,
      53,    -1,     6,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      27,    64,    43,    44,    45,    46,    -1,    -1,    27,    -1,
      -1,    52,    -1,    27,     6,    -1,    43,    44,    45,    46,
       6,    48,    36,    64,    43,    44,    45,    46,    -1,    43,
      44,    45,    46,    52,    -1,    27,    -1,    64,    -1,    -1,
      -1,    27,     6,    -1,    -1,    64,    -1,    -1,     6,    -1,
      64,    43,    44,    45,    46,     6,    -1,    43,    44,    45,
      46,    53,    -1,    27,    -1,    -1,    -1,    53,    -1,    27,
      -1,    -1,    64,    -1,    -1,    -1,    27,    -1,    64,    43,
      44,    45,    46,    -1,    48,    43,    44,    45,    46,    -1,
      48,    -1,    43,    44,    45,    46,    -1,     5,    -1,    -1,
      64,     9,    -1,    11,    -1,    -1,    64,    -1,    16,    -1,
      18,    19,    -1,    64,    -1,    -1,    10,    -1,    12,    -1,
      -1,    29,    30,    43,    44,    45,    46,    -1,    -1,    -1,
      -1,    -1,    26,    41,    42,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    12,    -1,    64,    -1,    40,    55,    -1,    43,
      -1,    59,    60,    47,    -1,    49,    26,    -1,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      40,    -1,    -1,    43,    -1,    -1,    -1,    47,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    40,    -1,
      -1,    43,    -1,    -1,    -1,    47,    -1,    49,    26,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    59,    60,    61,
      62,    63,    40,    -1,    -1,    43,     9,    -1,    11,    47,
      -1,    -1,    15,    16,    -1,    18,    19,    55,    -1,    57,
      58,    59,    60,    61,    62,    63,    29,    30,    -1,    -1,
       9,    -1,    11,    -1,    -1,    -1,    15,    16,    41,    18,
      19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    30,    55,    -1,    -1,    -1,    59,    60,     9,    -1,
      11,    -1,    41,    -1,    -1,    16,    -1,    18,    19,    -1,
      -1,    -1,    -1,    -1,    25,     9,    55,    11,    29,    30,
      59,    60,    16,    -1,    18,    19,    -1,    -1,    -1,    -1,
      41,    25,     9,    -1,    11,    29,    30,    -1,    15,    16,
      -1,    18,    19,    -1,    55,    -1,    -1,    41,    59,    60,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    -1,    -1,
       9,    55,    11,    -1,    41,    59,    60,    16,    -1,    18,
      19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      29,    30,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      59,    60
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     5,     9,    11,    16,    18,    19,    29,    30,    41,
      42,    55,    59,    60,    67,    68,    69,    70,    72,    73,
      74,    75,    76,    77,    78,    79,    82,    83,    84,    85,
      72,     4,     4,    18,     4,    18,    55,    75,    10,    26,
      40,    43,    47,    55,    57,    58,    59,    60,    61,    62,
      63,    87,    47,    87,    87,     7,    51,    54,     7,     7,
       0,     0,    69,    73,    15,    54,    39,    47,    87,    47,
      57,    87,    51,    47,    47,    47,     6,    27,    36,    43,
      44,    45,    46,    64,    60,    72,    87,    87,     7,    51,
      87,    87,     5,    87,    59,    60,    59,    48,    87,    48,
      87,    87,    87,    87,    71,    87,    87,    87,    87,    87,
      53,    15,    52,    12,    55,    56,    87,    89,    87,    35,
      71,    48,    48,    48,    52,    48,    53,    73,    80,    87,
      88,    89,    42,     7,    47,    54,    52,    87,    25,    87,
      13,    14,    15,    81,    48,    53,    49,    87,    55,    51,
       7,    71,    48,    71,    87,    19,    87,    89,    86,    87,
      48,    51,    54,    87,    49,    87,    89,    25,    15,    36,
      50,    53,    52,    48,    51,    52,    88,    19,    71,    87,
      53,    52,    50,    87,    53,    53,    87,    87,    53,    48,
      87,    48
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    66,    67,    68,    68,    68,    69,    69,    70,    71,
      71,    72,    72,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    74,    74,    74,    74,    74,    75,    75,
      75,    75,    75,    76,    77,    77,    78,    78,    79,    79,
      80,    80,    81,    82,    82,    83,    84,    85,    86,    86,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    88,    88,    88,    88,    89,    89,    89,    89,
      89,    89,    89
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     2,     1,     1,     2,     4,     5,     1,     0,
       2,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     6,     8,     7,     7,     9,     3,     3,
       3,     4,     4,     2,     2,     2,     2,     2,     6,     8,
       0,     2,     4,     7,     9,     6,     1,     2,     1,     3,
       1,     1,     2,     2,     1,     1,     1,     4,     3,     3,
       3,     3,     3,     3,     3,     3,     4,     4,     4,     4,
       6,     3,     1,     1,     3,     3,     1,     2,     5,     4,
       5,    10,    11
  };




#if YYDEBUG
  const short
  Parser::yyrline_[] =
  {
       0,   135,   135,   138,   139,   140,   143,   144,   147,   150,
     151,   154,   155,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   170,   171,   172,   173,   174,   177,   178,
     179,   180,   181,   184,   187,   188,   191,   192,   195,   196,
     199,   200,   203,   206,   207,   210,   213,   216,   226,   227,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   255,   257,   259,   260,   263,   264,   265,   266,
     267,   268,   269
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
#line 2856 "parser.cc"

#line 271 "parser.yy"


void
yy::Parser::error (const location_type& l, const std::string& m)
{
  drv.errors.push_back(Error(l.begin.line, l.begin.column, m));
}
