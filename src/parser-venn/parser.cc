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


// Take the name prefix into account.
#define yylex   vvlex



#include "parser.hh"


// Unqualified %code blocks.
#line 37 "parser.yy"

#include "driver.h"

#line 52 "parser.cc"


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

#line 11 "parser.yy"
namespace VENN {
#line 145 "parser.cc"

  /// Build a parser object.
  Parser::Parser (VennDriver& drv_yyarg, void* yyscanner_yyarg, VENN::location& loc_yyarg)
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
      case symbol_kind::S_assign: // assign
        value.YY_MOVE_OR_COPY< Assignment > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assignments: // assignments
        value.YY_MOVE_OR_COPY< Assignments > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_circle: // circle
        value.YY_MOVE_OR_COPY< Circle > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_circle_list: // circle_list
        value.YY_MOVE_OR_COPY< CircleList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_exp: // exp
        value.YY_MOVE_OR_COPY< VennExpression > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.YY_MOVE_OR_COPY< float > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_LIMIT: // "limit"
      case symbol_kind::S_LOG2: // "log2"
      case symbol_kind::S_LOGE: // "loge"
      case symbol_kind::S_LOG10: // "log10"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_MOD: // "mod"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_SCALE: // "scale"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_LBRACKET: // "["
      case symbol_kind::S_RBRACKET: // "]"
      case symbol_kind::S_COMMA: // ","
      case symbol_kind::S_QUESTION: // "?"
      case symbol_kind::S_COLON: // ":"
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
      case symbol_kind::S_NOTE: // "note"
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
      case symbol_kind::S_assign: // assign
        value.move< Assignment > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assignments: // assignments
        value.move< Assignments > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_circle: // circle
        value.move< Circle > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_circle_list: // circle_list
        value.move< CircleList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_exp: // exp
        value.move< VennExpression > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.move< float > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_LIMIT: // "limit"
      case symbol_kind::S_LOG2: // "log2"
      case symbol_kind::S_LOGE: // "loge"
      case symbol_kind::S_LOG10: // "log10"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_MOD: // "mod"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_SCALE: // "scale"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_LBRACKET: // "["
      case symbol_kind::S_RBRACKET: // "]"
      case symbol_kind::S_COMMA: // ","
      case symbol_kind::S_QUESTION: // "?"
      case symbol_kind::S_COLON: // ":"
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
      case symbol_kind::S_NOTE: // "note"
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
      case symbol_kind::S_assign: // assign
        value.copy< Assignment > (that.value);
        break;

      case symbol_kind::S_assignments: // assignments
        value.copy< Assignments > (that.value);
        break;

      case symbol_kind::S_circle: // circle
        value.copy< Circle > (that.value);
        break;

      case symbol_kind::S_circle_list: // circle_list
        value.copy< CircleList > (that.value);
        break;

      case symbol_kind::S_exp: // exp
        value.copy< VennExpression > (that.value);
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.copy< float > (that.value);
        break;

      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_LIMIT: // "limit"
      case symbol_kind::S_LOG2: // "log2"
      case symbol_kind::S_LOGE: // "loge"
      case symbol_kind::S_LOG10: // "log10"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_MOD: // "mod"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_SCALE: // "scale"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_LBRACKET: // "["
      case symbol_kind::S_RBRACKET: // "]"
      case symbol_kind::S_COMMA: // ","
      case symbol_kind::S_QUESTION: // "?"
      case symbol_kind::S_COLON: // ":"
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
      case symbol_kind::S_NOTE: // "note"
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
      case symbol_kind::S_assign: // assign
        value.move< Assignment > (that.value);
        break;

      case symbol_kind::S_assignments: // assignments
        value.move< Assignments > (that.value);
        break;

      case symbol_kind::S_circle: // circle
        value.move< Circle > (that.value);
        break;

      case symbol_kind::S_circle_list: // circle_list
        value.move< CircleList > (that.value);
        break;

      case symbol_kind::S_exp: // exp
        value.move< VennExpression > (that.value);
        break;

      case symbol_kind::S_NUMBER: // "number"
        value.move< float > (that.value);
        break;

      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_LIMIT: // "limit"
      case symbol_kind::S_LOG2: // "log2"
      case symbol_kind::S_LOGE: // "loge"
      case symbol_kind::S_LOG10: // "log10"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_MOD: // "mod"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_SCALE: // "scale"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_LBRACKET: // "["
      case symbol_kind::S_RBRACKET: // "]"
      case symbol_kind::S_COMMA: // ","
      case symbol_kind::S_QUESTION: // "?"
      case symbol_kind::S_COLON: // ":"
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
      case symbol_kind::S_NOTE: // "note"
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
      case symbol_kind::S_ASSIGN: // "="
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 547 "parser.cc"
        break;

