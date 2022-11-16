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
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_exp: // exp
        value.YY_MOVE_OR_COPY< Expression > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_wait_statement: // wait_statement
        value.YY_MOVE_OR_COPY< Line > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_statements: // statements
        value.YY_MOVE_OR_COPY< Statements > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.YY_MOVE_OR_COPY< float > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_ALL: // "all"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_CLAMP: // "clamp"
      case symbol_kind::S_CONTINUE: // "continue"
      case symbol_kind::S_ELSE: // "else"
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
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
      case symbol_kind::S_WAIT: // "wait"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_EQUALS: // "=="
      case symbol_kind::S_NOT_EQUALS: // "!="
      case symbol_kind::S_LT: // "<"
      case symbol_kind::S_LTE: // "<="
      case symbol_kind::S_GT: // ">"
      case symbol_kind::S_GTE: // ">="
      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
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
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_exp: // exp
        value.move< Expression > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_wait_statement: // wait_statement
        value.move< Line > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_statements: // statements
        value.move< Statements > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.move< float > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_ALL: // "all"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_CLAMP: // "clamp"
      case symbol_kind::S_CONTINUE: // "continue"
      case symbol_kind::S_ELSE: // "else"
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
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
      case symbol_kind::S_WAIT: // "wait"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_EQUALS: // "=="
      case symbol_kind::S_NOT_EQUALS: // "!="
      case symbol_kind::S_LT: // "<"
      case symbol_kind::S_LTE: // "<="
      case symbol_kind::S_GT: // ">"
      case symbol_kind::S_GTE: // ">="
      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
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
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_exp: // exp
        value.copy< Expression > (that.value);
        break;

      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_wait_statement: // wait_statement
        value.copy< Line > (that.value);
        break;

      case symbol_kind::S_statements: // statements
        value.copy< Statements > (that.value);
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.copy< float > (that.value);
        break;

      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_ALL: // "all"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_CLAMP: // "clamp"
      case symbol_kind::S_CONTINUE: // "continue"
      case symbol_kind::S_ELSE: // "else"
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
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
      case symbol_kind::S_WAIT: // "wait"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_EQUALS: // "=="
      case symbol_kind::S_NOT_EQUALS: // "!="
      case symbol_kind::S_LT: // "<"
      case symbol_kind::S_LTE: // "<="
      case symbol_kind::S_GT: // ">"
      case symbol_kind::S_GTE: // ">="
      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
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
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_exp: // exp
        value.move< Expression > (that.value);
        break;

      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_wait_statement: // wait_statement
        value.move< Line > (that.value);
        break;

      case symbol_kind::S_statements: // statements
        value.move< Statements > (that.value);
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.move< float > (that.value);
        break;

      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_ALL: // "all"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_CLAMP: // "clamp"
      case symbol_kind::S_CONTINUE: // "continue"
      case symbol_kind::S_ELSE: // "else"
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
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
      case symbol_kind::S_WAIT: // "wait"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_EQUALS: // "=="
      case symbol_kind::S_NOT_EQUALS: // "!="
      case symbol_kind::S_LT: // "<"
      case symbol_kind::S_LTE: // "<="
      case symbol_kind::S_GT: // ">"
      case symbol_kind::S_GTE: // ">="
      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
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
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 546 "parser.cc"
        break;

      case symbol_kind::S_ALL: // "all"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 552 "parser.cc"
        break;

      case symbol_kind::S_AND: // "and"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 558 "parser.cc"
        break;

      case symbol_kind::S_ASSIGN: // "="
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 564 "parser.cc"
        break;

      case symbol_kind::S_CEILING: // "ceiling"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 570 "parser.cc"
        break;

      case symbol_kind::S_CLAMP: // "clamp"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 576 "parser.cc"
        break;

      case symbol_kind::S_CONTINUE: // "continue"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 582 "parser.cc"
        break;

      case symbol_kind::S_ELSE: // "else"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 588 "parser.cc"
        break;

      case symbol_kind::S_END: // "end"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 594 "parser.cc"
        break;

      case symbol_kind::S_EXIT: // "exit"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 600 "parser.cc"
        break;

      case symbol_kind::S_FLOOR: // "floor"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 606 "parser.cc"
        break;

      case symbol_kind::S_FOR: // "for"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 612 "parser.cc"
        break;

      case symbol_kind::S_IF: // "if"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 618 "parser.cc"
        break;

      case symbol_kind::S_MAX: // "max"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 624 "parser.cc"
        break;

      case symbol_kind::S_MIN: // "min"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 630 "parser.cc"
        break;

      case symbol_kind::S_NEXT: // "next"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 636 "parser.cc"
        break;

      case symbol_kind::S_NOT: // "not"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 642 "parser.cc"
        break;

      case symbol_kind::S_OR: // "or"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 648 "parser.cc"
        break;

      case symbol_kind::S_POW: // "pow"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 654 "parser.cc"
        break;

      case symbol_kind::S_SIGN: // "sign"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 660 "parser.cc"
        break;

      case symbol_kind::S_SIN: // "sin"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 666 "parser.cc"
        break;

      case symbol_kind::S_STEP: // "step"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 672 "parser.cc"
        break;

      case symbol_kind::S_THEN: // "then"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 678 "parser.cc"
        break;

      case symbol_kind::S_TO: // "to"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 684 "parser.cc"
        break;

      case symbol_kind::S_WAIT: // "wait"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 690 "parser.cc"
        break;

      case symbol_kind::S_MINUS: // "-"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 696 "parser.cc"
        break;

      case symbol_kind::S_PLUS: // "+"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 702 "parser.cc"
        break;

      case symbol_kind::S_STAR: // "*"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 708 "parser.cc"
        break;

      case symbol_kind::S_SLASH: // "/"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 714 "parser.cc"
        break;

      case symbol_kind::S_LPAREN: // "("
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 720 "parser.cc"
        break;

      case symbol_kind::S_RPAREN: // ")"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 726 "parser.cc"
        break;

      case symbol_kind::S_EQUALS: // "=="
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 732 "parser.cc"
        break;

      case symbol_kind::S_NOT_EQUALS: // "!="
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 738 "parser.cc"
        break;

      case symbol_kind::S_LT: // "<"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 744 "parser.cc"
        break;

      case symbol_kind::S_LTE: // "<="
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 750 "parser.cc"
        break;

      case symbol_kind::S_GT: // ">"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 756 "parser.cc"
        break;

      case symbol_kind::S_GTE: // ">="
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 762 "parser.cc"
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < Expression > (); }
#line 768 "parser.cc"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < float > (); }
#line 774 "parser.cc"
        break;

      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 780 "parser.cc"
        break;

      case symbol_kind::S_statements: // statements
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < Statements > (); }
#line 786 "parser.cc"
        break;

      case symbol_kind::S_assignment: // assignment
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 792 "parser.cc"
        break;

      case symbol_kind::S_continue_statement: // continue_statement
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 798 "parser.cc"
        break;

      case symbol_kind::S_exit_statement: // exit_statement
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 804 "parser.cc"
        break;

      case symbol_kind::S_for_statement: // for_statement
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 810 "parser.cc"
        break;

      case symbol_kind::S_if_statement: // if_statement
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 816 "parser.cc"
        break;

      case symbol_kind::S_wait_statement: // wait_statement
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < Line > (); }
#line 822 "parser.cc"
        break;

      case symbol_kind::S_exp: // exp
