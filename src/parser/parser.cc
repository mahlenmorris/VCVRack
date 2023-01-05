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
#line 32 "parser.yy"

# include "driver.hh"

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
  parser::parser (Driver& drv_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      yy_lac_established_ (false),
      drv (drv_yyarg)
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
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_elseif_clause: // elseif_clause
      case symbol_kind::S_if_statement: // if_statement
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
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
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
      case symbol_kind::S_ONEPORTFUNC: // "oneportfunc"
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

  parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
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
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_elseif_clause: // elseif_clause
      case symbol_kind::S_if_statement: // if_statement
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
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
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
      case symbol_kind::S_ONEPORTFUNC: // "oneportfunc"
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
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
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
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_elseif_clause: // elseif_clause
      case symbol_kind::S_if_statement: // if_statement
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
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
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
      case symbol_kind::S_ONEPORTFUNC: // "oneportfunc"
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

  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (stack_symbol_type& that)
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
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_elseif_clause: // elseif_clause
      case symbol_kind::S_if_statement: // if_statement
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
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
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
      case symbol_kind::S_ONEPORTFUNC: // "oneportfunc"
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
        switch (yykind)
    {
      case symbol_kind::S_ABS: // "abs"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 658 "parser.cc"
        break;

      case symbol_kind::S_ALL: // "all"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 664 "parser.cc"
        break;

      case symbol_kind::S_ALSO: // "also"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 670 "parser.cc"
        break;

      case symbol_kind::S_AND: // "and"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 676 "parser.cc"
        break;

      case symbol_kind::S_ASSIGN: // "="
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 682 "parser.cc"
        break;

      case symbol_kind::S_CEILING: // "ceiling"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 688 "parser.cc"
        break;

      case symbol_kind::S_CONTINUE: // "continue"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 694 "parser.cc"
        break;

      case symbol_kind::S_ELSE: // "else"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 700 "parser.cc"
        break;

      case symbol_kind::S_ELSEIF: // "elseif"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 706 "parser.cc"
        break;

      case symbol_kind::S_END: // "end"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 712 "parser.cc"
        break;

      case symbol_kind::S_EXIT: // "exit"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 718 "parser.cc"
        break;

      case symbol_kind::S_FLOOR: // "floor"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 724 "parser.cc"
        break;

      case symbol_kind::S_FOR: // "for"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 730 "parser.cc"
        break;

      case symbol_kind::S_IF: // "if"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 736 "parser.cc"
        break;

      case symbol_kind::S_MAX: // "max"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 742 "parser.cc"
        break;

      case symbol_kind::S_MIN: // "min"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 748 "parser.cc"
        break;

      case symbol_kind::S_NEXT: // "next"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 754 "parser.cc"
        break;

      case symbol_kind::S_NOT: // "not"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 760 "parser.cc"
        break;

      case symbol_kind::S_OR: // "or"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 766 "parser.cc"
        break;

      case symbol_kind::S_POW: // "pow"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 772 "parser.cc"
        break;

      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 778 "parser.cc"
        break;

      case symbol_kind::S_SIGN: // "sign"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 784 "parser.cc"
        break;

      case symbol_kind::S_SIN: // "sin"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 790 "parser.cc"
        break;

      case symbol_kind::S_START: // "start"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 796 "parser.cc"
        break;

      case symbol_kind::S_STEP: // "step"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 802 "parser.cc"
        break;

      case symbol_kind::S_THEN: // "then"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 808 "parser.cc"
        break;

      case symbol_kind::S_TO: // "to"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 814 "parser.cc"
        break;

      case symbol_kind::S_WAIT: // "wait"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 820 "parser.cc"
        break;

      case symbol_kind::S_WHEN: // "when"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 826 "parser.cc"
        break;

      case symbol_kind::S_MINUS: // "-"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 832 "parser.cc"
        break;

      case symbol_kind::S_PLUS: // "+"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 838 "parser.cc"
        break;

      case symbol_kind::S_STAR: // "*"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 844 "parser.cc"
        break;

      case symbol_kind::S_SLASH: // "/"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 850 "parser.cc"
        break;

      case symbol_kind::S_LPAREN: // "("
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 856 "parser.cc"
        break;

      case symbol_kind::S_RPAREN: // ")"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 862 "parser.cc"
        break;

      case symbol_kind::S_LBRACE: // "{"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 868 "parser.cc"
        break;

      case symbol_kind::S_RBRACE: // "}"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 874 "parser.cc"
        break;

      case symbol_kind::S_LBRACKET: // "["
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 880 "parser.cc"
        break;

      case symbol_kind::S_RBRACKET: // "]"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 886 "parser.cc"
        break;

      case symbol_kind::S_COMMA: // ","
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 892 "parser.cc"
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 898 "parser.cc"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < float > (); }
#line 904 "parser.cc"
        break;

      case symbol_kind::S_NOTE: // "note"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 910 "parser.cc"
        break;

      case symbol_kind::S_IN_PORT: // "in_port"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 916 "parser.cc"
        break;

      case symbol_kind::S_OUT_PORT: // "out_port"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 922 "parser.cc"
        break;

      case symbol_kind::S_ZEROARGFUNC: // "zeroargfunc"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 928 "parser.cc"
        break;

      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 934 "parser.cc"
        break;

      case symbol_kind::S_ONEPORTFUNC: // "oneportfunc"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 940 "parser.cc"
        break;

      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 946 "parser.cc"
        break;

      case symbol_kind::S_COMPARISON: // "comparison"
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 952 "parser.cc"
        break;

      case symbol_kind::S_blocks: // blocks
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Blocks > (); }
#line 958 "parser.cc"
        break;

      case symbol_kind::S_block: // block
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Block > (); }
#line 964 "parser.cc"
        break;

      case symbol_kind::S_main_block: // main_block
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Block > (); }
#line 970 "parser.cc"
        break;

      case symbol_kind::S_zero_or_more_statements: // zero_or_more_statements
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 976 "parser.cc"
        break;

      case symbol_kind::S_one_or_more_statements: // one_or_more_statements
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 982 "parser.cc"
        break;

      case symbol_kind::S_statement: // statement
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 988 "parser.cc"
        break;

      case symbol_kind::S_array_assignment: // array_assignment
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 994 "parser.cc"
        break;

      case symbol_kind::S_assignment: // assignment
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1000 "parser.cc"
        break;

      case symbol_kind::S_continue_statement: // continue_statement
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1006 "parser.cc"
        break;

      case symbol_kind::S_exit_statement: // exit_statement
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1012 "parser.cc"
        break;

      case symbol_kind::S_for_statement: // for_statement
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1018 "parser.cc"
        break;

      case symbol_kind::S_elseif_group: // elseif_group
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 1024 "parser.cc"
        break;

      case symbol_kind::S_elseif_clause: // elseif_clause
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1030 "parser.cc"
        break;

      case symbol_kind::S_if_statement: // if_statement
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1036 "parser.cc"
        break;

      case symbol_kind::S_wait_statement: // wait_statement
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 1042 "parser.cc"
        break;

      case symbol_kind::S_expression_list: // expression_list
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < ExpressionList > (); }
#line 1048 "parser.cc"
        break;

      case symbol_kind::S_exp: // exp