      case symbol_kind::S_ABS: // "abs"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 553 "parser.cc"
        break;

      case symbol_kind::S_AND: // "and"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 559 "parser.cc"
        break;

      case symbol_kind::S_CEILING: // "ceiling"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 565 "parser.cc"
        break;

      case symbol_kind::S_FLOOR: // "floor"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 571 "parser.cc"
        break;

      case symbol_kind::S_LIMIT: // "limit"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 577 "parser.cc"
        break;

      case symbol_kind::S_LOG2: // "log2"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 583 "parser.cc"
        break;

      case symbol_kind::S_LOGE: // "loge"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 589 "parser.cc"
        break;

      case symbol_kind::S_LOG10: // "log10"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 595 "parser.cc"
        break;

      case symbol_kind::S_MAX: // "max"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 601 "parser.cc"
        break;

      case symbol_kind::S_MIN: // "min"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 607 "parser.cc"
        break;

      case symbol_kind::S_MOD: // "mod"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 613 "parser.cc"
        break;

      case symbol_kind::S_NOT: // "not"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 619 "parser.cc"
        break;

      case symbol_kind::S_OR: // "or"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 625 "parser.cc"
        break;

      case symbol_kind::S_POW: // "pow"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 631 "parser.cc"
        break;

      case symbol_kind::S_SCALE: // "scale"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 637 "parser.cc"
        break;

      case symbol_kind::S_SIGN: // "sign"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 643 "parser.cc"
        break;

      case symbol_kind::S_SIN: // "sin"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 649 "parser.cc"
        break;

      case symbol_kind::S_MINUS: // "-"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 655 "parser.cc"
        break;

      case symbol_kind::S_PLUS: // "+"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 661 "parser.cc"
        break;

      case symbol_kind::S_STAR: // "*"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 667 "parser.cc"
        break;

      case symbol_kind::S_SLASH: // "/"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 673 "parser.cc"
        break;

      case symbol_kind::S_LPAREN: // "("
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 679 "parser.cc"
        break;

      case symbol_kind::S_RPAREN: // ")"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 685 "parser.cc"
        break;

      case symbol_kind::S_LBRACKET: // "["
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 691 "parser.cc"
        break;

      case symbol_kind::S_RBRACKET: // "]"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 697 "parser.cc"
        break;

      case symbol_kind::S_COMMA: // ","
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 703 "parser.cc"
        break;

      case symbol_kind::S_QUESTION: // "?"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 709 "parser.cc"
        break;

      case symbol_kind::S_COLON: // ":"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 715 "parser.cc"
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 721 "parser.cc"
        break;

      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 727 "parser.cc"
        break;

      case symbol_kind::S_NUMBER: // "number"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < float > (); }
#line 733 "parser.cc"
        break;

      case symbol_kind::S_NOTE: // "note"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 739 "parser.cc"
        break;

      case symbol_kind::S_ONEARGFUNC: // "oneargfunc"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 745 "parser.cc"
        break;

      case symbol_kind::S_TWOARGFUNC: // "twoargfunc"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 751 "parser.cc"
        break;

      case symbol_kind::S_COMPARISON: // "comparison"
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < std::string > (); }
#line 757 "parser.cc"
        break;

      case symbol_kind::S_circle_list: // circle_list
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < CircleList > (); }
#line 763 "parser.cc"
        break;

      case symbol_kind::S_circle: // circle
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < Circle > (); }
#line 769 "parser.cc"
        break;

      case symbol_kind::S_assignments: // assignments
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < Assignments > (); }
#line 775 "parser.cc"
        break;

      case symbol_kind::S_assign: // assign
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < Assignment > (); }
#line 781 "parser.cc"
        break;

      case symbol_kind::S_exp: // exp
#line 88 "parser.yy"
                 { yyo << yysym.value.template as < VennExpression > (); }
