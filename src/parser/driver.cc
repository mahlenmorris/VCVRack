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

#include "driver.hh"
#include "parser.hh"

Driver::Driver()
  : trace_parsing (false), trace_scanning (false)
{
}

Driver::~Driver() {
  // Free up the floats that GetVarFromName() allocated.
  for (auto entry : symbol_floats) {
    delete entry.second;
  }
  // And any arrays.
  for (auto entry : symbol_arrays) {
    delete entry.second;
  }
}

// Ports are limited in number, so the module adds them before any
// compilation.
void Driver::AddPortForName(const std::string &name, bool is_input, int number) {
  PortPointer ptr;
  ptr.port_type = is_input ? PortPointer::INPUT : PortPointer::OUTPUT;
  ptr.index = number;
  symbol_ports[name] = ptr;
}

bool Driver::VarHasPort(const std::string &name) {
  return symbol_ports.find(name) != symbol_ports.end();
}

PortPointer Driver::GetPortFromName(const std::string &name) {
  return symbol_ports[name];
}

// Other variables are made on the fly, when asked for.
float* Driver::GetVarFromName(const std::string &name) {
  auto found = symbol_floats.find(name);
  if (found != symbol_floats.end()) {
    return found->second;
  } else {
    float* float_pointer = new float;
    *float_pointer = 0.0f;
    symbol_floats[name] = float_pointer;
    return float_pointer;
  }
}

STArray* Driver::GetArrayFromName(const std::string &name) {
  auto found = symbol_arrays.find(name);
  if (found != symbol_arrays.end()) {
    return found->second;
  } else {
    STArray* pointer = new std::vector<float>();
    symbol_arrays[name] = pointer;
    return pointer;
  }
}

// Returns zero on success.
int Driver::parse(const std::string &text)
{
  location.initialize();
  // Remove any existing lines from a previous parse().
  lines.clear();
  // Remove any errors from previous parse attempt.
  errors.clear();
  return set_text(text);
}