#line 108 "parser.yy"
                 { yyo << yysym.value.template as < Expression > (); }
#line 1054 "parser.cc"
        break;

      default:
        break;
    }
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
            symbol_type yylookahead (yylex (drv));
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
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_elseif_clause: // elseif_clause
      case symbol_kind::S_if_statement: // if_statement
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
      case symbol_kind::S_SAMPLE_RATE: // "sample_rate"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_START: // "start"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
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
      case symbol_kind::S_ONEPORTFUNC: // "oneportfunc"
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
#line 114 "parser.yy"
                                  { drv.blocks = yystack_[1].value.as < Blocks > ().block_list; }
#line 1408 "parser.cc"
    break;

  case 3: // blocks: main_block
#line 117 "parser.yy"
                                  { yylhs.value.as < Blocks > () = Blocks(yystack_[0].value.as < Block > ()); }
#line 1414 "parser.cc"
    break;

  case 4: // blocks: block
#line 118 "parser.yy"
                                  { yylhs.value.as < Blocks > () = Blocks(yystack_[0].value.as < Block > ()); }
#line 1420 "parser.cc"
    break;

  case 5: // blocks: blocks block
#line 119 "parser.yy"
                                  { yylhs.value.as < Blocks > () = yystack_[1].value.as < Blocks > ().Add(yystack_[0].value.as < Block > ()); }
#line 1426 "parser.cc"
    break;

  case 6: // block: "also" one_or_more_statements "end" "also"