#line 89 "parser.yy"
                 { yyo << yysym.value.template as < Expression > (); }
#line 828 "parser.cc"
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
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_exp: // exp
        yylhs.value.emplace< Expression > ();
        break;

      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_continue_statement: // continue_statement
      case symbol_kind::S_exit_statement: // exit_statement
      case symbol_kind::S_for_statement: // for_statement
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_wait_statement: // wait_statement
        yylhs.value.emplace< Line > ();
        break;

      case symbol_kind::S_statements: // statements
        yylhs.value.emplace< Statements > ();
        break;

      case symbol_kind::S_NUMBER: // "number"
        yylhs.value.emplace< float > ();
        break;

      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_ALL: // "all"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_CLAMP: // "clamp"
      case symbol_kind::S_CONTINUE: // "continue"
      case symbol_kind::S_ELSE: // "else"
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
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_STEP: // "step"
      case symbol_kind::S_THEN: // "then"
      case symbol_kind::S_TO: // "to"
      case symbol_kind::S_WAIT: // "wait"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_EQUALS: // "=="
      case symbol_kind::S_NOT_EQUALS: // "!="
      case symbol_kind::S_LT: // "<"
      case symbol_kind::S_LTE: // "<="
      case symbol_kind::S_GT: // ">"
      case symbol_kind::S_GTE: // ">="
      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
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
  case 2: // program: statements $end
