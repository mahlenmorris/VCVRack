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
      case symbol_kind::S_while_statement: // while_statement
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
      case symbol_kind::S_NEXTHIGHCPU: // "nexthighcpu"
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
      case symbol_kind::S_WHILE: // "while"
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
      case symbol_kind::S_while_statement: // while_statement
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
      case symbol_kind::S_NEXTHIGHCPU: // "nexthighcpu"
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
      case symbol_kind::S_WHILE: // "while"
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
      case symbol_kind::S_while_statement: // while_statement
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
      case symbol_kind::S_NEXTHIGHCPU: // "nexthighcpu"
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
      case symbol_kind::S_WHILE: // "while"
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
      case symbol_kind::S_while_statement: // while_statement
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
      case symbol_kind::S_NEXTHIGHCPU: // "nexthighcpu"
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
      case symbol_kind::S_WHILE: // "while"
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
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 756 "parser.cc"
        break;

      case symbol_kind::S_ALL: // "all"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 762 "parser.cc"
        break;

      case symbol_kind::S_ALSO: // "also"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 768 "parser.cc"
        break;

      case symbol_kind::S_AND: // "and"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 774 "parser.cc"
        break;

      case symbol_kind::S_ASSIGN: // "="
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 780 "parser.cc"
        break;

      case symbol_kind::S_CEILING: // "ceiling"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 786 "parser.cc"
        break;

      case symbol_kind::S_CHANNELS: // "channels"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 792 "parser.cc"
        break;

      case symbol_kind::S_CLEAR: // "clear"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 798 "parser.cc"
        break;

      case symbol_kind::S_CONNECTED: // "connected"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 804 "parser.cc"
        break;

      case symbol_kind::S_CONTINUE: // "continue"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 810 "parser.cc"
        break;

      case symbol_kind::S_DEBUG: // "debug"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 816 "parser.cc"
        break;

      case symbol_kind::S_ELSE: // "else"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 822 "parser.cc"
        break;

      case symbol_kind::S_ELSEIF: // "elseif"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 828 "parser.cc"
        break;

      case symbol_kind::S_END: // "end"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 834 "parser.cc"
        break;

      case symbol_kind::S_EXIT: // "exit"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 840 "parser.cc"
        break;

      case symbol_kind::S_FLOOR: // "floor"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 846 "parser.cc"
        break;

      case symbol_kind::S_FOR: // "for"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 852 "parser.cc"
        break;

      case symbol_kind::S_IF: // "if"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 858 "parser.cc"
        break;

      case symbol_kind::S_LOG2: // "log2"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 864 "parser.cc"
        break;

      case symbol_kind::S_LOGE: // "loge"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 870 "parser.cc"
        break;

      case symbol_kind::S_LOG10: // "log10"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 876 "parser.cc"
        break;

      case symbol_kind::S_MAX: // "max"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 882 "parser.cc"
        break;

      case symbol_kind::S_MIN: // "min"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 888 "parser.cc"
        break;

      case symbol_kind::S_NEXT: // "next"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 894 "parser.cc"
        break;

      case symbol_kind::S_NEXTHIGHCPU: // "nexthighcpu"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 900 "parser.cc"
        break;

      case symbol_kind::S_NOT: // "not"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 906 "parser.cc"
        break;

      case symbol_kind::S_OR: // "or"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 912 "parser.cc"
        break;

      case symbol_kind::S_POW: // "pow"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 918 "parser.cc"
        break;

      case symbol_kind::S_PRINT: // "print"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 924 "parser.cc"
        break;

      case symbol_kind::S_RESET: // "reset"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 930 "parser.cc"
        break;

      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 936 "parser.cc"
        break;

      case symbol_kind::S_SET_CHANNELS: // "set_channels"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 942 "parser.cc"
        break;

      case symbol_kind::S_SIGN: // "sign"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 948 "parser.cc"
        break;

      case symbol_kind::S_SIN: // "sin"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 954 "parser.cc"
        break;

      case symbol_kind::S_START: // "start"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 960 "parser.cc"
        break;

      case symbol_kind::S_STEP: // "step"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 966 "parser.cc"
        break;

      case symbol_kind::S_THEN: // "then"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 972 "parser.cc"
        break;

      case symbol_kind::S_TIME: // "time"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 978 "parser.cc"
        break;

      case symbol_kind::S_TIME_MILLIS: // "time_millis"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 984 "parser.cc"
        break;

      case symbol_kind::S_TO: // "to"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 990 "parser.cc"
        break;

      case symbol_kind::S_TRIGGER: // "trigger"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 996 "parser.cc"
        break;

      case symbol_kind::S_WAIT: // "wait"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1002 "parser.cc"
        break;

      case symbol_kind::S_WHEN: // "when"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1008 "parser.cc"
        break;

      case symbol_kind::S_WHILE: // "while"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1014 "parser.cc"
        break;

      case symbol_kind::S_MINUS: // "-"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1020 "parser.cc"
        break;

      case symbol_kind::S_PLUS: // "+"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1026 "parser.cc"
        break;

      case symbol_kind::S_STAR: // "*"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1032 "parser.cc"
        break;

      case symbol_kind::S_SLASH: // "/"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1038 "parser.cc"
        break;

      case symbol_kind::S_LPAREN: // "("
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1044 "parser.cc"
        break;

      case symbol_kind::S_RPAREN: // ")"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1050 "parser.cc"
        break;

      case symbol_kind::S_LBRACE: // "{"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1056 "parser.cc"
        break;

      case symbol_kind::S_RBRACE: // "}"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1062 "parser.cc"
        break;

      case symbol_kind::S_LBRACKET: // "["
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1068 "parser.cc"
        break;

      case symbol_kind::S_RBRACKET: // "]"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1074 "parser.cc"
        break;

      case symbol_kind::S_COMMA: // ","
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1080 "parser.cc"
        break;

      case symbol_kind::S_DOLLAR: // "$"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1086 "parser.cc"
        break;

      case symbol_kind::S_QUESTION: // "?"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1092 "parser.cc"
        break;

      case symbol_kind::S_COLON: // ":"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1098 "parser.cc"
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1104 "parser.cc"
        break;

      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1110 "parser.cc"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < float > (); }
