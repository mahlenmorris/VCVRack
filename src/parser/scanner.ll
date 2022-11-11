/* Scanner for BASICally.   -*- C++ -*-

 */

%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <cstring> // strerror
# include <string>
# include "driver.hh"
# include "parser.hh"
%}

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

%option noyywrap nounput noinput batch debug

%{
  // A number symbol corresponding to the value in S.
  yy::parser::symbol_type
  make_NUMBER (const std::string &s, const yy::parser::location_type& loc);
%}

id    [a-zA-Z][a-zA-Z_0-9]*
float ([0-9]*[.])?[0-9]+
blank [ \t\r]

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}
%%
%{
  // A handy shortcut to the location held by the driver.
  yy::location& loc = drv.location;
  // Code run each time yylex is called.
  loc.step ();
%}
{blank}+   loc.step ();
\n+        loc.lines (yyleng); loc.step ();
"' ".*     // Comments, skip over. Hint: '.' will not match \n. But location might need fixing.
"-"        return yy::parser::make_MINUS  (yytext, loc);
"+"        return yy::parser::make_PLUS   (yytext, loc);
"*"        return yy::parser::make_STAR   (yytext, loc);
"/"        return yy::parser::make_SLASH  (yytext, loc);
"("        return yy::parser::make_LPAREN (yytext, loc);
")"        return yy::parser::make_RPAREN (yytext, loc);
"="        return yy::parser::make_ASSIGN (yytext, loc);
"=="       return yy::parser::make_EQUALS (yytext, loc);
"!="       return yy::parser::make_NOT_EQUALS (yytext, loc);
"<"        return yy::parser::make_LT     (yytext, loc);
"<="       return yy::parser::make_LTE    (yytext, loc);
">"        return yy::parser::make_GT     (yytext, loc);
">="       return yy::parser::make_GTE    (yytext, loc);
"all"      return yy::parser::make_ALL    (yytext, loc);
"continue" return yy::parser::make_CONTINUE  (yytext, loc);
"else"     return yy::parser::make_ELSE   (yytext, loc);
"endif"    return yy::parser::make_ENDIF  (yytext, loc);
"exit"     return yy::parser::make_EXIT   (yytext, loc);
"for"      return yy::parser::make_FOR    (yytext, loc);
"if"       return yy::parser::make_IF     (yytext, loc);
"next"     return yy::parser::make_NEXT   (yytext, loc);
"step"     return yy::parser::make_STEP   (yytext, loc);
"then"     return yy::parser::make_THEN   (yytext, loc);
"to"       return yy::parser::make_TO     (yytext, loc);
"wait"     return yy::parser::make_WAIT   (yytext, loc);

{float}    return make_NUMBER (yytext, loc);
{id}       return yy::parser::make_IDENTIFIER (yytext, loc);
.          {
             throw yy::parser::syntax_error
               (loc, "invalid character: " + std::string(yytext));
}
<<EOF>>    return yy::parser::make_YYEOF (loc);
%%

yy::parser::symbol_type
make_NUMBER (const std::string &s, const yy::parser::location_type& loc) {
  errno = 0;
  float n = strtof(s.c_str(), NULL);
  return yy::parser::make_NUMBER (n, loc);
}

int
Driver::set_text(const std::string &text) {
  yy_flex_debug = trace_scanning;
  // Creates a buffer from the string.
  YY_BUFFER_STATE input_buffer = yy_scan_string(text.c_str());
  // Tell Flex to use this buffer.
  yy_switch_to_buffer(input_buffer);
  yy::parser parse(*this);
  parse.set_debug_level(trace_parsing);
  int res = parse();
  yy_delete_buffer(input_buffer);  // Free the buffer
  return res;
}