#line 95 "parser.yy"
                   { drv.lines = yystack_[1].value.as < Statements > ().lines; }
#line 1154 "parser.cc"
    break;

  case 3: // statements: %empty
#line 98 "parser.yy"
                             {}
#line 1160 "parser.cc"
    break;

  case 4: // statements: statements assignment
#line 99 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1166 "parser.cc"
    break;

  case 5: // statements: statements continue_statement
#line 100 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1172 "parser.cc"
    break;

  case 6: // statements: statements exit_statement
#line 101 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1178 "parser.cc"
    break;

  case 7: // statements: statements for_statement
#line 102 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1184 "parser.cc"
    break;

  case 8: // statements: statements if_statement
#line 103 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1190 "parser.cc"
    break;

  case 9: // statements: statements wait_statement
#line 104 "parser.yy"
                                  { yylhs.value.as < Statements > () = yystack_[1].value.as < Statements > ().add(yystack_[0].value.as < Line > ()); }
#line 1196 "parser.cc"
    break;

  case 10: // assignment: "identifier" "=" exp
#line 107 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Assignment(yystack_[2].value.as < Expression > (), yystack_[0].value.as < Expression > ()); }
#line 1202 "parser.cc"
    break;

  case 11: // continue_statement: "continue" "for"
#line 110 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1208 "parser.cc"
    break;

  case 12: // continue_statement: "continue" "all"
#line 111 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Continue(yystack_[0].value.as < std::string > ()); }
#line 1214 "parser.cc"
    break;

  case 13: // exit_statement: "exit" "for"
#line 114 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1220 "parser.cc"
    break;

  case 14: // exit_statement: "exit" "all"
#line 115 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Exit(yystack_[0].value.as < std::string > ()); }
#line 1226 "parser.cc"
    break;

  case 15: // for_statement: "for" assignment "to" exp statements "next"
#line 118 "parser.yy"
                                               { yylhs.value.as < Line > () = Line::ForNext(yystack_[4].value.as < Line > (), yystack_[2].value.as < Expression > (), Expression::Number(1.0), yystack_[1].value.as < Statements > ()); }
#line 1232 "parser.cc"
    break;

  case 16: // for_statement: "for" assignment "to" exp "step" exp statements "next"
#line 119 "parser.yy"
                                                         { yylhs.value.as < Line > () = Line::ForNext(yystack_[6].value.as < Line > (), yystack_[4].value.as < Expression > (), yystack_[2].value.as < Expression > (), yystack_[1].value.as < Statements > ()); }
#line 1238 "parser.cc"
    break;

  case 17: // if_statement: "if" exp "then" statements "end" "if"
#line 122 "parser.yy"
                                                           { yylhs.value.as < Line > () = Line::IfThen(yystack_[4].value.as < Expression > (), yystack_[2].value.as < Statements > ()); }
#line 1244 "parser.cc"
    break;

  case 18: // if_statement: "if" exp "then" statements "else" statements "end" "if"
#line 123 "parser.yy"
                                                           { yylhs.value.as < Line > () = Line::IfThenElse(yystack_[6].value.as < Expression > (), yystack_[4].value.as < Statements > (), yystack_[2].value.as < Statements > ()); }
#line 1250 "parser.cc"
    break;

  case 19: // wait_statement: "wait" exp
#line 126 "parser.yy"
                        { yylhs.value.as < Line > () = Line::Wait(yystack_[0].value.as < Expression > ()); }
#line 1256 "parser.cc"
    break;

  case 20: // exp: "number"
#line 136 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::Number((float) yystack_[0].value.as < float > ()); }
#line 1262 "parser.cc"
    break;

  case 21: // exp: "-" "number"
#line 137 "parser.yy"
                           { yylhs.value.as < Expression > () = Expression::Number(-1 * (float) yystack_[0].value.as < float > ());}
#line 1268 "parser.cc"
    break;

  case 22: // exp: "not" exp
#line 138 "parser.yy"
                      { yylhs.value.as < Expression > () = Expression::Not(yystack_[0].value.as < Expression > ());}
#line 1274 "parser.cc"
    break;

  case 23: // exp: "identifier"
#line 139 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::Variable(yystack_[0].value.as < Expression > ()); }
#line 1280 "parser.cc"
    break;

  case 24: // exp: exp "+" exp