#line 787 "parser.cc"
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
      case symbol_kind::S_assign: // assign
        yylhs.value.emplace< Assignment > ();
        break;

      case symbol_kind::S_assignments: // assignments
        yylhs.value.emplace< Assignments > ();
        break;

      case symbol_kind::S_circle: // circle
        yylhs.value.emplace< Circle > ();
        break;

      case symbol_kind::S_circle_list: // circle_list
        yylhs.value.emplace< CircleList > ();
        break;

      case symbol_kind::S_exp: // exp
        yylhs.value.emplace< VennExpression > ();
        break;

      case symbol_kind::S_NUMBER: // "number"
        yylhs.value.emplace< float > ();
        break;

      case symbol_kind::S_ASSIGN: // "="
      case symbol_kind::S_ABS: // "abs"
      case symbol_kind::S_AND: // "and"
      case symbol_kind::S_CEILING: // "ceiling"
      case symbol_kind::S_FLOOR: // "floor"
      case symbol_kind::S_LIMIT: // "limit"
      case symbol_kind::S_LOG2: // "log2"
      case symbol_kind::S_LOGE: // "loge"
      case symbol_kind::S_LOG10: // "log10"
      case symbol_kind::S_MAX: // "max"
      case symbol_kind::S_MIN: // "min"
      case symbol_kind::S_MOD: // "mod"
      case symbol_kind::S_NOT: // "not"
      case symbol_kind::S_OR: // "or"
      case symbol_kind::S_POW: // "pow"
      case symbol_kind::S_SCALE: // "scale"
      case symbol_kind::S_SIGN: // "sign"
      case symbol_kind::S_SIN: // "sin"
      case symbol_kind::S_MINUS: // "-"
      case symbol_kind::S_PLUS: // "+"
      case symbol_kind::S_STAR: // "*"
      case symbol_kind::S_SLASH: // "/"
      case symbol_kind::S_LPAREN: // "("
      case symbol_kind::S_RPAREN: // ")"
      case symbol_kind::S_LBRACKET: // "["
      case symbol_kind::S_RBRACKET: // "]"
      case symbol_kind::S_COMMA: // ","
      case symbol_kind::S_QUESTION: // "?"
      case symbol_kind::S_COLON: // ":"
      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_QUOTED_STRING: // "quoted_string"
      case symbol_kind::S_NOTE: // "note"
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
  case 2: // meta_start: circle_list $end
#line 96 "parser.yy"
                                       { drv.diagram.circles = yystack_[1].value.as < CircleList > ().circles; }
#line 1112 "parser.cc"
    break;

  case 3: // meta_start: exp $end
#line 97 "parser.yy"
                                       { drv.exp = yystack_[1].value.as < VennExpression > (); }
#line 1118 "parser.cc"
    break;

  case 4: // circle_list: circle
#line 100 "parser.yy"
                                       { yylhs.value.as < CircleList > () = CircleList::NewCircleList(yystack_[0].value.as < Circle > ()); }
#line 1124 "parser.cc"
    break;

  case 5: // circle_list: circle_list circle
#line 101 "parser.yy"
                                       { yylhs.value.as < CircleList > () = yystack_[1].value.as < CircleList > ().Add(yystack_[0].value.as < Circle > ()); }
#line 1130 "parser.cc"
    break;

  case 6: // circle: "[" "]" assignments
#line 104 "parser.yy"
                                       { yylhs.value.as < Circle > () = Circle::NewCircle("", yystack_[0].value.as < Assignments > (), &drv); }
#line 1136 "parser.cc"
    break;

  case 7: // assignments: assign
#line 107 "parser.yy"
                                       { yylhs.value.as < Assignments > () = Assignments::NewAssignments(yystack_[0].value.as < Assignment > ()); }
#line 1142 "parser.cc"
    break;

  case 8: // assignments: assignments assign
#line 108 "parser.yy"
                                       { yylhs.value.as < Assignments > () = yystack_[1].value.as < Assignments > ().Add(yystack_[0].value.as < Assignment > ()); }
#line 1148 "parser.cc"
    break;

  case 9: // assign: "identifier" "=" "number"
#line 111 "parser.yy"
                                      { yylhs.value.as < Assignment > () = Assignment::NumericAssignment(yystack_[2].value.as < std::string > (), (float) yystack_[0].value.as < float > ()); }
#line 1154 "parser.cc"
    break;

  case 10: // assign: "identifier" "=" "-" "number"
#line 112 "parser.yy"
                                      { yylhs.value.as < Assignment > () = Assignment::NumericAssignment(yystack_[3].value.as < std::string > (), -1 * (float) yystack_[0].value.as < float > ()); }
