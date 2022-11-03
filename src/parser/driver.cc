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

Driver::Driver ()
  : trace_parsing (false), trace_scanning (false)
{
}

// Returns zero on success.
int Driver::parse(const std::string &text)
{
  location.initialize();
  // Remove any existing lines from a previous parse().
  lines.clear();
  int success_on_zero = set_text(text);
  // If it succeeded, then caller needs to translate the Line objects into
  // the pseudo-code that runs.
  return success_on_zero;
}