#line 140 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1286 "parser.cc"
    break;

  case 25: // exp: exp "-" exp
#line 141 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1292 "parser.cc"
    break;

  case 26: // exp: exp "*" exp
#line 142 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1298 "parser.cc"
    break;

  case 27: // exp: exp "/" exp
#line 143 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1304 "parser.cc"
    break;

  case 28: // exp: exp "<" exp
#line 144 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1310 "parser.cc"
    break;

  case 29: // exp: exp "<=" exp
#line 145 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1316 "parser.cc"
    break;

  case 30: // exp: exp ">" exp
#line 146 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1322 "parser.cc"
    break;

  case 31: // exp: exp ">=" exp
#line 147 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1328 "parser.cc"
    break;

  case 32: // exp: exp "==" exp
#line 148 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1334 "parser.cc"
    break;

  case 33: // exp: exp "!=" exp
#line 149 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1340 "parser.cc"
    break;

  case 34: // exp: exp "and" exp
#line 150 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1346 "parser.cc"
    break;

  case 35: // exp: exp "or" exp
#line 151 "parser.yy"
                { yylhs.value.as < Expression > () = Expression::CreateBinOp(yystack_[2].value.as < Expression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < Expression > ()); }
#line 1352 "parser.cc"
    break;

  case 36: // exp: "oneargfunc" "(" exp ")"
#line 152 "parser.yy"
                           { yylhs.value.as < Expression > () = Expression::OneArgFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < Expression > ()); }
#line 1358 "parser.cc"
    break;

  case 37: // exp: "(" exp ")"
#line 153 "parser.yy"
                { yylhs.value.as < Expression > () = yystack_[1].value.as < Expression > (); }
#line 1364 "parser.cc"
    break;


