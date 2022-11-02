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
%token <std::string>
  ASSIGN  "="
  WAIT    "wait"
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
  EQUALS  "=="
  NOT_EQUALS "!="
  LT      "<"
  LTE     "<="
  GT      ">"
  GTE     ">="
;

%token <Expression> IDENTIFIER "identifier"
%token <float> NUMBER "number"
%nterm <Expression> exp
%nterm <BoolExpression> bool_exp
%nterm <Statements> statements
%nterm <Line> assignment
%nterm <Line> wait_statement
/* %nterm <Line> if_statement */

%printer { yyo << $$; } <*>;

%%
%start program;

program:
  statements YYEOF { drv.lines = $1.lines; }

statements:
  %empty                     {}
| statements assignment      { $$ = $1.add($2); }
| statements wait_statement  { $$ = $1.add($2); }

/* | statements if_statement  { drv.lines.push_back($2); }  */

assignment:
  "identifier" "=" exp  { $$ = Line::Assignment($1, $3); }

wait_statement:
  "wait" exp            { $$ = Line::Wait($2); }

/*
if_statement:
  "if" bool_exp "then" statements "endif"
| "if" bool_exp "then" statements "else" statements "endif"
*/

%left "+" "-";
%left "*" "/";
%precedence NEG;   /* unary minus */

exp:
  "number"      { $$ = Expression::Number((float) $1); }
| MINUS "number" %prec NEG { $$ = Expression::Number(-1 * (float) $2);}
| "identifier"  { $$ = Expression::Variable($1); }
| exp "+" exp   { $$ = Expression::Plus($1, $3); }
| exp "-" exp   { $$ = Expression::Minus($1, $3); }
| exp "*" exp   { $$ = Expression::Times($1, $3); }
| exp "/" exp   { $$ = Expression::Divide($1, $3); }
| "(" exp ")"   { $$ = $2; }

bool_exp:
  exp "<" exp   { $$ = BoolExpression($1, $2, $3); }
| exp "<=" exp  { $$ = BoolExpression($1, $2, $3); }
| exp ">" exp   { $$ = BoolExpression($1, $2, $3); }
| exp ">=" exp  { $$ = BoolExpression($1, $2, $3); }
| exp "==" exp  { $$ = BoolExpression($1, $2, $3); }
| exp "!=" exp  { $$ = BoolExpression($1, $2, $3); }
%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