#line 1160 "parser.cc"
    break;

  case 11: // assign: "identifier" "=" "quoted_string"
#line 113 "parser.yy"
                                      { yylhs.value.as < Assignment > () = Assignment::StringAssignment(yystack_[2].value.as < std::string > (), yystack_[0].value.as < std::string > ()); }
#line 1166 "parser.cc"
    break;

  case 12: // exp: "number"
#line 124 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.Number((float) yystack_[0].value.as < float > ()); }
#line 1172 "parser.cc"
    break;

  case 13: // exp: "note"
#line 125 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.Note(yystack_[0].value.as < std::string > ()); }
#line 1178 "parser.cc"
    break;

  case 14: // exp: "-" "number"
#line 126 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.Number(-1 * (float) yystack_[0].value.as < float > ());}
#line 1184 "parser.cc"
    break;

  case 15: // exp: "not" exp
#line 127 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.Not(yystack_[0].value.as < VennExpression > ());}
#line 1190 "parser.cc"
    break;

  case 16: // exp: "identifier"
#line 128 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.Variable(yystack_[0].value.as < std::string > (), &drv); }
#line 1196 "parser.cc"
    break;

  case 17: // exp: exp "+" exp
#line 129 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.CreateBinOp(yystack_[2].value.as < VennExpression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < VennExpression > ()); }
#line 1202 "parser.cc"
    break;

  case 18: // exp: exp "-" exp
#line 130 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.CreateBinOp(yystack_[2].value.as < VennExpression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < VennExpression > ()); }
#line 1208 "parser.cc"
    break;

  case 19: // exp: exp "*" exp
#line 131 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.CreateBinOp(yystack_[2].value.as < VennExpression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < VennExpression > ()); }
#line 1214 "parser.cc"
    break;

  case 20: // exp: exp "/" exp
#line 132 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.CreateBinOp(yystack_[2].value.as < VennExpression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < VennExpression > ()); }
#line 1220 "parser.cc"
    break;

  case 21: // exp: exp "comparison" exp
#line 133 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.CreateBinOp(yystack_[2].value.as < VennExpression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < VennExpression > ()); }
#line 1226 "parser.cc"
    break;

  case 22: // exp: exp "and" exp
#line 134 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.CreateBinOp(yystack_[2].value.as < VennExpression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < VennExpression > ()); }
#line 1232 "parser.cc"
    break;

  case 23: // exp: exp "or" exp
#line 135 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.CreateBinOp(yystack_[2].value.as < VennExpression > (), yystack_[1].value.as < std::string > (), yystack_[0].value.as < VennExpression > ()); }
#line 1238 "parser.cc"
    break;

  case 24: // exp: "oneargfunc" "(" exp ")"
#line 136 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.OneArgFunc(yystack_[3].value.as < std::string > (), yystack_[1].value.as < VennExpression > ()); }
#line 1244 "parser.cc"
    break;

  case 25: // exp: "twoargfunc" "(" exp "," exp ")"
#line 137 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.TwoArgFunc(yystack_[5].value.as < std::string > (), yystack_[3].value.as < VennExpression > (), yystack_[1].value.as < VennExpression > ()); }
#line 1250 "parser.cc"
    break;

  case 26: // exp: "limit" "(" exp "," exp "," exp ")"
#line 138 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.Limit(yystack_[5].value.as < VennExpression > (), yystack_[3].value.as < VennExpression > (), yystack_[1].value.as < VennExpression > ()); }
#line 1256 "parser.cc"
    break;

  case 27: // exp: "scale" "(" exp "," exp "," exp "," exp "," exp ")"
#line 139 "parser.yy"
                                                       { yylhs.value.as < VennExpression > () = drv.factory.Scale(yystack_[9].value.as < VennExpression > (), yystack_[7].value.as < VennExpression > (), yystack_[5].value.as < VennExpression > (), yystack_[3].value.as < VennExpression > (), yystack_[1].value.as < VennExpression > ()); }
#line 1262 "parser.cc"
    break;

  case 28: // exp: exp "?" exp ":" exp
#line 140 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = drv.factory.TernaryFunc(yystack_[4].value.as < VennExpression > (), yystack_[2].value.as < VennExpression > (), yystack_[0].value.as < VennExpression > ()); }
#line 1268 "parser.cc"
    break;

  case 29: // exp: "(" exp ")"
