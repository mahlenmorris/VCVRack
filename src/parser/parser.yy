/* Parser for BASICally.   -*- C++ -*-
  TODO: Do I need to mark this as a %pure_parser? Saw something that says
  this is needed to make the parser reentrant.

*/

%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.2"
%header

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
  #include <string>
  #include "tree.h"
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
%token <std::string>
  ABS     "abs"
  ALL     "all"
  AND     "and"
  ASSIGN  "="
  CEILING "ceiling"
  CLAMP   "clamp"
  CONTINUE "continue"
  ELSE    "else"
  END     "end"
  EXIT    "exit"
  FLOOR   "floor"
  FOR     "for"
  IF      "if"
  MAX     "max"
  MIN     "min"
  NEXT    "next"
  NOT     "not"
  OR      "or"
  POW     "pow"
  SIGN    "sign"
  SIN     "sin"
  STEP    "step"
  THEN    "then"
  TO      "to"
  WAIT    "wait"
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
  COMMA   ","
;

%token <std::string> IDENTIFIER "identifier"
%token <float> NUMBER "number"
%token <std::string> NOTE "note"
%token <std::string> ONEARGFUNC "oneargfunc"
%token <std::string> TWOARGFUNC "twoargfunc"
%token <std::string> COMPARISON "comparison"
%nterm <Expression> exp
%nterm <Statements> statements
%nterm <Line> assignment
%nterm <Line> continue_statement
%nterm <Line> exit_statement
%nterm <Line> for_statement
%nterm <Line> if_statement
%nterm <Line> wait_statement

%printer { yyo << $$; } <*>;

%%
%start program;

program:
  statements YYEOF { drv.lines = $1.lines; }

statements:
  %empty                     {}
| statements assignment           { $$ = $1.add($2); }
| statements continue_statement   { $$ = $1.add($2); }
| statements exit_statement       { $$ = $1.add($2); }
| statements for_statement        { $$ = $1.add($2); }
| statements if_statement         { $$ = $1.add($2); }
| statements wait_statement       { $$ = $1.add($2); }

assignment:
  "identifier" "=" exp  { $$ = Line::Assignment($1, $3, &drv); }

continue_statement:
  "continue" "for"      { $$ = Line::Continue($2); }
| "continue" "all"      { $$ = Line::Continue($2); }

exit_statement:
  "exit" "for"          { $$ = Line::Exit($2); }
| "exit" "all"          { $$ = Line::Exit($2); }

for_statement:
  "for" assignment "to" exp statements "next"  { $$ = Line::ForNext($2, $4, drv.factory.Number(1.0), $5, &drv); }
| "for" assignment "to" exp "step" exp statements "next" { $$ = Line::ForNext($2, $4, $6, $7, &drv); }

if_statement:
  "if" exp "then" statements "end" "if"                    { $$ = Line::IfThen($2, $4); }
| "if" exp "then" statements "else" statements "end" "if"  { $$ = Line::IfThenElse($2, $4, $6); }

wait_statement:
  "wait" exp            { $$ = Line::Wait($2); }

%left "or";
%left "and";
%left COMPARISON;
%left "+" "-";
%left "*" "/";
%precedence NEG;   /* unary minus or "not" */

exp:
  "number"      { $$ = drv.factory.Number((float) $1); }
| "note"        { $$ = drv.factory.Note($1); }
| MINUS "number" %prec NEG { $$ = drv.factory.Number(-1 * (float) $2);}
| "not" exp %prec NEG { $$ = drv.factory.Not($2);}
| "identifier"  { $$ = drv.factory.Variable($1, &drv); }
| exp "+" exp   { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp "-" exp   { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp "*" exp   { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp "/" exp   { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp COMPARISON exp   { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp "and" exp { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp "or" exp  { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| "oneargfunc" "(" exp ")" { $$ = drv.factory.OneArgFunc($1, $3); }
| "twoargfunc" "(" exp "," exp ")" { $$ = drv.factory.TwoArgFunc($1, $3, $5); }
| "(" exp ")"   { $$ = $2; }
%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  drv.errors.push_back(Error(l.begin.line, l.begin.column, m));
}
