/* Scanner for Venn.   -*- C++ -*-

 */

%{ /* -*- C++ -*- */
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring> // strerror
#include <string>
#include "driver.h"
#include "parser.hh"

%}
/*
  #define YY_DECL VENN::Parser::symbol_type venn_yylex(yyscan_t yyscanner, VENN::location& loc)
  #define yyterminate() return VENN::Parser::make_END(loc)
*/

%{
#if defined __clang__
# define CLANG_VERSION (__clang_major__ * 100 + __clang_minor__)
#endif

// Clang and ICC like to pretend they are GCC.
#if defined __GNUC__ && !defined __clang__ && !defined __ICC
# define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#endif

// Pacify warnings in yy_init_buffer (observed with Flex 2.6.4)
// and GCC 6.4.0, 7.3.0 with -O3.
#if defined GCC_VERSION && 600 <= GCC_VERSION
# pragma GCC diagnostic ignored "-Wnull-dereference"
#endif

// This example uses Flex's C back end, yet compiles it as C++.
// So expect warnings about C style casts and NULL.
#if defined CLANG_VERSION && 500 <= CLANG_VERSION
# pragma clang diagnostic ignored "-Wold-style-cast"
# pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#elif defined GCC_VERSION && 407 <= GCC_VERSION
# pragma GCC diagnostic ignored "-Wold-style-cast"
# pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#define FLEX_VERSION (YY_FLEX_MAJOR_VERSION * 100 + YY_FLEX_MINOR_VERSION)

// Old versions of Flex (2.5.35) generate an incomplete documentation comment.
//
//  In file included from src/scan-code-c.c:3:
//  src/scan-code.c:2198:21: error: empty paragraph passed to '@param' command
//        [-Werror,-Wdocumentation]
//   * @param line_number
//     ~~~~~~~~~~~~~~~~~^
//  1 error generated.
#if FLEX_VERSION < 206 && defined CLANG_VERSION
# pragma clang diagnostic ignored "-Wdocumentation"
#endif

// Old versions of Flex (2.5.35) use 'register'.  Warnings introduced in
// GCC 7 and Clang 6.
#if FLEX_VERSION < 206
# if defined CLANG_VERSION && 600 <= CLANG_VERSION
#  pragma clang diagnostic ignored "-Wdeprecated-register"
# elif defined GCC_VERSION && 700 <= GCC_VERSION
#  pragma GCC diagnostic ignored "-Wregister"
# endif
#endif

#if FLEX_VERSION < 206
# if defined CLANG_VERSION
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wdocumentation"
#  pragma clang diagnostic ignored "-Wshorten-64-to-32"
#  pragma clang diagnostic ignored "-Wsign-conversion"
# elif defined GCC_VERSION
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
# endif
#endif

// Flex 2.6.4, GCC 9
// warning: useless cast to type 'int' [-Wuseless-cast]
// 1361 |   YY_CURRENT_BUFFER_LVALUE->yy_buf_size = (int) (new_size - 2);
//      |                                                 ^
#if defined GCC_VERSION && 900 <= GCC_VERSION
# pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
%}

%option reentrant noyywrap nounput noinput batch debug

%{
  // A number symbol corresponding to the value in S.
  VENN::Parser::symbol_type
  make_NUMBER (const std::string &s, const VENN::Parser::location_type& loc);

  // I would have preferred to have patterns for variable names and field names,
  // since they are both known at compile time, but "x" and "y" are in both sets,
  // and thus cannot be disambiguated (at least, not easily.)
%}

note  [a-g][#b]?(-1|[0-9]|10)
id    [a-zA-Z][a-zA-Z_0-9]*
oneargfunc "abs"|"ceiling"|"floor"|"log2"|"loge"|"log10"|"sign"|"sin"
twoargfunc "max"|"min"|"mod"|"pow"
comparison "<"|"<="|">"|">="|"=="|"!="
float ([0-9]*[.])?[0-9]+
blank [ \t\r]

%{
  // Code run each time a pattern is matched.
  #define YY_USER_ACTION  loc.columns (yyleng);
%}
%%
%{
  // A handy shortcut to the location held by the driver.
  //VENN::location& loc = drv.location;
  // Code run each time venn_yylex is called.
  loc.step ();
%}
{blank}+   loc.step ();
\n+        loc.lines (yyleng); loc.step ();
"' ".*     // Comments, skip over. Hint: '.' will not match \n. But location might need fixing.
\"[^"]*\"  return VENN::Parser::make_QUOTED_STRING  (yytext, loc);  // Will this grab newlines?
"-"        return VENN::Parser::make_MINUS  (yytext, loc);
"+"        return VENN::Parser::make_PLUS   (yytext, loc);
"*"        return VENN::Parser::make_STAR   (yytext, loc);
"/"        return VENN::Parser::make_SLASH  (yytext, loc);
"("        return VENN::Parser::make_LPAREN (yytext, loc);
")"        return VENN::Parser::make_RPAREN (yytext, loc);
"="        return VENN::Parser::make_ASSIGN (yytext, loc);
"["        return VENN::Parser::make_LBRACKET (yytext, loc);
"]"        return VENN::Parser::make_RBRACKET (yytext, loc);
","        return VENN::Parser::make_COMMA  (yytext, loc);
"?"        return VENN::Parser::make_QUESTION(yytext, loc);
":"        return VENN::Parser::make_COLON  (yytext, loc);
"and"      return VENN::Parser::make_AND    (yytext, loc);
"not"      return VENN::Parser::make_NOT    (yytext, loc);
"or"       return VENN::Parser::make_OR     (yytext, loc);
"limit"    return VENN::Parser::make_LIMIT  (yytext, loc);
"scale"    return VENN::Parser::make_SCALE  (yytext, loc);
{float}    return make_NUMBER (yytext, loc);
{oneargfunc} return VENN::Parser::make_ONEARGFUNC (yytext, loc);
{twoargfunc} return VENN::Parser::make_TWOARGFUNC (yytext, loc);
{comparison} return VENN::Parser::make_COMPARISON (yytext, loc);
{id}       return VENN::Parser::make_IDENTIFIER (yytext, loc);
.          {
             throw VENN::Parser::syntax_error
               (loc, "invalid character: " + std::string(yytext));
}
<<EOF>>    return VENN::Parser::make_YYEOF (loc);
%%

VENN::Parser::symbol_type
make_NUMBER (const std::string &s, const VENN::Parser::location_type& loc) {
  errno = 0;
  float n = strtof(s.c_str(), NULL);
  return VENN::Parser::make_NUMBER (n, loc);
}

int
VennDriver::set_text(const std::string &text) {
  yyscan_t scanner;

  yylex_init(&scanner);
  VENN::location* loc = new VENN::location();
  // yy_flex_debug = trace_scanning;
  // Creates a buffer from the string.
  YY_BUFFER_STATE input_buffer = yy_scan_string(text.c_str(), scanner);
  // Tell Flex to use this buffer.
  yy_switch_to_buffer(input_buffer, scanner);

  VENN::Parser the_parser(*this, scanner, *loc);
  the_parser.set_debug_level(trace_parsing);
  int res = the_parser.parse();
  yy_delete_buffer(input_buffer, scanner);  // Free the buffer
  yylex_destroy(scanner);
  delete loc;
  if (res == 0) {
    return (errors.size() == 0 ? 0 : 1);
  } else {
    return res;  
  }
}
