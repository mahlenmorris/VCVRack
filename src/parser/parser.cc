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
      case symbol_kind::S_procedure_call: // procedure_call
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
      case symbol_kind::S_CHANNELS: // "channels"
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
      case symbol_kind::S_SET_CHANNELS: // "set_channels"
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
      case symbol_kind::S_QUESTION: // "?"
      case symbol_kind::S_COLON: // ":"
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
      case symbol_kind::S_procedure_call: // procedure_call
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
      case symbol_kind::S_CHANNELS: // "channels"
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
      case symbol_kind::S_SET_CHANNELS: // "set_channels"
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
      case symbol_kind::S_QUESTION: // "?"
      case symbol_kind::S_COLON: // ":"
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
      case symbol_kind::S_procedure_call: // procedure_call
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
      case symbol_kind::S_CHANNELS: // "channels"
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
      case symbol_kind::S_SET_CHANNELS: // "set_channels"
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
      case symbol_kind::S_QUESTION: // "?"
      case symbol_kind::S_COLON: // ":"
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
      case symbol_kind::S_procedure_call: // procedure_call
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
      case symbol_kind::S_CHANNELS: // "channels"
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
      case symbol_kind::S_SET_CHANNELS: // "set_channels"
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
      case symbol_kind::S_QUESTION: // "?"
      case symbol_kind::S_COLON: // ":"
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
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 744 "parser.cc"
        break;

      case symbol_kind::S_ALL: // "all"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 750 "parser.cc"
        break;

      case symbol_kind::S_ALSO: // "also"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 756 "parser.cc"
        break;

      case symbol_kind::S_AND: // "and"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 762 "parser.cc"
        break;

      case symbol_kind::S_ASSIGN: // "="
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 768 "parser.cc"
        break;

      case symbol_kind::S_CEILING: // "ceiling"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 774 "parser.cc"
        break;

      case symbol_kind::S_CHANNELS: // "channels"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 780 "parser.cc"
        break;

      case symbol_kind::S_CLEAR: // "clear"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 786 "parser.cc"
        break;

      case symbol_kind::S_CONNECTED: // "connected"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 792 "parser.cc"
        break;

      case symbol_kind::S_CONTINUE: // "continue"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 798 "parser.cc"
        break;

      case symbol_kind::S_DEBUG: // "debug"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 804 "parser.cc"
        break;

      case symbol_kind::S_ELSE: // "else"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 810 "parser.cc"
        break;

      case symbol_kind::S_ELSEIF: // "elseif"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 816 "parser.cc"
        break;

      case symbol_kind::S_END: // "end"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 822 "parser.cc"
        break;

      case symbol_kind::S_EXIT: // "exit"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 828 "parser.cc"
        break;

      case symbol_kind::S_FLOOR: // "floor"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 834 "parser.cc"
        break;

      case symbol_kind::S_FOR: // "for"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 840 "parser.cc"
        break;

      case symbol_kind::S_IF: // "if"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 846 "parser.cc"
        break;

      case symbol_kind::S_LOG2: // "log2"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 852 "parser.cc"
        break;

      case symbol_kind::S_LOGE: // "loge"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 858 "parser.cc"
        break;

      case symbol_kind::S_LOG10: // "log10"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 864 "parser.cc"
        break;

      case symbol_kind::S_MAX: // "max"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 870 "parser.cc"
        break;

      case symbol_kind::S_MIN: // "min"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 876 "parser.cc"
        break;

      case symbol_kind::S_NEXT: // "next"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 882 "parser.cc"
        break;

      case symbol_kind::S_NOT: // "not"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 888 "parser.cc"
        break;

      case symbol_kind::S_OR: // "or"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 894 "parser.cc"
        break;

      case symbol_kind::S_POW: // "pow"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 900 "parser.cc"
        break;

      case symbol_kind::S_PRINT: // "print"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 906 "parser.cc"
        break;

      case symbol_kind::S_RESET: // "reset"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 912 "parser.cc"
        break;

      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 918 "parser.cc"
        break;

      case symbol_kind::S_SET_CHANNELS: // "set_channels"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 924 "parser.cc"
        break;

      case symbol_kind::S_SIGN: // "sign"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 930 "parser.cc"
        break;

      case symbol_kind::S_SIN: // "sin"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 936 "parser.cc"
        break;

      case symbol_kind::S_START: // "start"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 942 "parser.cc"
        break;

      case symbol_kind::S_STEP: // "step"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 948 "parser.cc"
        break;

      case symbol_kind::S_THEN: // "then"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 954 "parser.cc"
        break;

      case symbol_kind::S_TIME: // "time"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 960 "parser.cc"
        break;

      case symbol_kind::S_TIME_MILLIS: // "time_millis"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 966 "parser.cc"
        break;

      case symbol_kind::S_TO: // "to"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 972 "parser.cc"
        break;

      case symbol_kind::S_TRIGGER: // "trigger"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 978 "parser.cc"
        break;

      case symbol_kind::S_WAIT: // "wait"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 984 "parser.cc"
        break;

      case symbol_kind::S_WHEN: // "when"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 990 "parser.cc"
        break;

      case symbol_kind::S_MINUS: // "-"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 996 "parser.cc"
        break;

      case symbol_kind::S_PLUS: // "+"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1002 "parser.cc"
        break;

      case symbol_kind::S_STAR: // "*"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1008 "parser.cc"
        break;

      case symbol_kind::S_SLASH: // "/"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1014 "parser.cc"
        break;

      case symbol_kind::S_LPAREN: // "("
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1020 "parser.cc"
        break;

      case symbol_kind::S_RPAREN: // ")"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1026 "parser.cc"
        break;

      case symbol_kind::S_LBRACE: // "{"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1032 "parser.cc"
        break;

      case symbol_kind::S_RBRACE: // "}"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1038 "parser.cc"
        break;

      case symbol_kind::S_LBRACKET: // "["
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1044 "parser.cc"
        break;

      case symbol_kind::S_RBRACKET: // "]"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1050 "parser.cc"
        break;

      case symbol_kind::S_COMMA: // ","
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1056 "parser.cc"
        break;

      case symbol_kind::S_DOLLAR: // "$"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1062 "parser.cc"
        break;

      case symbol_kind::S_QUESTION: // "?"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1068 "parser.cc"
        break;

      case symbol_kind::S_COLON: // ":"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1074 "parser.cc"
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1080 "parser.cc"
        break;

      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1086 "parser.cc"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < float > (); }
