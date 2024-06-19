/* Driver for BASICally parsing.   -*- C++ -*-

Derived from the Driver for calc++, a Bison example program.

*/

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
  // Free up String variables.
  for (auto entry : symbol_strings) {
    delete entry.second;
  }
  // And any arrays.
  for (auto entry : symbol_arrays) {
    delete entry.second;
  }
  for (auto entry : symbol_string_arrays) {
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

std::string* Driver::GetStringVarFromName(const std::string &name) {
  auto found = symbol_strings.find(name);
  if (found != symbol_strings.end()) {
    return found->second;
  } else {
    std::string* string_pointer = new std::string;
    symbol_strings[name] = string_pointer;
    return string_pointer;
  }
}

FloatArray* Driver::GetArrayFromName(const std::string &name) {
  auto found = symbol_arrays.find(name);
  if (found != symbol_arrays.end()) {
    return found->second;
  } else {
    FloatArray* pointer = new FloatArray();
    symbol_arrays[name] = pointer;
    return pointer;
  }
}

StringArray* Driver::GetStringArrayFromName(const std::string &name) {
  auto found = symbol_string_arrays.find(name);
  if (found != symbol_string_arrays.end()) {
    return found->second;
  } else {
    StringArray* pointer = new StringArray();
    symbol_string_arrays[name] = pointer;
    return pointer;
  }
}

// Returns zero on success.
int Driver::parse(const std::string &text)
{
  location.initialize();
  // Remove any existing blocks from a previous parse(). Doesn't affect
  // currently running program.
  // Note that we don't erase the variable space, because we want to keep
  // the current values intact.
  blocks.clear();
  // Remove any errors from previous parse attempt.
  errors.clear();
  // Clear the list of INn ports that have trigger() called on them.
  trigger_port_indexes.clear();
  // Actually attempt to compile.
  return set_text(text);
}
