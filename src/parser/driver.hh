/* Driver for calc++.   -*- C++ -*-

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

#ifndef DRIVER_HH
#define DRIVER_HH
#include <string>
#include <vector>
#include "parser.hh"

// Give Flex the prototype of yylex we want ...
# define YY_DECL \
  yy::parser::symbol_type yylex (Driver& drv)
// ... and declare it for the parser's sake.
YY_DECL;

struct Error {
  int line;
  int column;
  std::string message;

  Error(int line, int column, const std::string message) : line{line},
      column{column}, message{message} {
  }
};

// Conducting the whole scanning and parsing of Calc++.
class Driver
{
public:
  // If able to parse, this is the abstract syntax tree for the program.
  // Cannot be executed; needs to be turned into PCode objects before
  // Basically can run it.
  std::vector<Line> lines;
  // List of syntax errors found before parser gave up.
  std::vector<Error> errors;
  // Whether to generate parser debug traces.
  bool trace_parsing;
  // Whether to generate scanner debug traces.
  bool trace_scanning;
  // The token's location used by the scanner.
  yy::location location;

  Driver();

  // Run the parser on the text of string f.  Return 0 on success.
  int parse(const std::string& f);

  // Handling the scanner.
  // Defined in scanner.ll, for odd reasons. Maybe shouldn't be.
  int set_text(const std::string &text);
};


#endif // ! DRIVER_HH
