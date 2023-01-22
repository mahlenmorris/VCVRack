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
#include "driver.hh"
}

%define api.token.prefix {TOK_}
%token <std::string>
  ABS     "abs"
  ALL     "all"
  ALSO    "also"
  AND     "and"
  ASSIGN  "="
  CEILING "ceiling"
  CLEAR   "clear"
  CONNECTED "connected"
  CONTINUE "continue"
  ELSE    "else"
  ELSEIF  "elseif"
  END     "end"
  EXIT    "exit"
  FLOOR   "floor"
  FOR     "for"
  IF      "if"
  LOG2    "log2"
  LOGE    "loge"
  LOG10   "log10"
  MAX     "max"
  MIN     "min"
  NEXT    "next"
  NOT     "not"
  OR      "or"
  POW     "pow"
  RESET   "reset"
  SAMPLE_RATE "sample_rate"
  SIGN    "sign"
  SIN     "sin"
  START   "start"
  STEP    "step"
  THEN    "then"
  TIME    "time"
  TIME_MILLIS "time_millis"
  TO      "to"
  TRIGGER "trigger"
  WAIT    "wait"
  WHEN    "when"
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
  LBRACE  "{"
  RBRACE  "}"
  LBRACKET "["
  RBRACKET "]"
  COMMA   ","
;

%token <std::string> IDENTIFIER "identifier"
%token <float> NUMBER "number"
%token <std::string> NOTE "note"
%token <std::string> IN_PORT "in_port"
%token <std::string> OUT_PORT "out_port"
%token <std::string> ZEROARGFUNC "zeroargfunc"
%token <std::string> ONEARGFUNC "oneargfunc"
%token <std::string> TWOARGFUNC "twoargfunc"
%token <std::string> COMPARISON "comparison"
%nterm <Expression> exp
%nterm <ExpressionList> expression_list
%nterm <Blocks> blocks
%nterm <Block> block
%nterm <Block> main_block
%nterm <Statements> elseif_group
%nterm <Statements> zero_or_more_statements
%nterm <Statements> one_or_more_statements
%nterm <Line> statement
%nterm <Line> array_assignment
%nterm <Line> assignment
%nterm <Line> clear_statement
%nterm <Line> continue_statement
%nterm <Line> elseif_clause
%nterm <Line> exit_statement
%nterm <Line> for_statement
%nterm <Line> if_statement
%nterm <Line> reset_statement
%nterm <Line> wait_statement

%printer { yyo << $$; } <*>;

%%
%start program;

program:
  blocks YYEOF                    { drv.blocks = $1.block_list; }

blocks:
  main_block                      { $$ = Blocks($1); }
| block                           { $$ = Blocks($1); }
| blocks block                    { $$ = $1.Add($2); }

block:
  "also" one_or_more_statements "end" "also"  { $$ = Block::MainBlock($2); }
| "when" exp one_or_more_statements "end" "when" { $$ = Block::WhenExpBlock($2, $3); }

main_block:
  one_or_more_statements          { $$ = Block::MainBlock($1); }

zero_or_more_statements:
  %empty                             {}
| zero_or_more_statements statement  { $$ = $1.add($2); }

one_or_more_statements:
  statement                        { $$  = Statements::FirstStatement($1); }
| one_or_more_statements statement { $$ = $1.add($2); }

statement:
  array_assignment     { $$ = $1; }
| assignment           { $$ = $1; }
| clear_statement      { $$ = $1; }
| continue_statement   { $$ = $1; }
| exit_statement       { $$ = $1; }
| for_statement        { $$ = $1; }
| if_statement         { $$ = $1; }
| reset_statement      { $$ = $1; }
| wait_statement       { $$ = $1; }

array_assignment:
  "identifier" "[" exp "]" "=" exp  { $$ = Line::ArrayAssignment($1, $3, $6, &drv); }