#line 1368 "parser.cc"

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
    "end of file", "error", "invalid token", "abs", "all", "and", "=",
  "ceiling", "clamp", "continue", "else", "end", "exit", "floor", "for",
  "if", "max", "min", "next", "not", "or", "pow", "sign", "sin", "step",
  "then", "to", "wait", "-", "+", "*", "/", "(", ")", "==", "!=", "<",
  "<=", ">", ">=", "identifier", "number", "oneargfunc", "NEG", "$accept",
  "program", "statements", "assignment", "continue_statement",
  "exit_statement", "for_statement", "if_statement", "wait_statement",
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


  const signed char parser::yypact_ninf_ = -34;

  const signed char parser::yytable_ninf_ = -1;

  const short
  parser::yypact_[] =
  {
     -34,     1,     2,   -34,   -34,     4,     5,   -33,   171,   171,
      14,   -34,   -34,   -34,   -34,   -34,   -34,   -34,   -34,   -34,
     -34,   -16,   171,   -18,   171,   -34,   -34,    -7,    35,   113,
     171,   171,   -34,   -34,    47,   171,   171,   171,   -34,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   113,
      74,   -34,    86,   190,   125,   156,    16,    16,   -34,   -34,
     -25,   -25,   -25,   -25,   -25,   -25,   171,   160,   -34,   -34,
      26,   113,   -34,   170,   -34,   177,    29,   -34,   -34
  };

  const signed char
  parser::yydefact_[] =
  {
       3,     0,     0,     1,     2,     0,     0,     0,     0,     0,
       0,     4,     5,     6,     7,     8,     9,    12,    11,    14,
      13,     0,     0,     0,     0,    23,    20,     0,     0,    19,
       0,     0,    22,    21,     0,     0,     0,     0,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    10,
       3,    37,     0,    34,    35,     0,    25,    24,    26,    27,
      32,    33,    28,    29,    30,    31,     0,     0,    36,     3,
       0,     3,    15,     0,    17,     0,     0,    16,    18
  };

  const signed char
  parser::yypgoto_[] =
  {
     -34,   -34,   -26,    41,   -34,   -34,   -34,   -34,   -34,    -9
  };

  const signed char
  parser::yydefgoto_[] =
  {
       0,     1,     2,    11,    12,    13,    14,    15,    16,    28
  };

  const signed char
  parser::yytable_[] =
  {
      29,     3,     4,    39,    40,    41,    42,    10,    17,    19,
      31,     5,    55,    32,     6,    34,     7,     8,    18,    20,
      30,    49,    50,    33,    67,    35,    52,    53,    54,     9,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      36,    74,    10,    73,    78,    75,    41,    42,    21,     0,
       0,     0,    36,     0,     0,    37,     0,    71,     0,     0,
      38,     0,     0,    39,    40,    41,    42,    37,     0,    43,
      44,    45,    46,    47,    48,    39,    40,    41,    42,    36,
      51,    43,    44,    45,    46,    47,    48,     0,     0,     0,
       0,    36,     0,     0,    37,     0,     0,     0,    66,     0,
       0,     0,    39,    40,    41,    42,    37,     0,    43,    44,
      45,    46,    47,    48,    39,    40,    41,    42,    36,    68,
      43,    44,    45,    46,    47,    48,     0,     0,     0,     0,
      36,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,    39,    40,    41,    42,     0,     0,    43,    44,    45,
      46,    47,    48,    39,    40,    41,    42,     0,     0,    43,
      44,    45,    46,    47,    48,     5,    69,    70,     6,     5,
       7,     8,     6,     0,     7,     8,     0,     0,    72,     5,
       0,    76,     6,     9,     7,     8,     5,     9,     0,     6,
      22,     7,     8,     0,     0,    77,    10,     9,     0,    23,
      10,     0,     0,    24,     9,     0,     0,     0,     0,     0,
      10,    25,    26,    27,     0,     0,     0,    10,    39,    40,
      41,    42,     0,     0,    43,    44,    45,    46,    47,    48
  };

  const signed char
  parser::yycheck_[] =
  {
       9,     0,     0,    28,    29,    30,    31,    40,     4,     4,
      26,     9,    38,    22,    12,    24,    14,    15,    14,    14,
       6,    30,    31,    41,    50,    32,    35,    36,    37,    27,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
       5,    15,    40,    69,    15,    71,    30,    31,     7,    -1,
      -1,    -1,     5,    -1,    -1,    20,    -1,    66,    -1,    -1,
      25,    -1,    -1,    28,    29,    30,    31,    20,    -1,    34,
      35,    36,    37,    38,    39,    28,    29,    30,    31,     5,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,     5,    -1,    -1,    20,    -1,    -1,    -1,    24,    -1,
      -1,    -1,    28,    29,    30,    31,    20,    -1,    34,    35,
      36,    37,    38,    39,    28,    29,    30,    31,     5,    33,
      34,    35,    36,    37,    38,    39,    -1,    -1,    -1,    -1,
       5,    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    30,    31,    -1,    -1,    34,    35,    36,
      37,    38,    39,    28,    29,    30,    31,    -1,    -1,    34,
      35,    36,    37,    38,    39,     9,    10,    11,    12,     9,
      14,    15,    12,    -1,    14,    15,    -1,    -1,    18,     9,
      -1,    11,    12,    27,    14,    15,     9,    27,    -1,    12,
      19,    14,    15,    -1,    -1,    18,    40,    27,    -1,    28,
      40,    -1,    -1,    32,    27,    -1,    -1,    -1,    -1,    -1,
      40,    40,    41,    42,    -1,    -1,    -1,    40,    28,    29,
      30,    31,    -1,    -1,    34,    35,    36,    37,    38,    39
  };

  const signed char
  parser::yystos_[] =
  {
       0,    45,    46,     0,     0,     9,    12,    14,    15,    27,
      40,    47,    48,    49,    50,    51,    52,     4,    14,     4,
      14,    47,    19,    28,    32,    40,    41,    42,    53,    53,
       6,    26,    53,    41,    53,    32,     5,    20,    25,    28,
      29,    30,    31,    34,    35,    36,    37,    38,    39,    53,
      53,    33,    53,    53,    53,    46,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    24,    46,    33,    10,
      11,    53,    18,    46,    15,    46,    11,    18,    15
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    44,    45,    46,    46,    46,    46,    46,    46,    46,
      47,    48,    48,    49,    49,    50,    50,    51,    51,    52,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     2,     0,     2,     2,     2,     2,     2,     2,
       3,     2,     2,     2,     2,     6,     8,     6,     8,     2,
       1,     2,     2,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     4,     3
  };




#if YYDEBUG
  const unsigned char
  parser::yyrline_[] =
  {
       0,    95,    95,    98,    99,   100,   101,   102,   103,   104,
     107,   110,   111,   114,   115,   118,   119,   122,   123,   126,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153
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
#line 1999 "parser.cc"

#line 154 "parser.yy"


void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