#line 1116 "parser.cc"
        break;

      case symbol_kind::S_NOTE: // "note"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1122 "parser.cc"
        break;

      case symbol_kind::S_IN_PORT: // "in_port"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1128 "parser.cc"
        break;

      case symbol_kind::S_OUT_PORT: // "out_port"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1134 "parser.cc"
        break;

      case symbol_kind::S_ZEROARGFUNC: // "zeroargfunc"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1140 "parser.cc"
        break;

      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1146 "parser.cc"
        break;

      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1152 "parser.cc"
        break;

      case symbol_kind::S_COMPARISON: // "comparison"
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 1158 "parser.cc"
        break;

      case symbol_kind::S_blocks: // blocks
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Blocks > (); }
#line 1164 "parser.cc"
        break;

      case symbol_kind::S_block: // block
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Block > (); }
#line 1170 "parser.cc"
        break;

      case symbol_kind::S_main_block: // main_block
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Block > (); }
#line 1176 "parser.cc"
        break;

      case symbol_kind::S_zero_or_more_statements: // zero_or_more_statements
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1182 "parser.cc"
        break;

      case symbol_kind::S_one_or_more_statements: // one_or_more_statements
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1188 "parser.cc"
        break;

      case symbol_kind::S_statement: // statement
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1194 "parser.cc"
        break;

      case symbol_kind::S_array_assignment: // array_assignment
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1200 "parser.cc"
        break;

      case symbol_kind::S_assignment: // assignment
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1206 "parser.cc"
        break;

      case symbol_kind::S_clear_statement: // clear_statement
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1212 "parser.cc"
        break;

      case symbol_kind::S_continue_statement: // continue_statement
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1218 "parser.cc"
        break;

      case symbol_kind::S_exit_statement: // exit_statement
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1224 "parser.cc"
        break;

      case symbol_kind::S_for_statement: // for_statement
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1230 "parser.cc"
        break;

      case symbol_kind::S_elseif_group: // elseif_group
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1236 "parser.cc"
        break;

      case symbol_kind::S_elseif_clause: // elseif_clause
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1242 "parser.cc"
        break;

      case symbol_kind::S_if_statement: // if_statement
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1248 "parser.cc"
        break;

      case symbol_kind::S_procedure_call: // procedure_call
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1254 "parser.cc"
        break;

      case symbol_kind::S_reset_statement: // reset_statement
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1260 "parser.cc"
        break;

      case symbol_kind::S_wait_statement: // wait_statement
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1266 "parser.cc"
        break;

      case symbol_kind::S_while_statement: // while_statement
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1272 "parser.cc"
        break;

      case symbol_kind::S_expression_list: // expression_list
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < ExpressionList > (); }
#line 1278 "parser.cc"
        break;

      case symbol_kind::S_exp: // exp
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Expression > (); }
#line 1284 "parser.cc"
        break;

      case symbol_kind::S_string_list: // string_list
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < ExpressionList > (); }
#line 1290 "parser.cc"
        break;

      case symbol_kind::S_string_exp: // string_exp
#line 136 "parser.yy"
                 { yyo << yysym.value.template as < Expression > (); }
#line 1296 "parser.cc"
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
      case symbol_kind::S_while_statement: // while_statement
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
      case symbol_kind::S_NEXTHIGHCPU: // "nexthighcpu"
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
      case symbol_kind::S_WHILE: // "while"
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
#line 142 "parser.yy"
                                  { drv.blocks = yystack_[1].value.as < Blocks > ().block_list; }
#line 1674 "parser.cc"
    break;

  case 3: // blocks: main_block
#line 145 "parser.yy"
                                  { yylhs.value.as < Blocks > () = Blocks(yystack_[0].value.as < Block > ()); }
#line 1680 "parser.cc"
    break;

  case 4: // blocks: block
#line 146 "parser.yy"
                                  { yylhs.value.as < Blocks > () = Blocks(yystack_[0].value.as < Block > ()); }
#line 1686 "parser.cc"
    break;

  case 5: // blocks: blocks block
#line 147 "parser.yy"
                                  { yylhs.value.as < Blocks > () = yystack_[1].value.as < Blocks > ().Add(yystack_[0].value.as < Block > ()); }
#line 1692 "parser.cc"
    break;

  case 6: // block: "also" one_or_more_statements "end" "also"
#line 150 "parser.yy"
                                              { yylhs.value.as < Block > () = Block::MainBlock(yystack_[2].value.as < Statements > ()); }
#line 1698 "parser.cc"
    break;

  case 7: // block: "when" exp one_or_more_statements "end" "when"
#line 151 "parser.yy"
                                                 { yylhs.value.as < Block > () = Block::WhenExpBlock(yystack_[3].value.as < Expression > (), yystack_[2].value.as < Statements > ()); }
#line 1704 "parser.cc"
    break;

  case 8: // main_block: one_or_more_statements
#line 154 "parser.yy"
                                  { yylhs.value.as < Block > () = Block::MainBlock(yystack_[0].value.as < Statements > ()); }
#line 1710 "parser.cc"
    break;

  case 9: // zero_or_more_statements: %empty
#line 157 "parser.yy"
                                     {}
#line 1716 "parser.cc"
    break;

  case 10: // zero_or_more_statements: zero_or_more_statements statement
