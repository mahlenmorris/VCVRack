/* Parser for calc++.   -*- C++ -*-

   Copyright (C) 2005-2015, 2018-2021 Free Software Foundation, Inc.

   This file is part of Bison, the GNU Compiler Compiler.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.2"
%header

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
  #include <string>
  #include "tree.hh"
  class Driver;
}

// The parsing context.
%param { Driver& drv }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
# include "driver.hh"
}

%define api.token.prefix {TOK_}
%token
  ASSIGN  ":="
  WAIT    "wait"  /* TODO: would be nice if could accept uppercase "WAIT" */
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
;

%token <Expression> IDENTIFIER "identifier"
%token <int> NUMBER "number"  /* TODO: Should be float and accept negative sign. */
%nterm <Expression> exp
%nterm <Line> assignment

%printer { yyo << $$; } <*>;

%%
%start assignments;

assignments:
  %empty                 {}
| assignments assignment { drv.lines.push_back($2); }

assignment:
  "identifier" ":=" exp { $$ = Line::Assignment($1, $3); }
| "wait" exp            { $$ = Line::Wait($2); }

/* ExpressionBase classes need to get destructors to avoid memory leaks.
 */

%left "+" "-";
%left "*" "/";
exp:
  "number"      { $$ = Expression::Number((float) $1); }
| "identifier"  { $$ = Expression::Variable($1); }
| exp "+" exp   { $$ = Expression::Plus($1, $3); }
| exp "-" exp   { $$ = Expression::Minus($1, $3); }
| exp "*" exp   { $$ = Expression::Times($1, $3); }
| exp "/" exp   { $$ = Expression::Divide($1, $3); }
| "(" exp ")"   { $$ = $2; }
%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
