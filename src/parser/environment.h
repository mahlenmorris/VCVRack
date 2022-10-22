#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <map>
#include <string>

// The input (in1-in4), output (out1-out4) and internal variables.
struct Environment {
  float in[4];
  float out[4];
  std::map<std::string, float> variables;
};

#endif // ENVIRONMENT_H