| "identifier" "[" exp "]" "=" "{" expression_list "}"  { $$ = Line::ArrayAssignment($1, $3, $7, &drv); }

assignment:
  "identifier" "=" exp  { $$ = Line::Assignment($1, $3, &drv); }
| "in_port" "=" exp     { $$ = Line::Assignment($1, $3, &drv); }
| "out_port" "=" exp    { $$ = Line::Assignment($1, $3, &drv); }

clear_statement:
  "clear" "all"         { $$ = Line::ClearAll(); }

continue_statement:
  "continue" "for"      { $$ = Line::Continue($2); }
| "continue" "all"      { $$ = Line::Continue($2); }

exit_statement:
  "exit" "for"          { $$ = Line::Exit($2); }
| "exit" "all"          { $$ = Line::Exit($2); }

for_statement:
  "for" assignment "to" exp zero_or_more_statements "next"  { $$ = Line::ForNext($2, $4, drv.factory.Number(1.0), $5, &drv); }
| "for" assignment "to" exp "step" exp zero_or_more_statements "next" { $$ = Line::ForNext($2, $4, $6, $7, &drv); }

elseif_group:
  %empty                          {}
| elseif_group elseif_clause      { $$ = $1.add($2); }

elseif_clause:
  "elseif" exp "then" zero_or_more_statements  { $$ = Line::ElseIf($2, $4); }

if_statement:
  "if" exp "then" zero_or_more_statements elseif_group "end" "if"       { $$ = Line::IfThen($2, $4, $5); }
| "if" exp "then" zero_or_more_statements elseif_group "else" zero_or_more_statements "end" "if"  { $$ = Line::IfThenElse($2, $4, $7, $5); }

reset_statement:
  "reset"               { $$ = Line::Reset(); }

wait_statement:
  "wait" exp            { $$ = Line::Wait($2); }

%left "or";
%left "and";
%left COMPARISON;
%left "+" "-";
%left "*" "/";
%precedence NEG;   /* unary minus or "not" */

expression_list:
  exp                         { $$ = ExpressionList($1); }
| expression_list "," exp     { $$ = $1.add($3); }

exp:
  "number"      { $$ = drv.factory.Number((float) $1); }
| "note"        { $$ = drv.factory.Note($1); }
| MINUS "number" %prec NEG { $$ = drv.factory.Number(-1 * (float) $2);}
| "not" exp %prec NEG { $$ = drv.factory.Not($2);}
| "in_port"     { $$ = drv.factory.Variable($1, &drv); }
| "out_port"    { $$ = drv.factory.Variable($1, &drv); }
| "identifier"  { $$ = drv.factory.Variable($1, &drv); }
| "identifier" "[" exp "]" { $$ = drv.factory.ArrayVariable($1, $3, &drv); }
| exp "+" exp   { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp "-" exp   { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp "*" exp   { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp "/" exp   { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp COMPARISON exp   { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp "and" exp { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| exp "or" exp  { $$ = drv.factory.CreateBinOp($1, $2, $3); }
| "zeroargfunc" "(" ")" { $$ = drv.factory.ZeroArgFunc($1); }
| "connected" "(" "in_port" ")" {$$ = drv.factory.OnePortFunc($1, $3, &drv);}
| "connected" "(" "out_port" ")" {$$ = drv.factory.OnePortFunc($1, $3, &drv);}
| "trigger"   "(" "in_port" ")" {$$ = drv.factory.OnePortFunc($1, $3, &drv);}
| "oneargfunc" "(" exp ")" { $$ = drv.factory.OneArgFunc($1, $3); }
| "twoargfunc" "(" exp "," exp ")" { $$ = drv.factory.TwoArgFunc($1, $3, $5); }
| "(" exp ")"   { $$ = $2; }
%%

void
yy::Parser::error (const location_type& l, const std::string& m)
{
  drv.errors.push_back(Error(l.begin.line, l.begin.column, m));
}
