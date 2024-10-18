/* Parser for Venn.   -*- C++ -*-

*/

%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.2"
%language "c++"
%header

// %define api.prefix {venn_yy}
%define api.namespace {VENN}
%define api.token.raw
%define api.parser.class {Parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
  #include <string>
  #include "tree.h"
  class VennDriver;
  typedef void* yyscan_t;
}

// The parsing context.
// If we don't include this, parser cannot access VennDriver instance.
%parse-param { VennDriver& drv } { void* yyscanner } {VENN::location& loc}
%lex-param { void* yyscanner } {VENN::location& loc}

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
#include "driver.h"
}

%define api.token.prefix {TOK_}
%token <std::string>
  ASSIGN  "="
  MINUS   "-"
  LBRACKET "["
  RBRACKET "]"
;

%token <std::string> IDENTIFIER "identifier"
%token <std::string> QUOTED_STRING "quoted_string"
%token <float> NUMBER "number"
%nterm <Assignment> assign
%nterm <Assignments> assignments
%nterm <Circle> circle
%nterm <CircleList> circle_list
%nterm <Expression> exp
%nterm meta_start

%printer { yyo << $$; } <*>;

%%
%start meta_start;

// We can parse a 'diagram' or just an 'exp'.
// This reduces the number of copies of the Expression parsing code.
meta_start:
  circle_list YYEOF                    { drv.diagram.circles = $1.circles; }
| exp YYEOF                            { drv.exp = $1; }

circle_list:
  circle                               { $$ = CircleList::NewCircleList($1); }
| circle_list circle                   { $$ = $1.Add($2); }

circle:
  "[" "]" assignments                  { $$ = Circle::NewCircle("", $3, &drv); }

assignments:
  assign                               { $$ = Assignments::NewAssignments($1); }
| assignments assign                   { $$ = $1.Add($2); }

assign:
  "identifier" "=" "number"            { $$ = Assignment::NumericAssignment($1, (float) $3); }
| "identifier" "=" MINUS "number"      { $$ = Assignment::NumericAssignment($1, -1 * (float) $4); }
| "identifier" "=" "quoted_string"     { $$ = Assignment::StringAssignment($1, $3); }

exp:
  "number"                             { $$ = drv.factory.Number((float) $1); }

%%

void
VENN::Parser::error (const location_type& l, const std::string& m)
{
  drv.errors.push_back(Error(l.begin.line, l.begin.column, m));
}