#line 1092 "parser.cc"
        break;

      case symbol_kind::S_NOTE: // "note"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1098 "parser.cc"
        break;

      case symbol_kind::S_IN_PORT: // "in_port"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1104 "parser.cc"
        break;

      case symbol_kind::S_OUT_PORT: // "out_port"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1110 "parser.cc"
        break;

      case symbol_kind::S_ZEROARGFUNC: // "zeroargfunc"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1116 "parser.cc"
        break;

      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1122 "parser.cc"
        break;

      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1128 "parser.cc"
        break;

      case symbol_kind::S_COMPARISON: // "comparison"
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1134 "parser.cc"
        break;

      case symbol_kind::S_blocks: // blocks
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Blocks > (); }
#line 1140 "parser.cc"
        break;

      case symbol_kind::S_block: // block
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Block > (); }
#line 1146 "parser.cc"
        break;

      case symbol_kind::S_main_block: // main_block
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Block > (); }
#line 1152 "parser.cc"
        break;

      case symbol_kind::S_zero_or_more_statements: // zero_or_more_statements
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1158 "parser.cc"
        break;

      case symbol_kind::S_one_or_more_statements: // one_or_more_statements
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1164 "parser.cc"
        break;

      case symbol_kind::S_statement: // statement
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1170 "parser.cc"
        break;

      case symbol_kind::S_array_assignment: // array_assignment
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1176 "parser.cc"
        break;

      case symbol_kind::S_assignment: // assignment
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1182 "parser.cc"
        break;

      case symbol_kind::S_clear_statement: // clear_statement
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1188 "parser.cc"
        break;

      case symbol_kind::S_continue_statement: // continue_statement
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1194 "parser.cc"
        break;

      case symbol_kind::S_exit_statement: // exit_statement
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1200 "parser.cc"
        break;

      case symbol_kind::S_for_statement: // for_statement
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1206 "parser.cc"
        break;

      case symbol_kind::S_elseif_group: // elseif_group
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1212 "parser.cc"
        break;

      case symbol_kind::S_elseif_clause: // elseif_clause
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1218 "parser.cc"
        break;

      case symbol_kind::S_if_statement: // if_statement
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1224 "parser.cc"
        break;

      case symbol_kind::S_procedure_call: // procedure_call
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1230 "parser.cc"
        break;

      case symbol_kind::S_reset_statement: // reset_statement
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1236 "parser.cc"
        break;

      case symbol_kind::S_wait_statement: // wait_statement
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1242 "parser.cc"
        break;

      case symbol_kind::S_expression_list: // expression_list
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < ExpressionList > (); }
#line 1248 "parser.cc"
        break;

      case symbol_kind::S_exp: // exp
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Expression > (); }
#line 1254 "parser.cc"
        break;

      case symbol_kind::S_string_list: // string_list
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < ExpressionList > (); }
#line 1260 "parser.cc"
        break;

      case symbol_kind::S_string_exp: // string_exp
#line 133 "parser.yy"
                 { yyo << yysym.value.template as < Expression > (); }
#line 1266 "parser.cc"
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
      case symbol_kind::S_procedure_call: // procedure_call
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
      case symbol_kind::S_CHANNELS: // "channels"
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
      case symbol_kind::S_SET_CHANNELS: // "set_channels"
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
      case symbol_kind::S_QUESTION: // "?"
      case symbol_kind::S_COLON: // ":"
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
#line 139 "parser.yy"
                                  { drv.blocks = yystack_[1].value.as < Blocks > ().block_list; }
#line 1641 "parser.cc"
    break;

  case 3: // blocks: main_block
#line 142 "parser.yy"
                                  { yylhs.value.as < Blocks > () = Blocks(yystack_[0].value.as < Block > ()); }
#line 1647 "parser.cc"
    break;

  case 4: // blocks: block
#line 143 "parser.yy"
                                  { yylhs.value.as < Blocks > () = Blocks(yystack_[0].value.as < Block > ()); }
#line 1653 "parser.cc"
    break;

  case 5: // blocks: blocks block
#line 144 "parser.yy"
                                  { yylhs.value.as < Blocks > () = yystack_[1].value.as < Blocks > ().Add(yystack_[0].value.as < Block > ()); }
#line 1659 "parser.cc"
    break;

  case 6: // block: "also" one_or_more_statements "end" "also"
#line 147 "parser.yy"
                                              { yylhs.value.as < Block > () = Block::MainBlock(yystack_[2].value.as < Statements > ()); }
#line 1665 "parser.cc"
    break;

  case 7: // block: "when" exp one_or_more_statements "end" "when"
#line 148 "parser.yy"
                                                 { yylhs.value.as < Block > () = Block::WhenExpBlock(yystack_[3].value.as < Expression > (), yystack_[2].value.as < Statements > ()); }
#line 1671 "parser.cc"
    break;

  case 8: // main_block: one_or_more_statements
