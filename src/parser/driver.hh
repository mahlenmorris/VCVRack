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
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include "tree.h"
#include "parser.hh"

// Give Flex the prototype of yylex we want ...
// TODO: move this to scanner.ll
typedef void* yyscan_t;
# define YY_DECL \
  yy::Parser::symbol_type yylex (yyscan_t yyscanner, yy::location& loc)
// ... and declare it for the parser's sake.
YY_DECL;
// From scanner.cc

struct Error {
  int line;
  int column;
  std::string message;

  Error(int line, int column, const std::string message) : line{line},
      column{column}, message{message} {
  }
  std::string to_string() {
    return "line: " + std::to_string(line) +
           ", column: " + std::to_string(column) +
           " : '" + message + "'.";
  }
};

// Conducting the whole scanning and parsing of Calc++.
class Driver
{
public:
  // If able to parse, this is the abstract syntax tree for the program.
  // Cannot be executed; needs to be turned into PCode objects before
  // Basically can run it.
  std::vector<Block> blocks;
  // List of syntax errors found before parser gave up.
  std::vector<Error> errors;
  // Knows how to create various kinds of Expression objects.
  ExpressionFactory factory;
  // Whether to generate parser debug traces.
  bool trace_parsing;
  // Whether to generate scanner debug traces.
  bool trace_scanning;
  // The token's location used by the scanner.
  yy::location location;
  // Maps the name of a variable to a pointer to it.
  std::unordered_map<std::string, float*> symbol_floats;
  // Maps the name of a string variable to a pointer to it.
  std::unordered_map<std::string, std::string*> symbol_strings;
  // Maps the name of a variable to the Port it refers to, if any.
  std::unordered_map<std::string, PortPointer> symbol_ports;
  // Maps the name of an array variable to a pointer to it.
  std::unordered_map<std::string, STArray* > symbol_arrays;
  // List of INn port indexes that need trigger() to be maintained
  // for the current program. Cleared with every attempted compile.
  std::unordered_set<int> trigger_port_indexes;

  Driver();
  ~Driver();

  bool VarHasPort(const std::string &name);
  float* GetVarFromName(const std::string &name);
  std::string* GetStringVarFromName(const std::string &name);
  void AddPortForName(const std::string &name, bool is_input, int number);
  PortPointer GetPortFromName(const std::string &name);
  STArray* GetArrayFromName(const std::string &name);

  void SetEnvironment(Environment* env) {
    factory.SetEnvironment(env);
  }

  // Reset the state of all variables to zero/empty.
  void Clear() {
    for (const auto &element : symbol_floats) {
      *(element.second) = 0.0f;
    }
    for (const auto &element : symbol_strings) {
      element.second->clear();
    }
    for (const auto &element : symbol_arrays) {
      element.second->clear();
    }
  }

  // Run the parser on the text of string f.  Return 0 on success.
  int parse(const std::string& f);

  // Handling the scanner.
  // Defined in scanner.ll, for odd reasons. Maybe shouldn't be.
  int set_text(const std::string &text);
};


#endif // ! DRIVER_HH
