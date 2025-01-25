/* Driver for Venn config parsing.   -*- C++ -*-

Derived from the Driver for calc++, a Bison example program.

*/

#include "driver.h"
#include "parser.hh"

VennDriver::VennDriver(std::shared_ptr<VennVariables> const& vars)
  : variables(vars), trace_parsing(false), trace_scanning(false)
{
}

VennDriver::~VennDriver() {
}

// Returns zero on success.
int VennDriver::parse(const std::string &text)
{
  location.initialize();
  // Remove any errors from previous parse attempt.
  errors.clear();

  // Actually attempt to compile.
  return set_text(text);
}