#line 151 "parser.yy"
                                  { yylhs.value.as < Block > () = Block::MainBlock(yystack_[0].value.as < Statements > ()); }
#line 1677 "parser.cc"
    break;

  case 9: // zero_or_more_statements: %empty
#line 154 "parser.yy"
                                     {}
#line 1683 "parser.cc"
    break;

  case 10: // zero_or_more_statements: zero_or_more_statements statement
#line 155 "parser.yy"
                                     { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1689 "parser.cc"
    break;

  case 11: // one_or_more_statements: statement
#line 158 "parser.yy"
                                   { yylhs.value.as < Statements > ()  = Statements::FirstStatement(yystack_[0].value.as < Line > ()); }
#line 1695 "parser.cc"
    break;

  case 12: // one_or_more_statements: one_or_more_statements statement
#line 159 "parser.yy"
                                   { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1701 "parser.cc"
    break;

  case 13: // statement: array_assignment
#line 162 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1707 "parser.cc"
    break;

  case 14: // statement: assignment
#line 163 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1713 "parser.cc"
    break;

  case 15: // statement: clear_statement
#line 164 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1719 "parser.cc"
    break;

  case 16: // statement: continue_statement
#line 165 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1725 "parser.cc"
    break;

  case 17: // statement: exit_statement
#line 166 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1731 "parser.cc"
    break;

  case 18: // statement: for_statement
#line 167 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1737 "parser.cc"
    break;

  case 19: // statement: if_statement
#line 168 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1743 "parser.cc"
    break;

  case 20: // statement: procedure_call
#line 169 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1749 "parser.cc"
    break;

  case 21: // statement: reset_statement
#line 170 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1755 "parser.cc"
    break;

  case 22: // statement: wait_statement
#line 171 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1761 "parser.cc"
    break;

  case 23: // array_assignment: "identifier" "[" exp "]" "=" exp
#line 174 "parser.yy"
                                    { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1767 "parser.cc"
    break;

  case 24: // array_assignment: "identifier" "[" exp "]" "=" "{" expression_list "}"
#line 175 "parser.yy"
                                                        { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[7].value.as < std::string > (), yystack_[5].value.as < Expression > (), yystack_[1].value.as < ExpressionList > (), &drv); }
#line 1773 "parser.cc"
    break;

  case 25: // array_assignment: "out_port" "[" exp "]" "=" exp
#line 176 "parser.yy"
                                 { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1779 "parser.cc"
    break;

  case 26: // array_assignment: "identifier" "$" "[" exp "]" "=" exp
#line 177 "parser.yy"
                                        { yylhs.value.as < Line > () = Line::StringArrayAssignment(yystack_[6].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1785 "parser.cc"
    break;

  case 27: // array_assignment: "identifier" "$" "[" exp "]" "=" string_exp
#line 178 "parser.yy"
                                               { yylhs.value.as < Line > () = Line::StringArrayAssignment(yystack_[6].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1791 "parser.cc"
    break;

  case 28: // array_assignment: "identifier" "$" "[" exp "]" "=" "{" string_list "}"
#line 179 "parser.yy"
                                                        { yylhs.value.as < Line > () = Line::StringArrayAssignment(yystack_[8].value.as < std::string > (), yystack_[5].value.as < Expression > (), yystack_[1].value.as < ExpressionList > (), &drv); }
#line 1797 "parser.cc"
    break;

  case 29: // assignment: "identifier" "=" exp
#line 182 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1803 "parser.cc"
    break;

  case 30: // assignment: "in_port" "=" exp
#line 183 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1809 "parser.cc"
    break;

  case 31: // assignment: "out_port" "=" exp
#line 184 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1815 "parser.cc"
    break;

  case 32: // assignment: "identifier" "$" "=" exp
#line 185 "parser.yy"
                            { yylhs.value.as < Line > () = Line::StringAssignment(yystack_[3].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1821 "parser.cc"
    break;

  case 33: // assignment: "identifier" "$" "=" string_exp
#line 186 "parser.yy"
                                   { yylhs.value.as < Line > () = Line::StringAssignment(yystack_[3].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1827 "parser.cc"
    break;

  case 34: // clear_statement: "clear" "all"
#line 189 "parser.yy"
                        { yylhs.value.as < Line > () = Line::ClearAll(); }
#line 1833 "parser.cc"
    break;

  case 35: // continue_statement: "continue" "for"
#line 192 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1839 "parser.cc"
    break;

  case 36: // continue_statement: "continue" "all"
#line 193 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1845 "parser.cc"
    break;

  case 37: // exit_statement: "exit" "for"
#line 196 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1851 "parser.cc"
    break;

  case 38: // exit_statement: "exit" "all"
#line 197 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1857 "parser.cc"
    break;

  case 39: // for_statement: "for" assignment "to" exp zero_or_more_statements "next"
#line 200 "parser.yy"
                                                            { yylhs.value.as < Line > () = Line::ForNext(yystack_[4].value.as < Line > (), yystack_[2].value.as < Expression > (), drv.factory.Number(1.0), yystack_[1].value.as < Statements > (), &drv); }
#line 1863 "parser.cc"
    break;

  case 40: // for_statement: "for" assignment "to" exp "step" exp zero_or_more_statements "next"
#line 201 "parser.yy"
                                                                      { yylhs.value.as < Line > () = Line::ForNext(yystack_[6].value.as < Line > (), yystack_[4].value.as < Expression > (), yystack_[2].value.as < Expression > (), yystack_[1].value.as < Statements > (), &drv); }
#line 1869 "parser.cc"
    break;

  case 41: // elseif_group: %empty
#line 204 "parser.yy"
                                  {}
#line 1875 "parser.cc"
    break;

  case 42: // elseif_group: elseif_group elseif_clause
#line 205 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1881 "parser.cc"
    break;

  case 43: // elseif_clause: "elseif" exp "then" zero_or_more_statements
#line 208 "parser.yy"
                                               { yylhs.value.as < Line > () = Line::ElseIf(yystack_[2].value.as < Expression > (), yystack_[0].value.as < Statements > ()); }
#line 1887 "parser.cc"
    break;

  case 44: // if_statement: "if" exp "then" zero_or_more_statements elseif_group "end" "if"
#line 211 "parser.yy"
                                                                        { yylhs.value.as < Line > () = Line::IfThen(yystack_[5].value.as < Expression > (), yystack_[3].value.as < Statements > (), yystack_[2].value.as < Statements > ()); }
#line 1893 "parser.cc"
    break;

  case 45: // if_statement: "if" exp "then" zero_or_more_statements elseif_group "else" zero_or_more_statements "end" "if"
#line 212 "parser.yy"
                                                                                                  { yylhs.value.as < Line > () = Line::IfThenElse(yystack_[7].value.as < Expression > (), yystack_[5].value.as < Statements > (), yystack_[2].value.as < Statements > (), yystack_[4].value.as < Statements > ()); }
#line 1899 "parser.cc"
    break;

  case 46: // procedure_call: "print" "(" "out_port" "," string_list ")"
#line 215 "parser.yy"
                                              {yylhs.value.as < Line > () = Line::Print(yystack_[3].value.as < std::string > (), yystack_[1].value.as < ExpressionList > (), &drv);}
#line 1905 "parser.cc"
    break;

  case 47: // procedure_call: "set_channels" "(" "out_port" "," exp ")"
#line 216 "parser.yy"
                                              {yylhs.value.as < Line > () = Line::SetChannels(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv);}
#line 1911 "parser.cc"
    break;

  case 48: // reset_statement: "reset"
#line 219 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Reset(); }
#line 1917 "parser.cc"
    break;

  case 49: // wait_statement: "wait" exp
#line 222 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Wait(yystack_[0].value.as < Expression > ()); }
#line 1923 "parser.cc"
    break;

  case 50: // expression_list: exp
#line 233 "parser.yy"
                              { yylhs.value.as < ExpressionList > () = ExpressionList(yystack_[0].value.as < Expression > ()); }
#line 1929 "parser.cc"
    break;

  case 51: // expression_list: expression_list "," exp
#line 234 "parser.yy"
                              { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 1935 "parser.cc"
    break;

  case 52: // exp: "number"
#line 237 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Number((float) yystack_[0].value.as < float > ()); }
#line 1941 "parser.cc"
    break;

  case 53: // exp: "note"
#line 238 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Note(yystack_[0].value.as < std::string > ()); }
#line 1947 "parser.cc"
    break;

  case 54: // exp: "-" "number"
#line 239 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.Number(-1 * (float) yystack_[0].value.as < float > ());}
#line 1953 "parser.cc"
    break;

  case 55: // exp: "not" exp
#line 240 "parser.yy"
                      { yylhs.value.as < Expression > () = drv.factory.Not(yystack_[0].value.as < Expression > ());}
#line 1959 "parser.cc"
    break;

  case 56: // exp: "in_port"
#line 241 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1965 "parser.cc"
    break;

  case 57: // exp: "in_port" "[" exp "]"
#line 242 "parser.yy"
                        { yylhs.value.as < Expression > () = drv.factory.ArrayVariable(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv); }
#line 1971 "parser.cc"
    break;

  case 58: // exp: "out_port"
#line 243 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1977 "parser.cc"
    break;

  case 59: // exp: "identifier"
#line 244 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1983 "parser.cc"
    break;

  case 60: // exp: "identifier" "[" exp "]"
#line 245 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.ArrayVariable(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv); }
#line 1989 "parser.cc"
    break;

  case 61: // exp: exp "+" exp
#line 246 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1995 "parser.cc"
    break;

  case 62: // exp: exp "-" exp
#line 247 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2001 "parser.cc"
    break;

  case 63: // exp: exp "*" exp
#line 248 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2007 "parser.cc"
    break;

  case 64: // exp: exp "/" exp
#line 249 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2013 "parser.cc"
    break;

  case 65: // exp: exp "comparison" exp
#line 250 "parser.yy"
                       { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2019 "parser.cc"
    break;

  case 66: // exp: exp "and" exp
#line 251 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2025 "parser.cc"
    break;

  case 67: // exp: exp "or" exp
#line 252 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2031 "parser.cc"
    break;

  case 68: // exp: "zeroargfunc" "(" ")"
#line 253 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.ZeroArgFunc(yystack_[2].value.as < std::string > ());}
#line 2037 "parser.cc"
    break;

  case 69: // exp: "channels" "(" "in_port" ")"
#line 254 "parser.yy"
                                {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 2043 "parser.cc"
    break;

  case 70: // exp: "connected" "(" "in_port" ")"
#line 255 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 2049 "parser.cc"
    break;

  case 71: // exp: "connected" "(" "out_port" ")"
#line 256 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 2055 "parser.cc"
    break;

  case 72: // exp: "trigger" "(" "in_port" ")"
#line 257 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 2061 "parser.cc"
    break;

  case 73: // exp: "oneargfunc" "(" exp ")"
#line 258 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.OneArgFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > ());}
#line 2067 "parser.cc"
    break;

  case 74: // exp: "twoargfunc" "(" exp "," exp ")"
#line 259 "parser.yy"
                                   {yylhs.value.as < Expression > () = drv.factory.TwoArgFunc(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > ());}
#line 2073 "parser.cc"
    break;

  case 75: // exp: exp "?" exp ":" exp
#line 260 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.TernaryFunc(yystack_[4].value.as < Expression > (), yystack_[2].value.as < Expression > (), yystack_[0].value.as < Expression > ());}
#line 2079 "parser.cc"
    break;

  case 76: // exp: "(" exp ")"
#line 261 "parser.yy"
                { yylhs.value.as < Expression > () = yystack_[1].value.as < Expression > (); }
#line 2085 "parser.cc"
    break;

  case 77: // string_list: string_exp
#line 265 "parser.yy"
                             { ExpressionList foo = ExpressionList::StringList();
                               yylhs.value.as < ExpressionList > () = foo.add(yystack_[0].value.as < Expression > ()); }
#line 2092 "parser.cc"
    break;

  case 78: // string_list: exp
#line 267 "parser.yy"
                             { ExpressionList foo = ExpressionList::StringList();
                               yylhs.value.as < ExpressionList > () = foo.add(yystack_[0].value.as < Expression > ()); }
#line 2099 "parser.cc"
    break;

  case 79: // string_list: string_list "," string_exp
#line 269 "parser.yy"
                             { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 2105 "parser.cc"
    break;

  case 80: // string_list: string_list "," exp
#line 270 "parser.yy"
                             { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 2111 "parser.cc"
    break;

  case 81: // string_exp: "quoted_string"
#line 273 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.Quoted(yystack_[0].value.as < std::string > ()); }
#line 2117 "parser.cc"
    break;

  case 82: // string_exp: "identifier" "$"
#line 274 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.StringVariable(yystack_[1].value.as < std::string > (), &drv); }
#line 2123 "parser.cc"
    break;

  case 83: // string_exp: "identifier" "$" "[" exp "]"
#line 275 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.StringArrayVariable(yystack_[4].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2129 "parser.cc"
    break;

  case 84: // string_exp: "debug" "(" "identifier" ")"
#line 276 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.DebugId(yystack_[1].value.as < std::string > (), &drv); }
#line 2135 "parser.cc"
    break;

  case 85: // string_exp: "debug" "(" "identifier" "$" ")"
#line 277 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.DebugIdString(yystack_[2].value.as < std::string > (), &drv); }
#line 2141 "parser.cc"
    break;

  case 86: // string_exp: "debug" "(" "identifier" "[" "]" "," exp "," exp ")"
#line 278 "parser.yy"
                                                       { yylhs.value.as < Expression > () = drv.factory.DebugId(yystack_[7].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2147 "parser.cc"
    break;

  case 87: // string_exp: "debug" "(" "identifier" "$" "[" "]" "," exp "," exp ")"
#line 279 "parser.yy"
                                                           { yylhs.value.as < Expression > () = drv.factory.DebugIdString(yystack_[8].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2153 "parser.cc"
    break;


#line 2157 "parser.cc"

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
  "=", "ceiling", "channels", "clear", "connected", "continue", "debug",
  "else", "elseif", "end", "exit", "floor", "for", "if", "log2", "loge",
  "log10", "max", "min", "next", "not", "or", "pow", "print", "reset",
  "sample_rate", "set_channels", "sign", "sin", "start", "step", "then",
  "time", "time_millis", "to", "trigger", "wait", "when", "-", "+", "*",
  "/", "(", ")", "{", "}", "[", "]", ",", "$", "?", ":", "identifier",
  "quoted_string", "number", "note", "in_port", "out_port", "zeroargfunc",
  "oneargfunc", "twoargfunc", "comparison", "NEG", "$accept", "program",
  "blocks", "block", "main_block", "zero_or_more_statements",
  "one_or_more_statements", "statement", "array_assignment", "assignment",
  "clear_statement", "continue_statement", "exit_statement",
  "for_statement", "elseif_group", "elseif_clause", "if_statement",
  "procedure_call", "reset_statement", "wait_statement", "expression_list",
  "exp", "string_list", "string_exp", YY_NULLPTR
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


  const signed char Parser::yypact_ninf_ = -97;

  const signed char Parser::yytable_ninf_ = -1;

  const short
  Parser::yypact_[] =
  {
     474,   757,     4,     6,    15,   -21,   590,   -32,   -97,   -16,
     590,   590,    14,    30,    16,    22,    11,   -97,   -97,   757,
     -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,
     -97,   628,   -97,   -97,   -97,   -97,   -97,    -4,    39,    25,
      64,    66,   590,    68,    18,   590,    35,   -97,   -97,    70,
     -97,    80,    82,    83,     3,    63,    74,   428,   161,   590,
     590,    21,   590,   590,   590,   -97,   -97,   -97,   -97,   134,
     136,   590,    81,    42,   -97,    84,   -97,    62,   590,   590,
      91,   590,   590,   590,   590,   -97,   590,   590,   590,   590,
     590,   590,    93,    94,   650,   428,    88,   523,   590,   428,
     428,   187,   -97,   211,   101,   102,   103,   104,   -97,   215,
     239,   -97,   264,   270,   -33,    56,   757,    78,    78,   -97,
     -97,   294,    52,   523,   590,   113,   151,   110,   -27,   -97,
     428,   -97,   300,   154,   590,   685,   -97,   -97,   -97,   -97,
     -97,   -97,   -97,   590,   -97,   106,   590,   428,   -10,   -97,
     325,   -97,   564,   105,   115,   156,   590,   428,   -97,   331,
     -97,   590,   149,   -97,   428,   -97,   523,   -97,   590,   428,
     -26,   590,   497,   428,   700,   -97,   722,   357,   -97,   428,
     -97,    -8,   428,   -97,   116,    36,   361,   523,   428,   -97,
     -97,   152,   -97,   -97,   590,   119,   -97,   121,   -97,    59,
     -97,   757,   428,   590,   122,   -97,   385,   590,   590,   391,
     415,   590,   -97,   421,   -97
  };

  const signed char
  Parser::yydefact_[] =
  {
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     3,     8,
      11,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,    34,    36,    35,    38,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    59,    52,    53,    56,
      58,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,     1,     2,     5,    12,     0,
       0,     0,     0,     0,    55,     0,    54,     0,     0,     0,
       0,     0,     0,     0,     0,     9,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    29,     0,     0,     0,    30,
      31,     0,     6,     9,     0,     0,     0,     0,    76,     0,
       0,    68,     0,     0,    66,    67,    41,    62,    61,    63,
      64,     0,    65,     0,     0,     0,     0,     0,    59,    81,
      32,    33,     0,     0,     0,     0,    69,    70,    71,    72,
      60,    57,    73,     0,    10,     0,     0,    78,     0,    77,
       0,     7,     0,     0,    82,     0,     0,     9,    39,     0,
       9,     0,     0,    42,    75,    46,     0,    47,     0,    23,
       0,     0,     0,    25,     0,    74,     0,     0,    44,    80,
      79,     0,    50,    84,     0,     0,     0,     0,    26,    27,
      40,     0,     9,    24,     0,     0,    85,     0,    83,     0,
      45,    43,    51,     0,     0,    28,     0,     0,     0,     0,
       0,     0,    86,     0,    87
  };

  const short
  Parser::yypgoto_[] =
  {
     -97,   -97,   -97,   160,   -97,   -96,     5,     1,   -97,   174,
     -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,
     -97,    -6,    -5,   -79
  };

  const unsigned char
  Parser::yydefgoto_[] =
  {
       0,    15,    16,    17,    18,   116,    19,   144,    21,    22,
      23,    24,    25,    26,   145,   163,    27,    28,    29,    30,
     181,   147,   148,   149
  };

  const unsigned char
  Parser::yytable_[] =
  {
      54,    20,    20,    59,    57,    58,    31,   135,    32,    83,
      33,    66,    86,    87,    88,    89,     1,    55,   131,    35,
      68,    59,    65,    63,   183,    34,    78,   184,    97,   154,
     185,    84,    68,    56,    36,    91,    74,    62,    37,    77,
     165,    85,    13,    38,   193,   166,    63,   194,    86,    87,
      88,    89,    70,    95,    96,    11,    99,   100,   101,    20,
      90,   174,    83,    94,   176,   103,    71,    60,    83,    64,
      61,    91,   109,   110,    98,   112,   113,   114,   115,    76,
     117,   118,   119,   120,   121,   122,   196,   180,    78,   197,
      84,   130,   132,   189,    83,    68,   201,    86,    87,    88,
      89,    86,    87,    88,    89,   105,   106,    86,    87,    88,
      89,   205,   108,    72,   166,    73,    84,    75,   150,    90,
     160,   161,   162,    79,    91,    88,    89,    92,   157,    80,
      91,    81,    82,    86,    87,    88,    89,   159,    93,   102,
     164,   111,   126,    97,   104,    90,   169,   107,   123,   124,
     173,   136,   137,   138,   139,   177,    91,   151,   152,   153,
     179,   156,   182,   172,   170,   186,   188,    83,   171,   178,
     195,     2,   200,     3,   203,   204,    67,   207,     4,    39,
       5,     6,   199,     0,     0,     0,     0,     0,   202,    84,
       0,     7,     8,    83,     9,     0,     0,   206,     0,     0,
       0,   209,   210,     0,    10,   213,    86,    87,    88,    89,
       0,     0,     0,     0,     0,    84,     0,    83,    90,     0,
      12,    83,     0,     0,    13,    14,     0,     0,     0,    91,
       0,     0,    86,    87,    88,    89,     0,     0,     0,    84,
       0,   133,     0,    84,    90,    83,     0,     0,   134,     0,
       0,     0,     0,     0,     0,    91,    86,    87,    88,    89,
      86,    87,    88,    89,     0,     0,     0,    84,    90,   140,
      83,     0,    90,     0,     0,     0,    83,     0,     0,    91,
       0,     0,     0,    91,    86,    87,    88,    89,     0,     0,
       0,     0,    84,   141,     0,     0,    90,     0,    84,     0,
      83,     0,     0,     0,     0,     0,    83,    91,     0,    86,
      87,    88,    89,     0,   142,    86,    87,    88,    89,     0,
       0,    90,    84,     0,     0,   143,     0,    90,    84,     0,
       0,    83,    91,     0,     0,     0,     0,    83,    91,    86,
      87,    88,    89,     0,     0,    86,    87,    88,    89,     0,
       0,    90,   146,    84,   155,     0,     0,    90,     0,    84,
       0,     0,    91,    83,     0,     0,     0,    83,    91,     0,
      86,    87,    88,    89,     0,   167,    86,    87,    88,    89,
       0,   175,    90,     0,     0,    84,     0,     0,    90,    84,
       0,    83,     0,    91,     0,   192,     0,    83,     0,    91,
       0,     0,    86,    87,    88,    89,    86,    87,    88,    89,
       0,     0,     0,    84,    90,   198,     0,     0,    90,    84,
       0,    83,     0,     0,     0,    91,     0,    83,     0,    91,
      86,    87,    88,    89,    83,     0,    86,    87,    88,    89,
     208,     0,    90,    84,     0,     0,   211,     0,    90,    84,
       0,     0,     0,    91,     0,     0,    84,     0,     0,    91,
      86,    87,    88,    89,     0,   212,    86,    87,    88,    89,
       0,   214,    90,    86,    87,    88,    89,     0,    90,     1,
       0,     0,     0,    91,     2,    90,     3,     0,     0,    91,
       0,     4,     0,     5,     6,     0,    91,     0,     0,     0,
       0,     0,     0,     0,     7,     8,    40,     9,    41,     0,
     127,     0,     0,     0,     0,     0,     0,    10,    11,     0,
       0,     0,     0,     0,    42,     0,     0,     0,     0,     0,
       0,     0,    40,    12,    41,     0,   127,    13,    14,    43,
       0,     0,    44,     0,     0,     0,    45,     0,   187,     0,
      42,     0,     0,     0,     0,     0,   128,   129,    47,    48,
      49,    50,    51,    52,    53,    43,     0,     0,    44,     0,
       0,     0,    45,    40,     0,    41,     0,     0,     0,     0,
       0,     0,   128,   129,    47,    48,    49,    50,    51,    52,
      53,    42,     0,     0,     0,     0,     0,     0,     0,    40,
       0,    41,     0,     0,     0,     0,    43,     0,     0,    44,
       0,     0,     0,    45,     0,   168,     0,    42,     0,     0,
       0,     0,     0,    46,     0,    47,    48,    49,    50,    51,
      52,    53,    43,     0,     0,    44,     0,     0,     2,    45,
       3,     0,     0,     0,    69,     4,     0,     5,     6,    46,
       0,    47,    48,    49,    50,    51,    52,    53,     7,     8,
       2,     9,     3,     0,     0,     0,   125,     4,     0,     5,
       6,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       7,     8,     0,     9,     0,     0,     0,    12,     0,     0,
       0,    13,    14,    10,     0,     2,     0,     3,     0,     0,
       0,     0,     4,     0,     5,     6,     0,     0,     0,    12,
       2,   158,     3,    13,    14,     7,     8,     4,     9,     5,
       6,     0,     0,     0,     0,     0,   190,     0,    10,     0,
       7,     8,     2,     9,     3,     0,     0,     0,   191,     4,
       0,     5,     6,    10,    12,     0,     0,     0,    13,    14,
       0,     0,     7,     8,     0,     9,     0,     0,     0,    12,
       0,     0,     0,    13,    14,    10,     0,     2,     0,     3,
       0,     0,     0,     0,     4,     0,     5,     6,     0,     0,
       0,    12,     0,     0,     0,    13,    14,     7,     8,     0,
       9,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,     0,     0,     0,
      13,    14
  };

  const short
  Parser::yycheck_[] =
  {
       6,     0,     1,     7,    10,    11,     1,   103,     4,     6,
       4,     0,    45,    46,    47,    48,     5,    49,    97,     4,
      19,     7,     0,     7,    50,    19,    53,    53,     7,    56,
      56,    28,    31,    49,    19,    68,    42,     7,    59,    45,
      50,    38,    63,    64,    52,    55,     7,    55,    45,    46,
      47,    48,    56,    59,    60,    44,    62,    63,    64,    58,
      57,   157,     6,    58,   160,    71,    41,    53,     6,    53,
      56,    68,    78,    79,    53,    81,    82,    83,    84,    61,
      86,    87,    88,    89,    90,    91,    50,   166,    53,    53,
      28,    97,    98,   172,     6,    94,   192,    45,    46,    47,
      48,    45,    46,    47,    48,    63,    64,    45,    46,    47,
      48,    52,    50,    49,    55,    49,    28,    49,   124,    57,
      14,    15,    16,    53,    68,    47,    48,    64,   134,    49,
      68,    49,    49,    45,    46,    47,    48,   143,    64,     5,
     146,    50,    54,     7,    63,    57,   152,    63,    55,    55,
     156,    50,    50,    50,    50,   161,    68,    44,     7,    49,
     166,     7,   168,     7,    59,   171,   172,     6,    53,    20,
      54,    10,    20,    12,    55,    54,    16,    55,    17,     5,
      19,    20,   187,    -1,    -1,    -1,    -1,    -1,   194,    28,
      -1,    30,    31,     6,    33,    -1,    -1,   203,    -1,    -1,
      -1,   207,   208,    -1,    43,   211,    45,    46,    47,    48,
      -1,    -1,    -1,    -1,    -1,    28,    -1,     6,    57,    -1,
      59,     6,    -1,    -1,    63,    64,    -1,    -1,    -1,    68,
      -1,    -1,    45,    46,    47,    48,    -1,    -1,    -1,    28,
      -1,    54,    -1,    28,    57,     6,    -1,    -1,    37,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    45,    46,    47,    48,
      45,    46,    47,    48,    -1,    -1,    -1,    28,    57,    54,
       6,    -1,    57,    -1,    -1,    -1,     6,    -1,    -1,    68,
      -1,    -1,    -1,    68,    45,    46,    47,    48,    -1,    -1,
      -1,    -1,    28,    54,    -1,    -1,    57,    -1,    28,    -1,
       6,    -1,    -1,    -1,    -1,    -1,     6,    68,    -1,    45,
      46,    47,    48,    -1,    50,    45,    46,    47,    48,    -1,
      -1,    57,    28,    -1,    -1,    55,    -1,    57,    28,    -1,
      -1,     6,    68,    -1,    -1,    -1,    -1,     6,    68,    45,
      46,    47,    48,    -1,    -1,    45,    46,    47,    48,    -1,
      -1,    57,    58,    28,    54,    -1,    -1,    57,    -1,    28,
      -1,    -1,    68,     6,    -1,    -1,    -1,     6,    68,    -1,
      45,    46,    47,    48,    -1,    50,    45,    46,    47,    48,
      -1,    50,    57,    -1,    -1,    28,    -1,    -1,    57,    28,
      -1,     6,    -1,    68,    -1,    38,    -1,     6,    -1,    68,
      -1,    -1,    45,    46,    47,    48,    45,    46,    47,    48,
      -1,    -1,    -1,    28,    57,    54,    -1,    -1,    57,    28,
      -1,     6,    -1,    -1,    -1,    68,    -1,     6,    -1,    68,
      45,    46,    47,    48,     6,    -1,    45,    46,    47,    48,
      55,    -1,    57,    28,    -1,    -1,    55,    -1,    57,    28,
      -1,    -1,    -1,    68,    -1,    -1,    28,    -1,    -1,    68,
      45,    46,    47,    48,    -1,    50,    45,    46,    47,    48,
      -1,    50,    57,    45,    46,    47,    48,    -1,    57,     5,
      -1,    -1,    -1,    68,    10,    57,    12,    -1,    -1,    68,
      -1,    17,    -1,    19,    20,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,     9,    33,    11,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    59,    11,    -1,    13,    63,    64,    42,
      -1,    -1,    45,    -1,    -1,    -1,    49,    -1,    51,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    42,    -1,    -1,    45,    -1,
      -1,    -1,    49,     9,    -1,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      -1,    11,    -1,    -1,    -1,    -1,    42,    -1,    -1,    45,
      -1,    -1,    -1,    49,    -1,    51,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    59,    -1,    61,    62,    63,    64,    65,
      66,    67,    42,    -1,    -1,    45,    -1,    -1,    10,    49,
      12,    -1,    -1,    -1,    16,    17,    -1,    19,    20,    59,
      -1,    61,    62,    63,    64,    65,    66,    67,    30,    31,
      10,    33,    12,    -1,    -1,    -1,    16,    17,    -1,    19,
      20,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    -1,    -1,    -1,    59,    -1,    -1,
      -1,    63,    64,    43,    -1,    10,    -1,    12,    -1,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    59,
      10,    26,    12,    63,    64,    30,    31,    17,    33,    19,
      20,    -1,    -1,    -1,    -1,    -1,    26,    -1,    43,    -1,
      30,    31,    10,    33,    12,    -1,    -1,    -1,    16,    17,
      -1,    19,    20,    43,    59,    -1,    -1,    -1,    63,    64,
      -1,    -1,    30,    31,    -1,    33,    -1,    -1,    -1,    59,
      -1,    -1,    -1,    63,    64,    43,    -1,    10,    -1,    12,
      -1,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    59,    -1,    -1,    -1,    63,    64,    30,    31,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,
      63,    64
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     5,    10,    12,    17,    19,    20,    30,    31,    33,
      43,    44,    59,    63,    64,    71,    72,    73,    74,    76,
      77,    78,    79,    80,    81,    82,    83,    86,    87,    88,
      89,    76,     4,     4,    19,     4,    19,    59,    64,    79,
       9,    11,    27,    42,    45,    49,    59,    61,    62,    63,
      64,    65,    66,    67,    91,    49,    49,    91,    91,     7,
      53,    56,     7,     7,    53,     0,     0,    73,    77,    16,
      56,    41,    49,    49,    91,    49,    61,    91,    53,    53,
      49,    49,    49,     6,    28,    38,    45,    46,    47,    48,
      57,    68,    64,    64,    76,    91,    91,     7,    53,    91,
      91,    91,     5,    91,    63,    63,    64,    63,    50,    91,
      91,    50,    91,    91,    91,    91,    75,    91,    91,    91,
      91,    91,    91,    55,    55,    16,    54,    13,    59,    60,
      91,    93,    91,    54,    37,    75,    50,    50,    50,    50,
      54,    54,    50,    55,    77,    84,    58,    91,    92,    93,
      91,    44,     7,    49,    56,    54,     7,    91,    26,    91,
      14,    15,    16,    85,    91,    50,    55,    50,    51,    91,
      59,    53,     7,    91,    75,    50,    75,    91,    20,    91,
      93,    90,    91,    50,    53,    56,    91,    51,    91,    93,
      26,    16,    38,    52,    55,    54,    50,    53,    54,    92,
      20,    75,    91,    55,    54,    52,    91,    55,    55,    91,
      91,    55,    50,    91,    50
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    70,    71,    72,    72,    72,    73,    73,    74,    75,
      75,    76,    76,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    78,    78,    78,    78,    78,    78,    79,
      79,    79,    79,    79,    80,    81,    81,    82,    82,    83,
      83,    84,    84,    85,    86,    86,    87,    87,    88,    89,
      90,    90,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    92,    92,    92,
      92,    93,    93,    93,    93,    93,    93,    93
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     2,     1,     1,     2,     4,     5,     1,     0,
       2,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     6,     8,     6,     7,     7,     9,     3,
       3,     3,     4,     4,     2,     2,     2,     2,     2,     6,
       8,     0,     2,     4,     7,     9,     6,     6,     1,     2,
       1,     3,     1,     1,     2,     2,     1,     4,     1,     1,
       4,     3,     3,     3,     3,     3,     3,     3,     3,     4,
       4,     4,     4,     4,     6,     5,     3,     1,     1,     3,
       3,     1,     2,     5,     4,     5,    10,    11
  };




#if YYDEBUG
  const short
  Parser::yyrline_[] =
  {
       0,   139,   139,   142,   143,   144,   147,   148,   151,   154,
     155,   158,   159,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   174,   175,   176,   177,   178,   179,   182,
     183,   184,   185,   186,   189,   192,   193,   196,   197,   200,
     201,   204,   205,   208,   211,   212,   215,   216,   219,   222,
     233,   234,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   265,   267,   269,
     270,   273,   274,   275,   276,   277,   278,   279
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
#line 2976 "parser.cc"

#line 281 "parser.yy"


void
yy::Parser::error (const location_type& l, const std::string& m)
{
  drv.errors.push_back(Error(l.begin.line, l.begin.column, m));
}
