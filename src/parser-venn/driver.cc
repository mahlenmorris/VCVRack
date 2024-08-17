/* Driver for Venn config parsing.   -*- C++ -*-

Derived from the Driver for calc++, a Bison example program.

*/

#include "driver.h"
#include "parser.hh"

Driver::Driver()
  : trace_parsing (false), trace_scanning (false)
{
}

Driver::~Driver() {
}

// Returns zero on success.
int Driver::parse(const std::string &text)
{
  location.initialize();
  // Remove any errors from previous parse attempt.
  errors.clear();

  // Actually attempt to compile.
  return set_text(text);
}
