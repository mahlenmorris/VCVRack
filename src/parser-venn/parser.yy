/* Parser for BASICally.   -*- C++ -*-
  TODO: Do I need to mark this as a %pure_parser? Saw something that says
  this is needed to make the parser reentrant.

*/

%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.2"
%header

%define api.token.raw
%define api.parser.class {Parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
  #include <string>
  #include "tree.h"
  class Driver;
  typedef void* yyscan_t;
}

// The parsing context.
// If we don't include this, parser cannot access Driver instance.
%parse-param { Driver& drv } { void* yyscanner } {yy::location& loc}
%lex-param { void* yyscanner } {yy::location& loc}

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
%nterm <NumericAssignment> numeric_assign
%nterm <Assignments> assignments
%nterm <Diagram> diagram
%nterm <Circle> circle
%nterm <CircleList> circle_list

%printer { yyo << $$; } <*>;

%%
%start diagram;

// This would likely change once we can define a path.
diagram:
  circle_list YYEOF                    { drv.diagram.circles = $1.circles; }

circle_list:
  circle                               { $$ = CircleList::NewCircleList($1); }
| circle_list circle                   { $$ = $1.Add($2); }

circle:
  "[" "identifier" "]" assignments     { $$ = Circle::NewCircle($2, $4, &drv); }

assignments:
  numeric_assign                       { $$ = Assignments::NewAssignments($1); }
| assignments numeric_assign           { $$ = $1.Add($2); }

numeric_assign:
  "identifier" "=" "number"            { $$ = NumericAssignment::NewAssignment($1, (float) $3); }
| "identifier" "=" MINUS "number"      { $$ = NumericAssignment::NewAssignment($1, -1 * (float) $4); }
 
%%

void
yy::Parser::error (const location_type& l, const std::string& m)
{
  drv.errors.push_back(Error(l.begin.line, l.begin.column, m));
}
