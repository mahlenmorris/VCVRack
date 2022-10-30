#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <unordered_map>
#include <string>

// The input (in1-in4), output (out1-out4) and internal variables.
struct Environment {
  float in[4];
  float out[4];
  std::unordered_map<std::string, float> variables;

  Environment() {
    variables.reserve(25);  // Unlikely to find many variable in a program.
  }
};

#endif // ENVIRONMENT_H
