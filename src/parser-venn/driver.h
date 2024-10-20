/* Driver for Venn module.   -*- C++ -*-

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

#ifndef DRIVER_H
#define DRIVER_H
#include <string>
#include <unordered_map>
#include <vector>
#include "tree.h"
#include "parser.hh"

// Give Flex the prototype of yylex we want ...
// TODO: move this to scanner.ll
typedef void* yyscan_t;
# define YY_DECL \
  VENN::Parser::symbol_type yylex (yyscan_t yyscanner, VENN::location& loc)
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

// This constitutes the variable space for all of our expressions.
// Unlike BASICally, the user cannot create variables; they can only
// get their values, which are set by the module.
struct VennVariables {
  float var_x, var_y, var_distance, var_within, var_pointx, var_pointy;
  float var_leftx, var_rightx, var_topy, var_bottomy;

  std::unordered_map<std::string, float*> name_to_variable;

  VennVariables() {
    name_to_variable["x"] = &var_x;
    name_to_variable["y"] = &var_y;
    name_to_variable["distance"] = &var_distance;
    name_to_variable["within"] = &var_within;
    name_to_variable["pointx"] = &var_pointx;
    name_to_variable["pointy"] = &var_pointy;
    name_to_variable["leftx"] = &var_leftx;
    name_to_variable["rightx"] = &var_rightx;
    name_to_variable["topy"] = &var_topy;
    name_to_variable["bottomy"] = &var_bottomy;
  }

  bool IsVariableName(const char * var_name) {
    return name_to_variable.find(var_name) != name_to_variable.end();
  }

  float* GetVarFromName(const char * var_name) {
    return name_to_variable.at(var_name);
  }

};

// Conducting the whole scanning and parsing of Calc++.
class VennDriver
{
public:
  // This Driver can parser two different kinds of objects,
  // Diagrams and Expressions.
  // The Venn module Diagram being constructed.
  Diagram diagram;
  // _OR_
  // Expression we just parsed.
  Expression exp;

  // List of syntax errors found before parser gave up.
  std::vector<Error> errors;

  VennVariables variables;

  // Knows how to create various kinds of Expression objects.
  ExpressionFactory factory;

  // Whether to generate parser debug traces.
  bool trace_parsing;
  // Whether to generate scanner debug traces.
  bool trace_scanning;
  // The token's location used by the scanner.
  VENN::location location;

  VennDriver();
  ~VennDriver();

  // Reset the state of all variables to zero/empty.
  void Clear() {
    diagram.circles.clear();
  }

  bool IsVariableName(const char * var_name) {
    return variables.IsVariableName(var_name);
  }

  // Will throw exception if name not found, use IsVariableName prior
  // to calling.
  float* GetVarFromName(const char * var_name) {
    return variables.GetVarFromName(var_name);
  }

  // Run the parser on the text of string f.  Return 0 on success.
  int parse(const std::string& f);

  // Handling the scanner.
  // Defined in scanner.ll, for odd reasons. Maybe shouldn't be.
  int set_text(const std::string &text);

  void AddError(const std::string& message) {
    errors.push_back(Error(location.begin.line, location.begin.column, message));
  }
};


#endif // ! DRIVER_H
