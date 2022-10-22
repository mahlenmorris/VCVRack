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
  # include <string>
  #include "Tree.h"
  class driver;
}

// The parsing context.
%param { driver& drv }

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
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
;

%token <VariableExpression> IDENTIFIER "identifier"
%token <int> NUMBER "number"
%nterm <ExpressionBase> exp

%printer { yyo << $$; } <*>;

%%
%start unit;
unit: assignments

assignments:
  %empty                 {}
| assignments assignment {};

assignment:
  "identifier" ":=" exp {
    drv.lines.push_back(Line::Assignment($1, $3));
   };

%left "+" "-";
%left "*" "/";
exp:
  "number"      { $$ = NumberExpression((float) $1); }
| "identifier"  { $$ = VariableExpression($1); }
| exp "+" exp   { $$ = BinOpExpression(yy::parser::token::token_kind_type::TOK_PLUS, $1, $3); }
| exp "-" exp   { $$ = BinOpExpression(yy::parser::token::token_kind_type::TOK_MINUS, $1, $3); }
| exp "*" exp   { $$ = BinOpExpression(yy::parser::token::token_kind_type::TOK_STAR, $1, $3); }
| exp "/" exp   { $$ = BinOpExpression(yy::parser::token::token_kind_type::TOK_SLASH, $1, $3); }
| "(" exp ")"   { $$ = $2; }
%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