#line 141 "parser.yy"
                                       { yylhs.value.as < VennExpression > () = yystack_[1].value.as < VennExpression > (); }
#line 1274 "parser.cc"
    break;


#line 1278 "parser.cc"

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
    "end of file", "error", "invalid token", "=", "abs", "and", "ceiling",
  "floor", "limit", "log2", "loge", "log10", "max", "min", "mod", "not",
  "or", "pow", "scale", "sign", "sin", "-", "+", "*", "/", "(", ")", "[",
  "]", ",", "?", ":", "identifier", "quoted_string", "number", "note",
  "oneargfunc", "twoargfunc", "comparison", "NEG", "$accept", "meta_start",
  "circle_list", "circle", "assignments", "assign", "exp", YY_NULLPTR
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


  const signed char Parser::yypact_ninf_ = -23;

  const signed char Parser::yytable_ninf_ = -1;

  const short
  Parser::yypact_[] =
  {
     264,   -21,   272,   -19,   -12,   272,   -15,   -23,   -23,   -23,
     -10,    20,    12,     7,   -23,    19,   272,   -23,   272,   -23,
      53,    -9,   272,   272,   -23,   -23,   -23,   -23,   272,   272,
     272,   272,   272,   272,   272,   272,    65,    77,   -23,    41,
      -9,   -23,    97,   109,   -13,   246,   -22,   -22,   -23,   -23,
     121,    15,   272,   272,   -16,   -23,   -23,   272,   272,   141,
     145,    13,   -23,   -23,   168,   236,   272,   272,   -23,   -23,
     181,   194,   -23,   272,   204,   272,   224,   -23
  };

  const signed char
  Parser::yydefact_[] =
  {
       0,     0,     0,     0,     0,     0,     0,    16,    12,    13,
       0,     0,     0,     0,     4,     0,     0,    15,     0,    14,
       0,     0,     0,     0,     1,     2,     5,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    29,     0,
       6,     7,     0,     0,    22,    23,    18,    17,    19,    20,
       0,    21,     0,     0,     0,     8,    24,     0,     0,     0,
       0,     0,    11,     9,     0,    28,     0,     0,    10,    25,
       0,     0,    26,     0,     0,     0,     0,    27
  };

  const signed char
  Parser::yypgoto_[] =
  {
     -23,   -23,   -23,    33,   -23,     8,    -2
  };

  const signed char
  Parser::yydefgoto_[] =
  {
       0,    12,    13,    14,    40,    41,    15
  };

  const signed char
  Parser::yytable_[] =
  {
      17,    32,    33,    20,    16,    61,    18,    25,    30,    31,
      32,    33,    24,    21,    36,    22,    37,    62,    63,    27,
      42,    43,    19,    39,    28,    35,    44,    45,    46,    47,
      48,    49,    50,    51,     6,    29,    30,    31,    32,    33,
      30,    31,    32,    33,    54,    23,    26,    68,    55,    34,
      59,    60,     0,     0,     0,    64,    65,    35,    28,     0,
       0,     0,     0,     0,    70,    71,     0,     0,     0,    29,
      28,    74,     0,    76,    30,    31,    32,    33,     0,    38,
       0,    29,    28,    34,     0,     0,    30,    31,    32,    33,
       0,    35,     0,    29,    52,    34,     0,     0,    30,    31,
      32,    33,    28,    35,     0,     0,    53,    34,     0,     0,
       0,     0,     0,    29,    28,    35,     0,     0,    30,    31,
      32,    33,     0,    56,     0,    29,    28,    34,     0,     0,
      30,    31,    32,    33,     0,    35,     0,    29,    57,    34,
       0,     0,    30,    31,    32,    33,    28,    35,     0,     0,
      28,    34,    58,     0,     0,     0,     0,    29,     0,    35,
       0,    29,    30,    31,    32,    33,    30,    31,    32,    33,
      66,    34,     0,    28,    67,    34,     0,     0,     0,    35,
       0,     0,     0,    35,    29,     0,    28,     0,     0,    30,
      31,    32,    33,     0,    69,     0,     0,    29,    34,    28,
       0,     0,    30,    31,    32,    33,    35,    72,     0,    28,
      29,    34,     0,     0,     0,    30,    31,    32,    33,    35,
      29,     0,     0,    73,    34,    30,    31,    32,    33,    28,
       0,     0,    35,    75,    34,     0,     0,     0,     0,     0,
      29,    28,    35,     0,     0,    30,    31,    32,    33,     0,
      77,    28,    29,     0,    34,     0,     0,    30,    31,    32,
      33,     0,    35,     0,     0,     0,    34,    30,    31,    32,
      33,     0,     1,     0,    35,     0,     0,     0,     0,     2,
       1,     0,     3,     0,    35,     4,     0,     2,     0,     5,
       3,     6,     0,     4,     0,     0,     7,     5,     8,     9,
      10,    11,     0,     0,     7,     0,     8,     9,    10,    11
  };

  const signed char
  Parser::yycheck_[] =
  {
       2,    23,    24,     5,    25,    21,    25,     0,    21,    22,
      23,    24,     0,    28,    16,    25,    18,    33,    34,     0,
      22,    23,    34,    32,     5,    38,    28,    29,    30,    31,
      32,    33,    34,    35,    27,    16,    21,    22,    23,    24,
      21,    22,    23,    24,     3,    25,    13,    34,    40,    30,
      52,    53,    -1,    -1,    -1,    57,    58,    38,     5,    -1,
      -1,    -1,    -1,    -1,    66,    67,    -1,    -1,    -1,    16,
       5,    73,    -1,    75,    21,    22,    23,    24,    -1,    26,
      -1,    16,     5,    30,    -1,    -1,    21,    22,    23,    24,
      -1,    38,    -1,    16,    29,    30,    -1,    -1,    21,    22,
      23,    24,     5,    38,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    -1,    16,     5,    38,    -1,    -1,    21,    22,
      23,    24,    -1,    26,    -1,    16,     5,    30,    -1,    -1,
      21,    22,    23,    24,    -1,    38,    -1,    16,    29,    30,
      -1,    -1,    21,    22,    23,    24,     5,    38,    -1,    -1,
       5,    30,    31,    -1,    -1,    -1,    -1,    16,    -1,    38,
      -1,    16,    21,    22,    23,    24,    21,    22,    23,    24,
      29,    30,    -1,     5,    29,    30,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    38,    16,    -1,     5,    -1,    -1,    21,
      22,    23,    24,    -1,    26,    -1,    -1,    16,    30,     5,
      -1,    -1,    21,    22,    23,    24,    38,    26,    -1,     5,
      16,    30,    -1,    -1,    -1,    21,    22,    23,    24,    38,
      16,    -1,    -1,    29,    30,    21,    22,    23,    24,     5,
      -1,    -1,    38,    29,    30,    -1,    -1,    -1,    -1,    -1,
      16,     5,    38,    -1,    -1,    21,    22,    23,    24,    -1,
      26,     5,    16,    -1,    30,    -1,    -1,    21,    22,    23,
      24,    -1,    38,    -1,    -1,    -1,    30,    21,    22,    23,
      24,    -1,     8,    -1,    38,    -1,    -1,    -1,    -1,    15,
       8,    -1,    18,    -1,    38,    21,    -1,    15,    -1,    25,
      18,    27,    -1,    21,    -1,    -1,    32,    25,    34,    35,
      36,    37,    -1,    -1,    32,    -1,    34,    35,    36,    37
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     8,    15,    18,    21,    25,    27,    32,    34,    35,
      36,    37,    41,    42,    43,    46,    25,    46,    25,    34,
      46,    28,    25,    25,     0,     0,    43,     0,     5,    16,
      21,    22,    23,    24,    30,    38,    46,    46,    26,    32,
      44,    45,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    29,    29,     3,    45,    26,    29,    31,    46,
      46,    21,    33,    34,    46,    46,    29,    29,    34,    26,
      46,    46,    26,    29,    46,    29,    46,    26
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    40,    41,    41,    42,    42,    43,    44,    44,    45,
      45,    45,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    46
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     2,     2,     1,     2,     3,     1,     2,     3,
       4,     3,     1,     1,     2,     2,     1,     3,     3,     3,
       3,     3,     3,     3,     4,     6,     8,    12,     5,     3
  };




#if YYDEBUG
  const unsigned char
  Parser::yyrline_[] =
  {
       0,    96,    96,    97,   100,   101,   104,   107,   108,   111,
     112,   113,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141
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


#line 11 "parser.yy"
} // VENN
#line 1921 "parser.cc"

#line 143 "parser.yy"


void
VENN::Parser::error (const location_type& l, const std::string& m)
{
  drv.errors.push_back(Error(l.begin.line, l.begin.column, m));
}