#line 122 "parser.yy"
                                              { yylhs.value.as < Block > () = Block::MainBlock(yystack_[2].value.as < Statements > ()); }
#line 1432 "parser.cc"
    break;

  case 7: // block: "when" "start" one_or_more_statements "end" "when"
#line 123 "parser.yy"
                                                     { yylhs.value.as < Block > () = Block::WhenBlock(yystack_[3].value.as < std::string > (), yystack_[2].value.as < Statements > ()); }
#line 1438 "parser.cc"
    break;

  case 8: // main_block: one_or_more_statements
#line 126 "parser.yy"
                                  { yylhs.value.as < Block > () = Block::MainBlock(yystack_[0].value.as < Statements > ()); }
#line 1444 "parser.cc"
    break;

  case 9: // zero_or_more_statements: %empty
#line 129 "parser.yy"
                                     {}
#line 1450 "parser.cc"
    break;

  case 10: // zero_or_more_statements: zero_or_more_statements statement
#line 130 "parser.yy"
                                     { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1456 "parser.cc"
    break;

  case 11: // one_or_more_statements: statement
#line 133 "parser.yy"
                                   { yylhs.value.as < Statements > ()  = Statements::FirstStatement(yystack_[0].value.as < Line > ()); }
#line 1462 "parser.cc"
    break;

  case 12: // one_or_more_statements: one_or_more_statements statement
#line 134 "parser.yy"
                                   { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1468 "parser.cc"
    break;

  case 13: // statement: array_assignment
#line 137 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1474 "parser.cc"
    break;

  case 14: // statement: assignment
#line 138 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1480 "parser.cc"
    break;

  case 15: // statement: continue_statement
#line 139 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1486 "parser.cc"
    break;

  case 16: // statement: exit_statement
#line 140 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1492 "parser.cc"
    break;

  case 17: // statement: for_statement
#line 141 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1498 "parser.cc"
    break;

  case 18: // statement: if_statement
#line 142 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1504 "parser.cc"
    break;

  case 19: // statement: wait_statement
#line 143 "parser.yy"
                       { yylhs.value.as < Line > () = yystack_[0].value.as < Line > (); }
#line 1510 "parser.cc"
    break;

  case 20: // array_assignment: "identifier" "[" exp "]" "=" exp
#line 147 "parser.yy"
                                    { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1516 "parser.cc"
    break;

  case 21: // array_assignment: "identifier" "[" exp "]" "=" "{" expression_list "}"
#line 148 "parser.yy"
                                                        { yylhs.value.as < Line > () = Line::ArrayAssignment(yystack_[7].value.as < std::string > (), yystack_[5].value.as < Expression > (), yystack_[1].value.as < ExpressionList > (), &drv); }
#line 1522 "parser.cc"
    break;

  case 22: // assignment: "identifier" "=" exp
#line 151 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1528 "parser.cc"
    break;

  case 23: // assignment: "in_port" "=" exp
#line 152 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1534 "parser.cc"
    break;

  case 24: // assignment: "out_port" "=" exp
#line 153 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < Expression > (), &drv); }
#line 1540 "parser.cc"
    break;

  case 25: // continue_statement: "continue" "for"
#line 156 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1546 "parser.cc"
    break;

  case 26: // continue_statement: "continue" "all"
#line 157 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1552 "parser.cc"
    break;

  case 27: // exit_statement: "exit" "for"
#line 160 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1558 "parser.cc"
    break;

  case 28: // exit_statement: "exit" "all"
#line 161 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1564 "parser.cc"
    break;

  case 29: // for_statement: "for" assignment "to" exp zero_or_more_statements "next"
#line 164 "parser.yy"
                                                            { yylhs.value.as < Line > () = Line::ForNext(yystack_[4].value.as < Line > (), yystack_[2].value.as < Expression > (), drv.factory.Number(1.0), yystack_[1].value.as < Statements > (), &drv); }
#line 1570 "parser.cc"
    break;

  case 30: // for_statement: "for" assignment "to" exp "step" exp zero_or_more_statements "next"
#line 165 "parser.yy"
                                                                      { yylhs.value.as < Line > () = Line::ForNext(yystack_[6].value.as < Line > (), yystack_[4].value.as < Expression > (), yystack_[2].value.as < Expression > (), yystack_[1].value.as < Statements > (), &drv); }
#line 1576 "parser.cc"
    break;

  case 31: // elseif_group: %empty
#line 168 "parser.yy"
                                  {}
#line 1582 "parser.cc"
    break;

  case 32: // elseif_group: elseif_group elseif_clause
#line 169 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1588 "parser.cc"
    break;

  case 33: // elseif_clause: "elseif" exp "then" zero_or_more_statements
#line 172 "parser.yy"
                                               { yylhs.value.as < Line > () = Line::ElseIf(yystack_[2].value.as < Expression > (), yystack_[0].value.as < Statements > ()); }
#line 1594 "parser.cc"
    break;

  case 34: // if_statement: "if" exp "then" zero_or_more_statements elseif_group "end" "if"
#line 175 "parser.yy"
                                                                        { yylhs.value.as < Line > () = Line::IfThen(yystack_[5].value.as < Expression > (), yystack_[3].value.as < Statements > (), yystack_[2].value.as < Statements > ()); }
#line 1600 "parser.cc"
    break;

  case 35: // if_statement: "if" exp "then" zero_or_more_statements elseif_group "else" zero_or_more_statements "end" "if"
#line 176 "parser.yy"
                                                                                                  { yylhs.value.as < Line > () = Line::IfThenElse(yystack_[7].value.as < Expression > (), yystack_[5].value.as < Statements > (), yystack_[2].value.as < Statements > (), yystack_[4].value.as < Statements > ()); }
#line 1606 "parser.cc"
    break;

  case 36: // wait_statement: "wait" exp
#line 179 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Wait(yystack_[0].value.as < Expression > ()); }
#line 1612 "parser.cc"
    break;

  case 37: // expression_list: exp
#line 189 "parser.yy"
                              { yylhs.value.as < ExpressionList > () = ExpressionList(yystack_[0].value.as < Expression > ()); }
#line 1618 "parser.cc"
    break;

  case 38: // expression_list: expression_list "," exp
#line 190 "parser.yy"
                              { yylhs.value.as < ExpressionList > () = yystack_[2].value.as < ExpressionList > ().add(yystack_[0].value.as < Expression > ()); }
#line 1624 "parser.cc"
    break;

  case 39: // exp: "number"
#line 193 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Number((float) yystack_[0].value.as < float > ()); }
#line 1630 "parser.cc"
    break;

  case 40: // exp: "note"
#line 194 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Note(yystack_[0].value.as < std::string > ()); }
#line 1636 "parser.cc"
    break;

  case 41: // exp: "-" "number"
#line 195 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.Number(-1 * (float) yystack_[0].value.as < float > ());}
#line 1642 "parser.cc"
    break;

  case 42: // exp: "not" exp
#line 196 "parser.yy"
                      { yylhs.value.as < Expression > () = drv.factory.Not(yystack_[0].value.as < Expression > ());}
#line 1648 "parser.cc"
    break;

  case 43: // exp: "in_port"
#line 197 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1654 "parser.cc"
    break;

  case 44: // exp: "out_port"
#line 198 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1660 "parser.cc"
    break;

  case 45: // exp: "identifier"
#line 199 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1666 "parser.cc"
    break;

  case 46: // exp: "identifier" "[" exp "]"
#line 200 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.ArrayVariable(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > (), &drv); }
#line 1672 "parser.cc"
    break;

  case 47: // exp: exp "+" exp
#line 201 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1678 "parser.cc"
    break;

  case 48: // exp: exp "-" exp
#line 202 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1684 "parser.cc"
    break;

  case 49: // exp: exp "*" exp
#line 203 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1690 "parser.cc"
    break;

  case 50: // exp: exp "/" exp
#line 204 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1696 "parser.cc"
    break;

  case 51: // exp: exp "comparison" exp
#line 205 "parser.yy"
                       { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1702 "parser.cc"
    break;

  case 52: // exp: exp "and" exp
#line 206 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1708 "parser.cc"
    break;

  case 53: // exp: exp "or" exp
#line 207 "parser.yy"
                { yylhs.value.as < Expression > () = drv.factory.CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1714 "parser.cc"
    break;

  case 54: // exp: "zeroargfunc" "(" ")"
#line 208 "parser.yy"
                        { yylhs.value.as < Expression > () = drv.factory.ZeroArgFunc(yystack_[2].value.as < std::string > ()); }
#line 1720 "parser.cc"
    break;

  case 55: // exp: "oneportfunc" "(" "in_port" ")"
#line 209 "parser.yy"
                                  {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 1726 "parser.cc"
    break;

  case 56: // exp: "oneportfunc" "(" "out_port" ")"
#line 210 "parser.yy"
                                   {yylhs.value.as < Expression > () = drv.factory.OnePortFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < std::string > (), &drv);}
#line 1732 "parser.cc"
    break;

  case 57: // exp: "oneargfunc" "(" exp ")"
#line 211 "parser.yy"
                           { yylhs.value.as < Expression > () = drv.factory.OneArgFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > ()); }
#line 1738 "parser.cc"
    break;

  case 58: // exp: "twoargfunc" "(" exp "," exp ")"
#line 212 "parser.yy"
                                   { yylhs.value.as < Expression > () = drv.factory.TwoArgFunc(yystack_[5].value.as < std::string > (), yystack_[3].value.as < Expression > (), yystack_[1].value.as < Expression > ()); }
#line 1744 "parser.cc"
    break;

  case 59: // exp: "(" exp ")"
#line 213 "parser.yy"
                { yylhs.value.as < Expression > () = yystack_[1].value.as < Expression > (); }
#line 1750 "parser.cc"
    break;


#line 1754 "parser.cc"

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

  const char *
  parser::symbol_name (symbol_kind_type yysymbol)
  {
    static const char *const yy_sname[] =
    {
    "end of file", "error", "invalid token", "abs", "all", "also", "and",
  "=", "ceiling", "continue", "else", "elseif", "end", "exit", "floor",
  "for", "if", "max", "min", "next", "not", "or", "pow", "sample_rate",
  "sign", "sin", "start", "step", "then", "to", "wait", "when", "-", "+",
  "*", "/", "(", ")", "{", "}", "[", "]", ",", "identifier", "number",
  "note", "in_port", "out_port", "zeroargfunc", "oneargfunc",
  "oneportfunc", "twoargfunc", "comparison", "NEG", "$accept", "program",
  "blocks", "block", "main_block", "zero_or_more_statements",
  "one_or_more_statements", "statement", "array_assignment", "assignment",
  "continue_statement", "exit_statement", "for_statement", "elseif_group",
  "elseif_clause", "if_statement", "wait_statement", "expression_list",
  "exp", YY_NULLPTR
    };
    return yy_sname[yysymbol];
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


  const signed char parser::yypact_ninf_ = -78;

  const signed char parser::yytable_ninf_ = -1;

  const short
  parser::yypact_[] =
  {
     209,   282,     1,     9,   -28,   314,   314,    13,    -3,    39,
      47,    14,     7,   -78,   -78,   282,   -78,   -78,   -78,   -78,
     -78,   -78,   -78,   -78,   219,   -78,   -78,   -78,   -78,    53,
      23,   314,   -13,   314,    29,   -78,   -78,   -78,   -78,    34,
      36,    40,    55,    45,   171,   282,   314,   314,   314,   314,
     -78,   -78,   -78,   -78,    70,   314,   -78,   -78,    50,   314,
      44,   314,    -2,   314,   314,   314,   -78,   314,   314,   314,
     314,   314,   229,   171,    82,   171,   171,   -78,    92,   -78,
     114,   -78,   124,    49,    52,   130,   -24,   178,   282,    33,
      33,   -78,   -78,   -12,    61,    86,   314,   238,   -78,   -78,
     -78,   -78,   314,   -78,    24,   -78,   294,   171,   -78,   146,
     -78,   314,    78,   -78,   314,   171,   258,   -78,   277,   167,
     -78,   -10,   171,   -78,    79,   -78,   -78,   314,   -78,   282,
     171
  };

  const signed char
  parser::yydefact_[] =
  {
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     3,     8,    11,    13,    14,    15,
      16,    17,    18,    19,     0,    26,    25,    28,    27,     0,
       0,     0,     0,     0,    45,    39,    40,    43,    44,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       1,     2,     5,    12,     0,     0,    42,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     9,     0,     0,     0,
       0,     0,     0,    22,     0,    23,    24,     6,     9,    59,
       0,    54,     0,     0,     0,     0,    52,    53,    31,    48,
      47,    49,    50,    51,     0,     0,     0,     0,    46,    57,
      55,    56,     0,    10,     0,     7,     0,     9,    29,     0,
       9,     0,     0,    32,     0,    20,     0,    58,     0,     0,
      34,     0,    37,    30,     0,     9,    21,     0,    35,    33,
      38
  };

  const signed char
  parser::yypgoto_[] =
  {
     -78,   -78,   -78,    87,   -78,   -77,     5,     2,   -78,    97,
     -78,   -78,   -78,   -78,   -78,   -78,   -78,   -78,    -6
  };

  const signed char
  parser::yydefgoto_[] =
  {
       0,    11,    12,    13,    14,    88,    15,   103,    17,    18,
      19,    20,    21,   104,   113,    22,    23,   121,    43
  };

  const unsigned char
  parser::yytable_[] =
  {
      44,    97,    16,    16,    46,    25,    24,    51,    67,    68,
      69,    70,     1,    27,    50,    29,    26,    53,     9,    10,
      67,    68,    69,    70,    28,    56,    53,    58,    71,   126,
     116,    57,   127,   118,   110,   111,   112,    47,     7,    45,
      73,    74,    75,    76,    83,    84,    48,    16,   129,    78,
      72,    64,    55,    80,    49,    82,    64,    85,    86,    87,
      46,    89,    90,    91,    92,    93,    65,    69,    70,    59,
      60,    65,    61,    66,    53,    77,    62,    67,    68,    69,
      70,    81,    67,    68,    69,    70,   100,    79,    64,   101,
     107,    63,   105,   106,   120,   128,   109,    71,    64,    52,
     115,    30,    71,    65,     0,   119,     0,     0,   122,     0,
       0,     0,     0,    65,    67,    68,    69,    70,     0,    96,
      64,   130,     0,    95,    67,    68,    69,    70,     0,     0,
      64,     0,     0,     0,    71,    65,    64,     0,     0,     0,
       0,     0,     0,     0,    71,    65,    67,    68,    69,    70,
       0,    65,    64,     0,     0,    98,    67,    68,    69,    70,
       0,    99,    67,    68,    69,    70,    71,    65,     0,     0,
       0,     0,   102,    64,     0,     0,    71,    64,    67,    68,
      69,    70,    71,   117,    64,     0,     0,     0,    65,     0,
       0,     0,    65,     0,     0,   125,     0,     0,    71,    67,
      68,    69,    70,    67,    68,    69,    70,     0,     0,     0,
      67,    68,    69,    70,     1,     0,     0,     0,     2,    71,
       0,     0,     3,    71,     4,     5,     0,     0,     2,     0,
      71,    54,     3,     0,     4,     5,     0,     0,     2,     6,
       7,    94,     3,     0,     4,     5,     0,     2,     0,     6,
       0,     3,     8,     4,     5,     9,    10,   108,     0,     6,
       0,     0,     8,     0,     0,     9,    10,     2,     6,     0,
       0,     3,     8,     4,     5,     9,    10,   123,     0,     0,
       0,     8,     0,     0,     9,    10,     2,     0,     6,   124,
       3,     2,     4,     5,     0,     3,     0,     4,     5,     0,
       0,     8,     0,     0,     9,    10,     0,     6,     0,     0,
       0,     0,     6,     0,    31,     0,     0,     0,     0,     0,
       8,     0,     0,     9,    10,     8,    32,     0,     9,    10,
      33,     0,   114,     0,    31,     0,     0,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    32,     0,     0,     0,
      33,     0,     0,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,    41,    42
  };

  const signed char
  parser::yycheck_[] =
  {
       6,    78,     0,     1,     7,     4,     1,     0,    32,    33,
      34,    35,     5,     4,     0,    43,    15,    15,    46,    47,
      32,    33,    34,    35,    15,    31,    24,    33,    52,    39,
     107,    44,    42,   110,    10,    11,    12,    40,    31,    26,
      46,    47,    48,    49,    46,    47,     7,    45,   125,    55,
      45,     6,    29,    59,     7,    61,     6,    63,    64,    65,
       7,    67,    68,    69,    70,    71,    21,    34,    35,    40,
      36,    21,    36,    28,    72,     5,    36,    32,    33,    34,
      35,    37,    32,    33,    34,    35,    37,    37,     6,    37,
      96,    36,    31,     7,    16,    16,   102,    52,     6,    12,
     106,     4,    52,    21,    -1,   111,    -1,    -1,   114,    -1,
      -1,    -1,    -1,    21,    32,    33,    34,    35,    -1,    27,
       6,   127,    -1,    41,    32,    33,    34,    35,    -1,    -1,
       6,    -1,    -1,    -1,    52,    21,     6,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    21,    32,    33,    34,    35,
      -1,    21,     6,    -1,    -1,    41,    32,    33,    34,    35,
      -1,    37,    32,    33,    34,    35,    52,    21,    -1,    -1,
      -1,    -1,    42,     6,    -1,    -1,    52,     6,    32,    33,
      34,    35,    52,    37,     6,    -1,    -1,    -1,    21,    -1,
      -1,    -1,    21,    -1,    -1,    28,    -1,    -1,    52,    32,
      33,    34,    35,    32,    33,    34,    35,    -1,    -1,    -1,
      32,    33,    34,    35,     5,    -1,    -1,    -1,     9,    52,
      -1,    -1,    13,    52,    15,    16,    -1,    -1,     9,    -1,
      52,    12,    13,    -1,    15,    16,    -1,    -1,     9,    30,
      31,    12,    13,    -1,    15,    16,    -1,     9,    -1,    30,
      -1,    13,    43,    15,    16,    46,    47,    19,    -1,    30,
      -1,    -1,    43,    -1,    -1,    46,    47,     9,    30,    -1,
      -1,    13,    43,    15,    16,    46,    47,    19,    -1,    -1,
      -1,    43,    -1,    -1,    46,    47,     9,    -1,    30,    12,
      13,     9,    15,    16,    -1,    13,    -1,    15,    16,    -1,
      -1,    43,    -1,    -1,    46,    47,    -1,    30,    -1,    -1,
      -1,    -1,    30,    -1,    20,    -1,    -1,    -1,    -1,    -1,
      43,    -1,    -1,    46,    47,    43,    32,    -1,    46,    47,
      36,    -1,    38,    -1,    20,    -1,    -1,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    32,    -1,    -1,    -1,
      36,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,
      46,    47,    48,    49,    50,    51
  };

  const signed char
  parser::yystos_[] =
  {
       0,     5,     9,    13,    15,    16,    30,    31,    43,    46,
      47,    55,    56,    57,    58,    60,    61,    62,    63,    64,
      65,    66,    69,    70,    60,     4,    15,     4,    15,    43,
      63,    20,    32,    36,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    72,    72,    26,     7,    40,     7,     7,
       0,     0,    57,    61,    12,    29,    72,    44,    72,    40,
      36,    36,    36,    36,     6,    21,    28,    32,    33,    34,
      35,    52,    60,    72,    72,    72,    72,     5,    72,    37,
      72,    37,    72,    46,    47,    72,    72,    72,    59,    72,
      72,    72,    72,    72,    12,    41,    27,    59,    41,    37,
      37,    37,    42,    61,    67,    31,     7,    72,    19,    72,
      10,    11,    12,    68,    38,    72,    59,    37,    59,    72,
      16,    71,    72,    19,    12,    28,    39,    42,    16,    59,
      72
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    54,    55,    56,    56,    56,    57,    57,    58,    59,
      59,    60,    60,    61,    61,    61,    61,    61,    61,    61,
      62,    62,    63,    63,    63,    64,    64,    65,    65,    66,
      66,    67,    67,    68,    69,    69,    70,    71,    71,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     2,     1,     1,     2,     4,     5,     1,     0,
       2,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       6,     8,     3,     3,     3,     2,     2,     2,     2,     6,
       8,     0,     2,     4,     7,     9,     2,     1,     3,     1,
       1,     2,     2,     1,     1,     1,     4,     3,     3,     3,
       3,     3,     3,     3,     3,     4,     4,     4,     6,     3
  };




#if YYDEBUG
  const unsigned char
  parser::yyrline_[] =
  {
       0,   114,   114,   117,   118,   119,   122,   123,   126,   129,
     130,   133,   134,   137,   138,   139,   140,   141,   142,   143,
     147,   148,   151,   152,   153,   156,   157,   160,   161,   164,
     165,   168,   169,   172,   175,   176,   179,   189,   190,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213
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
#line 2443 "parser.cc"

#line 214 "parser.yy"


void
yy::parser::error (const location_type& l, const std::string& m)
{
  drv.errors.push_back(Error(l.begin.line, l.begin.column, m));
}