#line 158 "parser.yy"
                                     { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1722 "parser.cc"
    break;

  case 11: // one_or_more_statements: statement
#line 161 "parser.yy"
                                   { yylhs.value.as < Statements > ()  = Statements::FirstStatement(yystack_[0].value.as < Line > ()); }
#line 1728 "parser.cc"
    break;

  case 12: // one_or_more_statements: one_or_more_statements statement
#line 162 "parser.yy"
                                   { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1734 "parser.cc"
    break;

  case 13: // statement: array_assignment
#line 165 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1740 "parser.cc"
    break;

  case 14: // statement: assignment
#line 166 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1746 "parser.cc"
    break;

  case 15: // statement: clear_statement
#line 167 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1752 "parser.cc"
    break;

  case 16: // statement: continue_statement
#line 168 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1758 "parser.cc"
    break;

  case 17: // statement: exit_statement
#line 169 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1764 "parser.cc"
    break;

  case 18: // statement: for_statement
#line 170 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1770 "parser.cc"
    break;

  case 19: // statement: if_statement
#line 171 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1776 "parser.cc"
    break;

  case 20: // statement: procedure_call
#line 172 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1782 "parser.cc"
    break;

  case 21: // statement: reset_statement
#line 173 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1788 "parser.cc"
    break;

  case 22: // statement: wait_statement
#line 174 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1794 "parser.cc"
    break;

  case 23: // statement: while_statement
#line 175 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1800 "parser.cc"
    break;

  case 24: // array_assignment: "identifier" "[" exp "]" "=" exp
#line 178 "parser.yy"
                                    { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1806 "parser.cc"
    break;

  case 25: // array_assignment: "identifier" "[" exp "]" "=" "{" expression_list "}"
#line 179 "parser.yy"
                                                        { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[7].value.as < std::string > (), yystack_[5].value.as < Expression > (), yystack_[1].value.as < ExpressionList > (), &drv); }
#line 1812 "parser.cc"
    break;

  case 26: // array_assignment: "out_port" "[" exp "]" "=" exp
#line 180 "parser.yy"
                                 { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1818 "parser.cc"
    break;

  case 27: // array_assignment: "identifier" "$" "[" exp "]" "=" exp
#line 181 "parser.yy"
                                        { yylhs.value.as < Line > () = Line::StringArrayAssignment(yystack_[6].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1824 "parser.cc"
    break;

  case 28: // array_assignment: "identifier" "$" "[" exp "]" "=" string_exp
#line 182 "parser.yy"
                                               { yylhs.value.as < Line > () = Line::StringArrayAssignment(yystack_[6].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1830 "parser.cc"
    break;

  case 29: // array_assignment: "identifier" "$" "[" exp "]" "=" "{" string_list "}"
#line 183 "parser.yy"
                                                        { yylhs.value.as < Line > () = Line::StringArrayAssignment(yystack_[8].value.as < std::string > (), yystack_[5].value.as < Expression > (), yystack_[1].value.as < ExpressionList > (), &drv); }
#line 1836 "parser.cc"
    break;

  case 30: // assignment: "identifier" "=" exp
#line 186 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1842 "parser.cc"
    break;

  case 31: // assignment: "in_port" "=" exp
#line 187 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1848 "parser.cc"
    break;

  case 32: // assignment: "out_port" "=" exp
#line 188 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1854 "parser.cc"
    break;

  case 33: // assignment: "identifier" "$" "=" exp
#line 189 "parser.yy"
                            { yylhs.value.as < Line > () = Line::StringAssignment(yystack_[3].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1860 "parser.cc"
    break;

  case 34: // assignment: "identifier" "$" "=" string_exp
#line 190 "parser.yy"
                                   { yylhs.value.as < Line > () = Line::StringAssignment(yystack_[3].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1866 "parser.cc"
    break;

  case 35: // clear_statement: "clear" "all"
#line 193 "parser.yy"
                        { yylhs.value.as < Line > () = Line::ClearAll(); }
#line 1872 "parser.cc"
    break;

  case 36: // continue_statement: "continue" "for"
#line 196 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1878 "parser.cc"
    break;

  case 37: // continue_statement: "continue" "while"
#line 197 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1884 "parser.cc"
    break;

  case 38: // continue_statement: "continue" "all"
#line 198 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1890 "parser.cc"
    break;

  case 39: // exit_statement: "exit" "for"
#line 201 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1896 "parser.cc"
    break;

  case 40: // exit_statement: "exit" "while"
#line 202 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1902 "parser.cc"
    break;

  case 41: // exit_statement: "exit" "all"
#line 203 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1908 "parser.cc"
    break;

  case 42: // for_statement: "for" assignment "to" exp zero_or_more_statements "next"
#line 206 "parser.yy"
                                                            { yylhs.value.as < Line > () = Line::ForNext(yystack_[4].value.as < Line > (), yystack_[2].value.as < Expression > (), drv.factory.Number(1.0), yystack_[1].value.as < Statements > (), true, &drv); }
#line 1914 "parser.cc"
    break;

  case 43: // for_statement: "for" assignment "to" exp "step" exp zero_or_more_statements "next"
#line 207 "parser.yy"
                                                                      { yylhs.value.as < Line > () = Line::ForNext(yystack_[6].value.as < Line > (), yystack_[4].value.as < Expression > (), yystack_[2].value.as < Expression > (), yystack_[1].value.as < Statements > (), true, &drv); }
#line 1920 "parser.cc"
    break;

  case 44: // for_statement: "for" assignment "to" exp zero_or_more_statements "nexthighcpu"
#line 208 "parser.yy"
                                                                   { yylhs.value.as < Line > () = Line::ForNext(yystack_[4].value.as < Line > (), yystack_[2].value.as < Expression > (), drv.factory.Number(1.0), yystack_[1].value.as < Statements > (), false, &drv); }
#line 1926 "parser.cc"
    break;

  case 45: // for_statement: "for" assignment "to" exp "step" exp zero_or_more_statements "nexthighcpu"
#line 209 "parser.yy"
                                                                             { yylhs.value.as < Line > () = Line::ForNext(yystack_[6].value.as < Line > (), yystack_[4].value.as < Expression > (), yystack_[2].value.as < Expression > (), yystack_[1].value.as < Statements > (), false, &drv); }
#line 1932 "parser.cc"
    break;

  case 46: // elseif_group: %empty
#line 212 "parser.yy"
                                  {}
#line 1938 "parser.cc"
    break;

  case 47: // elseif_group: elseif_group elseif_clause
#line 213 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1944 "parser.cc"
    break;

  case 48: // elseif_clause: "elseif" exp "then" zero_or_more_statements
#line 216 "parser.yy"
                                               { yylhs.value.as < Line > () = Line::ElseIf(yystack_[2].value.as < Expression > (), yystack_[0].value.as < Statements > ()); }
#line 1950 "parser.cc"
    break;

  case 49: // if_statement: "if" exp "then" zero_or_more_statements elseif_group "end" "if"
#line 219 "parser.yy"
                                                                        { yylhs.value.as < Line > () = Line::IfThen(yystack_[5].value.as < Expression > (), yystack_[3].value.as < Statements > (), yystack_[2].value.as < Statements > ()); }
#line 1956 "parser.cc"
    break;

  case 50: // if_statement: "if" exp "then" zero_or_more_statements elseif_group "else" zero_or_more_statements "end" "if"
#line 220 "parser.yy"
                                                                                                  { yylhs.value.as < Line > () = Line::IfThenElse(yystack_[7].value.as < Expression > (), yystack_[5].value.as < Statements > (), yystack_[2].value.as < Statements > (), yystack_[4].value.as < Statements > ()); }
#line 1962 "parser.cc"
    break;

  case 51: // procedure_call: "print" "(" "out_port" "," string_list ")"
#line 223 "parser.yy"
                                              {yylhs.value.as < Line > () = Line::Print(yystack_[3].value.as < std::string > (), yystack_[1].value.as < ExpressionList > (), &drv);}
#line 1968 "parser.cc"
    break;

  case 52: // procedure_call: "set_channels" "(" "out_port" "," exp ")"
#line 224 "parser.yy"
                                              {yylhs.value.as < Line > () = Line::SetChannels(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv);}
#line 1974 "parser.cc"
    break;

  case 53: // reset_statement: "reset"
#line 227 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Reset(); }
#line 1980 "parser.cc"
    break;

  case 54: // wait_statement: "wait" exp
#line 230 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Wait(yystack_[0].value.as < Expression > ()); }
#line 1986 "parser.cc"
    break;

  case 55: // while_statement: "while" exp zero_or_more_statements "end" "while"
#line 233 "parser.yy"
                                                     { yylhs.value.as < Line > () = Line::While(yystack_[3].value.as < Expression > (), yystack_[2].value.as < Statements > (), &drv); }
#line 1992 "parser.cc"
    break;

  case 56: // expression_list: exp
#line 244 "parser.yy"
                              { yylhs.value.as < ExpressionList > () = ExpressionList(yystack_[0].value.as < Expression > ()); }
#line 1998 "parser.cc"
    break;

  case 57: // expression_list: expression_list "," exp
#line 245 "parser.yy"
                              { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 2004 "parser.cc"
    break;

  case 58: // exp: "number"
#line 248 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Number((float) yystack_[0].value.as < float > ()); }
#line 2010 "parser.cc"
    break;

  case 59: // exp: "note"
#line 249 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Note(yystack_[0].value.as < std::string > ()); }
#line 2016 "parser.cc"
    break;

  case 60: // exp: "-" "number"
#line 250 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.Number(-1 * (float) yystack_[0].value.as < float > ());}
#line 2022 "parser.cc"
    break;

  case 61: // exp: "not" exp
#line 251 "parser.yy"
                      { yylhs.value.as < Expression > () = drv.factory.Not(yystack_[0].value.as < Expression > ());}
#line 2028 "parser.cc"
    break;

  case 62: // exp: "in_port"
#line 252 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 2034 "parser.cc"
    break;

  case 63: // exp: "in_port" "[" exp "]"
#line 253 "parser.yy"
                        { yylhs.value.as < Expression > () = drv.factory.ArrayVariable(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2040 "parser.cc"
    break;

  case 64: // exp: "out_port"
#line 254 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 2046 "parser.cc"
    break;

  case 65: // exp: "identifier"
#line 255 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 2052 "parser.cc"
    break;

  case 66: // exp: "identifier" "[" exp "]"
#line 256 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.ArrayVariable(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2058 "parser.cc"
    break;

  case 67: // exp: exp "+" exp
#line 257 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2064 "parser.cc"
    break;

  case 68: // exp: exp "-" exp
#line 258 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2070 "parser.cc"
    break;

  case 69: // exp: exp "*" exp
#line 259 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2076 "parser.cc"
    break;

  case 70: // exp: exp "/" exp
#line 260 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2082 "parser.cc"
    break;

  case 71: // exp: exp "comparison" exp
#line 261 "parser.yy"
                       { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2088 "parser.cc"
    break;

  case 72: // exp: exp "and" exp
#line 262 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2094 "parser.cc"
    break;

  case 73: // exp: exp "or" exp
#line 263 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 2100 "parser.cc"
    break;

  case 74: // exp: "zeroargfunc" "(" ")"
#line 264 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.ZeroArgFunc(yystack_[2].value.as < std::string > ());}
#line 2106 "parser.cc"
    break;

  case 75: // exp: "channels" "(" "in_port" ")"
#line 265 "parser.yy"
                                {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 2112 "parser.cc"
    break;

  case 76: // exp: "connected" "(" "in_port" ")"
#line 266 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 2118 "parser.cc"
    break;

  case 77: // exp: "connected" "(" "out_port" ")"
#line 267 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 2124 "parser.cc"
    break;

  case 78: // exp: "trigger" "(" "in_port" ")"
#line 268 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 2130 "parser.cc"
    break;

  case 79: // exp: "oneargfunc" "(" exp ")"
#line 269 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.OneArgFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > ());}
#line 2136 "parser.cc"
    break;

  case 80: // exp: "twoargfunc" "(" exp "," exp ")"
#line 270 "parser.yy"
                                   {yylhs.value.as < Expression > () = drv.factory.TwoArgFunc(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > ());}
#line 2142 "parser.cc"
    break;

  case 81: // exp: exp "?" exp ":" exp
#line 271 "parser.yy"
                                 {yylhs.value.as < Expression > () = drv.factory.TernaryFunc(yystack_[4].value.as < Expression > (), yystack_[2].value.as < Expression > (), yystack_[0].value.as < Expression > ());}
#line 2148 "parser.cc"
    break;

  case 82: // exp: "(" exp ")"
#line 272 "parser.yy"
                { yylhs.value.as < Expression > () = yystack_[1].value.as < Expression > (); }
#line 2154 "parser.cc"
    break;

  case 83: // string_list: string_exp
#line 276 "parser.yy"
                             { ExpressionList foo = ExpressionList::StringList();
                               yylhs.value.as < ExpressionList > () = foo.add(yystack_[0].value.as < Expression > ()); }
#line 2161 "parser.cc"
    break;

  case 84: // string_list: exp
#line 278 "parser.yy"
                             { ExpressionList foo = ExpressionList::StringList();
                               yylhs.value.as < ExpressionList > () = foo.add(yystack_[0].value.as < Expression > ()); }
#line 2168 "parser.cc"
    break;

  case 85: // string_list: string_list "," string_exp
#line 280 "parser.yy"
                             { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 2174 "parser.cc"
    break;

  case 86: // string_list: string_list "," exp
#line 281 "parser.yy"
                             { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 2180 "parser.cc"
    break;

  case 87: // string_exp: "quoted_string"
#line 284 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.Quoted(yystack_[0].value.as < std::string > ()); }
#line 2186 "parser.cc"
    break;

  case 88: // string_exp: "identifier" "$"
#line 285 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.StringVariable(yystack_[1].value.as < std::string > (), &drv); }
#line 2192 "parser.cc"
    break;

  case 89: // string_exp: "identifier" "$" "[" exp "]"
#line 286 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.StringArrayVariable(yystack_[4].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2198 "parser.cc"
    break;

  case 90: // string_exp: "debug" "(" "identifier" ")"
#line 287 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.DebugId(yystack_[1].value.as < std::string > (), &drv); }
#line 2204 "parser.cc"
    break;

  case 91: // string_exp: "debug" "(" "identifier" "$" ")"
#line 288 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.DebugIdString(yystack_[2].value.as < std::string > (), &drv); }
#line 2210 "parser.cc"
    break;

  case 92: // string_exp: "debug" "(" "identifier" "[" "]" "," exp "," exp ")"
#line 289 "parser.yy"
                                                       { yylhs.value.as < Expression > () = drv.factory.DebugId(yystack_[7].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2216 "parser.cc"
    break;

  case 93: // string_exp: "debug" "(" "identifier" "$" "[" "]" "," exp "," exp ")"
#line 290 "parser.yy"
                                                           { yylhs.value.as < Expression > () = drv.factory.DebugIdString(yystack_[8].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > (), &drv); }
#line 2222 "parser.cc"
    break;


#line 2226 "parser.cc"

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
  "log10", "max", "min", "next", "nexthighcpu", "not", "or", "pow",
  "print", "reset", "sample_rate", "set_channels", "sign", "sin", "start",
  "step", "then", "time", "time_millis", "to", "trigger", "wait", "when",
  "while", "-", "+", "*", "/", "(", ")", "{", "}", "[", "]", ",", "$", "?",
  ":", "identifier", "quoted_string", "number", "note", "in_port",
  "out_port", "zeroargfunc", "oneargfunc", "twoargfunc", "comparison",
  "NEG", "$accept", "program", "blocks", "block", "main_block",
  "zero_or_more_statements", "one_or_more_statements", "statement",
  "array_assignment", "assignment", "clear_statement",
  "continue_statement", "exit_statement", "for_statement", "elseif_group",
  "elseif_clause", "if_statement", "procedure_call", "reset_statement",
  "wait_statement", "while_statement", "expression_list", "exp",
  "string_list", "string_exp", YY_NULLPTR
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


  const signed char Parser::yypact_ninf_ = -103;

  const signed char Parser::yytable_ninf_ = -1;

  const short
  Parser::yypact_[] =
  {
     422,   762,    13,   116,   133,   -53,   543,   -24,  -103,   -14,
     543,   543,   543,    41,    32,    -5,    49,    15,  -103,  -103,
     762,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,  -103,  -103,   663,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,     7,    44,    30,    25,    33,   543,    58,    51,   543,
      66,  -103,  -103,    70,  -103,    76,    79,    80,    -3,    77,
      78,   381,   136,   381,   543,   543,    11,   543,   543,   543,
    -103,  -103,  -103,  -103,   127,   142,   543,    69,    28,  -103,
      86,  -103,     5,   543,   543,   105,   543,   543,   543,   543,
    -103,   543,   543,   543,   543,   543,   543,   102,   103,   686,
     723,   381,    63,   473,   543,   381,   381,   144,  -103,   170,
     109,   111,   114,   120,  -103,   175,   199,  -103,   204,   230,
      53,    68,   762,    56,    56,  -103,  -103,   235,   -17,   473,
     543,   132,   141,  -103,   171,   137,   -34,  -103,   381,  -103,
     259,   182,   543,   603,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,   543,   125,   543,   381,   -29,  -103,   264,  -103,  -103,
     516,   129,   154,   189,   543,   381,  -103,  -103,   290,  -103,
     543,   193,  -103,   381,  -103,   473,  -103,   543,   381,   -33,
     543,   446,   381,   626,  -103,   746,   296,  -103,   381,  -103,
     -16,   381,  -103,   160,    52,   321,   473,   381,  -103,  -103,
    -103,   201,  -103,  -103,   543,   169,  -103,   174,  -103,    72,
    -103,   762,   381,   543,   178,  -103,   304,   543,   543,   335,
     350,   543,  -103,   366,  -103
  };

  const signed char
  Parser::yydefact_[] =
  {
       0,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     3,
       8,    11,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,     0,    35,    38,    36,    37,    41,    39,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      65,    58,    59,    62,    64,     0,     0,     0,     0,     0,
       0,    54,     0,     9,     0,     0,     0,     0,     0,     0,
       1,     2,     5,    12,     0,     0,     0,     0,     0,    61,
       0,    60,     0,     0,     0,     0,     0,     0,     0,     0,
       9,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    30,     0,     0,     0,    31,    32,     0,     6,     9,
       0,     0,     0,     0,    82,     0,     0,    74,     0,     0,
      72,    73,    46,    68,    67,    69,    70,     0,    71,     0,
       0,     0,     0,    10,     0,     0,    65,    87,    33,    34,
       0,     0,     0,     0,    75,    76,    77,    78,    66,    63,
      79,     0,     0,     0,    84,     0,    83,     0,     7,    55,
       0,     0,    88,     0,     0,     9,    42,    44,     0,     9,
       0,     0,    47,    81,    51,     0,    52,     0,    24,     0,
       0,     0,    26,     0,    80,     0,     0,    49,    86,    85,
       0,    56,    90,     0,     0,     0,     0,    27,    28,    43,
      45,     0,     9,    25,     0,     0,    91,     0,    89,     0,
      50,    48,    57,     0,     0,    29,     0,     0,     0,     0,
       0,     0,    92,     0,    93
  };

  const short
  Parser::yypgoto_[] =
  {
    -103,  -103,  -103,   210,  -103,   -74,     6,     9,  -103,   227,
    -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,  -103,    -6,    42,  -102
  };

  const unsigned char
  Parser::yydefgoto_[] =
  {
       0,    16,    17,    18,    19,   100,    20,   133,    22,    23,
      24,    25,    26,    27,   152,   172,    28,    29,    30,    31,
      32,   190,   154,   155,   156
  };

  const unsigned char
  Parser::yytable_[] =
  {
      58,   139,    68,    88,    61,    62,    63,    33,    41,    21,
      21,    88,    14,    42,    64,    71,   122,    34,   103,   192,
       1,    83,   193,   174,   162,   194,    89,    59,   175,    73,
      91,    92,    93,    94,    89,   143,    90,    60,   203,    67,
      79,   204,    73,    82,    91,    92,    93,    94,    64,    70,
      69,    68,    91,    92,    93,    94,    95,   114,   101,   102,
      11,   105,   106,   107,    95,    75,   104,    96,    99,    88,
     109,    21,    76,   189,    88,    96,    77,   115,   116,   198,
     118,   119,   120,   121,    78,   123,   124,   125,   126,   127,
     128,   183,    89,   111,   112,   185,    65,   138,   140,    66,
      91,    92,    93,    94,   206,    93,    94,   207,    73,    80,
      91,    92,    93,    94,    81,    91,    92,    93,    94,   134,
      35,    83,    95,    96,   157,    84,   215,    85,   211,   175,
      86,    87,   108,    96,   110,    36,   165,    38,    96,   169,
     170,   171,    88,    97,    98,   168,     2,   173,     3,   103,
      88,   113,    39,     4,   178,     5,     6,   117,   182,   129,
     130,   144,    37,   145,   186,    89,   146,     7,     8,   188,
       9,   191,   147,    89,   195,   197,    88,   158,   160,    40,
      10,    88,    12,    91,    92,    93,    94,   159,   161,   164,
     179,    91,    92,    93,    94,    95,   181,    13,   212,    89,
     141,    14,    15,    95,    89,    88,    96,   216,   142,   180,
      88,   219,   220,   187,    96,   223,   205,    91,    92,    93,
      94,   210,    91,    92,    93,    94,   213,    72,    89,    95,
     214,   148,    43,    89,    95,   217,    88,     0,   209,     0,
      96,    88,     0,     0,     0,    96,    91,    92,    93,    94,
       0,    91,    92,    93,    94,   149,   150,     0,    95,    89,
       0,     0,     0,    95,    89,    88,     0,     0,     0,    96,
      88,     0,     0,     0,    96,     0,     0,    91,    92,    93,
      94,     0,    91,    92,    93,    94,     0,   151,    89,    95,
       0,     0,     0,    89,    95,   153,    88,     0,     0,     0,
      96,     0,    88,     0,     0,    96,    91,    92,    93,    94,
      88,    91,    92,    93,    94,   163,   176,     0,    95,    89,
       0,     0,     0,    95,     0,    89,     0,    88,     0,    96,
       0,     0,     0,    89,    96,   202,     0,    91,    92,    93,
      94,    88,   184,    91,    92,    93,    94,     0,     0,    95,
      89,    91,    92,    93,    94,    95,    88,     0,     0,     0,
      96,   218,     0,    95,    89,     0,    96,     0,    91,    92,
      93,    94,    88,     0,    96,     0,     0,   208,     0,    89,
      95,     0,    91,    92,    93,    94,     0,    88,     0,     0,
       0,    96,   221,     0,    95,    89,     0,    91,    92,    93,
      94,     0,   222,     0,     0,    96,     0,     0,     0,    95,
      89,     0,     0,    91,    92,    93,    94,     0,   224,     0,
      96,     0,     0,     0,     0,    95,     0,     1,    91,    92,
      93,    94,     2,     0,     3,     0,    96,     0,     0,     4,
      95,     5,     6,     0,     0,     0,     0,     0,     0,     0,
       0,    96,     0,     7,     8,    44,     9,    45,     0,   135,
       0,     0,     0,     0,     0,     0,    10,    11,    12,     0,
       0,     0,     0,     0,    46,     0,     0,     0,     0,     0,
       0,     0,    44,    13,    45,     0,   135,    14,    15,    47,
       0,     0,     0,    48,     0,     0,     0,    49,     0,   196,
       0,    46,     0,     0,     0,     0,     0,   136,   137,    51,
      52,    53,    54,    55,    56,    57,    47,     0,     0,     0,
      48,     0,     0,     0,    49,    44,     0,    45,     0,     0,
       0,     0,     0,     0,   136,   137,    51,    52,    53,    54,
      55,    56,    57,     0,    46,     0,     0,     0,     0,     0,
       0,     0,    44,     0,    45,     0,     0,     0,     0,    47,
       0,     0,     0,    48,     0,     0,     0,    49,     0,   177,
       0,    46,     0,     0,     0,     0,     0,    50,     0,    51,
      52,    53,    54,    55,    56,    57,    47,     0,     0,     0,
      48,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,    51,    52,    53,    54,
      55,    56,    57,     2,     0,     3,     0,     0,     0,     0,
       4,     0,     5,     6,     0,     0,     0,     0,     0,   166,
     167,     0,     0,     0,     7,     8,     2,     9,     3,     0,
       0,     0,     0,     4,     0,     5,     6,    10,     0,    12,
       0,     0,   199,   200,     0,     0,     0,     7,     8,     0,
       9,     0,     0,     0,    13,     0,     0,     0,    14,    15,
      10,     0,    12,     2,     0,     3,     0,     0,     0,    74,
       4,     0,     5,     6,     0,     0,     0,    13,     0,     0,
       0,    14,    15,     0,     7,     8,     2,     9,     3,     0,
       0,     0,   131,     4,     0,     5,     6,    10,     0,    12,
       0,     0,     0,     0,     0,     0,     0,     7,     8,     0,
       9,     0,     0,     0,    13,     0,     0,     0,    14,    15,
      10,     0,    12,     2,     0,     3,     0,     0,     0,   132,
       4,     0,     5,     6,     0,     0,     0,    13,     0,     0,
       0,    14,    15,     0,     7,     8,     2,     9,     3,     0,
       0,     0,   201,     4,     0,     5,     6,    10,     0,    12,
       0,     0,     2,     0,     3,     0,     0,     7,     8,     4,
       9,     5,     6,     0,    13,     0,     0,     0,    14,    15,
      10,     0,    12,     7,     8,     0,     9,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    10,    13,    12,     0,
       0,    14,    15,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,     0,     0,     0,    14,    15
  };

  const short
  Parser::yycheck_[] =
  {
       6,   103,     7,     6,    10,    11,    12,     1,    61,     0,
       1,     6,    65,    66,     7,     0,    90,     4,     7,    52,
       5,    55,    55,    52,    58,    58,    29,    51,    57,    20,
      47,    48,    49,    50,    29,   109,    39,    51,    54,     7,
      46,    57,    33,    49,    47,    48,    49,    50,     7,     0,
      55,     7,    47,    48,    49,    50,    59,    52,    64,    65,
      45,    67,    68,    69,    59,    58,    55,    70,    62,     6,
      76,    62,    42,   175,     6,    70,    51,    83,    84,   181,
      86,    87,    88,    89,    51,    91,    92,    93,    94,    95,
      96,   165,    29,    65,    66,   169,    55,   103,   104,    58,
      47,    48,    49,    50,    52,    49,    50,    55,    99,    51,
      47,    48,    49,    50,    63,    47,    48,    49,    50,    56,
       4,    55,    59,    70,   130,    55,    54,    51,   202,    57,
      51,    51,     5,    70,    65,    19,   142,     4,    70,    14,
      15,    16,     6,    66,    66,   151,    10,   153,    12,     7,
       6,    65,    19,    17,   160,    19,    20,    52,   164,    57,
      57,    52,    46,    52,   170,    29,    52,    31,    32,   175,
      34,   177,    52,    29,   180,   181,     6,    45,     7,    46,
      44,     6,    46,    47,    48,    49,    50,    46,    51,     7,
      61,    47,    48,    49,    50,    59,     7,    61,   204,    29,
      56,    65,    66,    59,    29,     6,    70,   213,    38,    55,
       6,   217,   218,    20,    70,   221,    56,    47,    48,    49,
      50,    20,    47,    48,    49,    50,    57,    17,    29,    59,
      56,    56,     5,    29,    59,    57,     6,    -1,   196,    -1,
      70,     6,    -1,    -1,    -1,    70,    47,    48,    49,    50,
      -1,    47,    48,    49,    50,    56,    52,    -1,    59,    29,
      -1,    -1,    -1,    59,    29,     6,    -1,    -1,    -1,    70,
       6,    -1,    -1,    -1,    70,    -1,    -1,    47,    48,    49,
      50,    -1,    47,    48,    49,    50,    -1,    57,    29,    59,
      -1,    -1,    -1,    29,    59,    60,     6,    -1,    -1,    -1,
      70,    -1,     6,    -1,    -1,    70,    47,    48,    49,    50,
       6,    47,    48,    49,    50,    56,    52,    -1,    59,    29,
      -1,    -1,    -1,    59,    -1,    29,    -1,     6,    -1,    70,
      -1,    -1,    -1,    29,    70,    39,    -1,    47,    48,    49,
      50,     6,    52,    47,    48,    49,    50,    -1,    -1,    59,
      29,    47,    48,    49,    50,    59,     6,    -1,    -1,    -1,
      70,    57,    -1,    59,    29,    -1,    70,    -1,    47,    48,
      49,    50,     6,    -1,    70,    -1,    -1,    56,    -1,    29,
      59,    -1,    47,    48,    49,    50,    -1,     6,    -1,    -1,
      -1,    70,    57,    -1,    59,    29,    -1,    47,    48,    49,
      50,    -1,    52,    -1,    -1,    70,    -1,    -1,    -1,    59,
      29,    -1,    -1,    47,    48,    49,    50,    -1,    52,    -1,
      70,    -1,    -1,    -1,    -1,    59,    -1,     5,    47,    48,
      49,    50,    10,    -1,    12,    -1,    70,    -1,    -1,    17,
      59,    19,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    31,    32,     9,    34,    11,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    46,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    61,    11,    -1,    13,    65,    66,    43,
      -1,    -1,    -1,    47,    -1,    -1,    -1,    51,    -1,    53,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    43,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    51,     9,    -1,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    -1,    11,    -1,    -1,    -1,    -1,    43,
      -1,    -1,    -1,    47,    -1,    -1,    -1,    51,    -1,    53,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    61,    -1,    63,
      64,    65,    66,    67,    68,    69,    43,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    61,    -1,    63,    64,    65,    66,
      67,    68,    69,    10,    -1,    12,    -1,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    -1,    26,
      27,    -1,    -1,    -1,    31,    32,    10,    34,    12,    -1,
      -1,    -1,    -1,    17,    -1,    19,    20,    44,    -1,    46,
      -1,    -1,    26,    27,    -1,    -1,    -1,    31,    32,    -1,
      34,    -1,    -1,    -1,    61,    -1,    -1,    -1,    65,    66,
      44,    -1,    46,    10,    -1,    12,    -1,    -1,    -1,    16,
      17,    -1,    19,    20,    -1,    -1,    -1,    61,    -1,    -1,
      -1,    65,    66,    -1,    31,    32,    10,    34,    12,    -1,
      -1,    -1,    16,    17,    -1,    19,    20,    44,    -1,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,
      34,    -1,    -1,    -1,    61,    -1,    -1,    -1,    65,    66,
      44,    -1,    46,    10,    -1,    12,    -1,    -1,    -1,    16,
      17,    -1,    19,    20,    -1,    -1,    -1,    61,    -1,    -1,
      -1,    65,    66,    -1,    31,    32,    10,    34,    12,    -1,
      -1,    -1,    16,    17,    -1,    19,    20,    44,    -1,    46,
      -1,    -1,    10,    -1,    12,    -1,    -1,    31,    32,    17,
      34,    19,    20,    -1,    61,    -1,    -1,    -1,    65,    66,
      44,    -1,    46,    31,    32,    -1,    34,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    61,    46,    -1,
      -1,    65,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    -1,    -1,    -1,    65,    66
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     5,    10,    12,    17,    19,    20,    31,    32,    34,
      44,    45,    46,    61,    65,    66,    73,    74,    75,    76,
      78,    79,    80,    81,    82,    83,    84,    85,    88,    89,
      90,    91,    92,    78,     4,     4,    19,    46,     4,    19,
      46,    61,    66,    81,     9,    11,    28,    43,    47,    51,
      61,    63,    64,    65,    66,    67,    68,    69,    94,    51,
      51,    94,    94,    94,     7,    55,    58,     7,     7,    55,
       0,     0,    75,    79,    16,    58,    42,    51,    51,    94,
      51,    63,    94,    55,    55,    51,    51,    51,     6,    29,
      39,    47,    48,    49,    50,    59,    70,    66,    66,    78,
      77,    94,    94,     7,    55,    94,    94,    94,     5,    94,
      65,    65,    66,    65,    52,    94,    94,    52,    94,    94,
      94,    94,    77,    94,    94,    94,    94,    94,    94,    57,
      57,    16,    16,    79,    56,    13,    61,    62,    94,    96,
      94,    56,    38,    77,    52,    52,    52,    52,    56,    56,
      52,    57,    86,    60,    94,    95,    96,    94,    45,    46,
       7,    51,    58,    56,     7,    94,    26,    27,    94,    14,
      15,    16,    87,    94,    52,    57,    52,    53,    94,    61,
      55,     7,    94,    77,    52,    77,    94,    20,    94,    96,
      93,    94,    52,    55,    58,    94,    53,    94,    96,    26,
      27,    16,    39,    54,    57,    56,    52,    55,    56,    95,
      20,    77,    94,    57,    56,    54,    94,    57,    57,    94,
      94,    57,    52,    94,    52
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    72,    73,    74,    74,    74,    75,    75,    76,    77,
      77,    78,    78,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    80,    80,    80,    80,    80,    80,
      81,    81,    81,    81,    81,    82,    83,    83,    83,    84,
      84,    84,    85,    85,    85,    85,    86,    86,    87,    88,
      88,    89,    89,    90,    91,    92,    93,    93,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    95,    95,    95,    95,    96,    96,    96,
      96,    96,    96,    96
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     2,     1,     1,     2,     4,     5,     1,     0,
       2,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     6,     8,     6,     7,     7,     9,
       3,     3,     3,     4,     4,     2,     2,     2,     2,     2,
       2,     2,     6,     8,     6,     8,     0,     2,     4,     7,
       9,     6,     6,     1,     2,     5,     1,     3,     1,     1,
       2,     2,     1,     4,     1,     1,     4,     3,     3,     3,
       3,     3,     3,     3,     3,     4,     4,     4,     4,     4,
       6,     5,     3,     1,     1,     3,     3,     1,     2,     5,
       4,     5,    10,    11
  };




#if YYDEBUG
  const short
  Parser::yyrline_[] =
  {
       0,   142,   142,   145,   146,   147,   150,   151,   154,   157,
     158,   161,   162,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   178,   179,   180,   181,   182,   183,
     186,   187,   188,   189,   190,   193,   196,   197,   198,   201,
     202,   203,   206,   207,   208,   209,   212,   213,   216,   219,
     220,   223,   224,   227,   230,   233,   244,   245,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   276,   278,   280,   281,   284,   285,   286,
     287,   288,   289,   290
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
#line 3052 "parser.cc"

#line 292 "parser.yy"


void
yy::Parser::error (const location_type& l, const std::string& m)
{
  drv.errors.push_back(Error(l.begin.line, l.begin.column, m));
}
