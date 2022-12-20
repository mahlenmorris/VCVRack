// Code for methods related to parsing data structures.
#include "tree.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <vector>
#include "driver.hh"

std::unordered_map<std::string, Expression::Operation> Expression::string_to_operation = {
  {"+", PLUS},
  {"-", MINUS},
  {"*", TIMES},
  {"/", DIVIDE},
  {"==", EQUAL},
  {"!=", NOT_EQUAL},
  {">", GT},
  {">=", GTE},
  {"<", LT},
  {"<=", LTE},
  {"and", AND},
  {"or", OR},
  {"abs", ABS},
  {"ceiling", CEILING},
  {"floor", FLOOR},
  {"sign", SIGN},
  {"sin", SIN},
  {"mod", MOD},
  {"max", MAX},
  {"min", MIN},
  {"pow", POW}
};

std::unordered_map<std::string, float> note_to_volt_same_octave = {
  {"c", 0.0},
  {"c#", 0.08333333},
  {"db", 0.08333333},
  {"d", 0.16666666},
  {"d#", 0.24999999},
  {"eb", 0.24999999},
  {"e", 0.33333332},
  {"f", 0.41666665},
  {"f#", 0.49999998},
  {"gb", 0.49999998},
  {"g", 0.58333331},
  {"g#", 0.66666664},
  {"ab", 0.66666664},
  {"a", 0.74999997},
  {"a#", 0.8333333},
  {"bb", 0.8333333},
  {"b", 0.91666663}
};

std::unordered_set<std::string> Expression::volatile_inputs = {
  "in1", "in2", "in3", "in4"
};


Expression Expression::Not(const Expression &expr) {
  Expression ex;
  ex.type = NOT;
  ex.subexpressions.push_back(expr);
  return ex;
}

Expression Expression::Note(const std::string &note_name) {
  Expression ex;
  ex.type = NUMBER;
  // split into name and octave
  int octave;
  std::string name;
  // Number at end might be two chars long, in the case of -1 or 10.
  if (note_name.size() == 4 || (note_name.size() == 3 && (
      note_name[1] != '#' && note_name[1] != 'b'))) {
    name = note_name.substr(0, note_name.size() - 2);
    octave = strtol(note_name.c_str() + note_name.size() - 2, NULL, 10);
  } else {
    name = note_name.substr(0, note_name.size() - 1);
    octave = strtol(note_name.c_str() + note_name.size() - 1, NULL, 10);
  }
  auto found = note_to_volt_same_octave.find(name);
  // This can happen. Current regex can accept invalid values like b#.
  // TODO: maybe fix the regex?
  if (found != note_to_volt_same_octave.end()) {
    ex.float_value = found->second + octave - 4;
  } else {
    // Invalid name.
    ex.float_value = 0.0f;
  }
  return ex;
}

Expression Expression::Number(float the_value) {
  Expression ex;
  ex.type = NUMBER;
  ex.float_value = the_value;
  return ex;
}

Expression Expression::OneArgFunc(const std::string &func_name,
                                  const Expression &arg1) {
  Expression ex;
  ex.type = ONEARGFUNC;
  ex.operation = string_to_operation.at(func_name);
  ex.subexpressions.push_back(arg1);
  return ex;
}

Expression Expression::TwoArgFunc(const std::string &func_name,
                                  const Expression &arg1,
                                  const Expression &arg2) {
  Expression ex;
  ex.type = TWOARGFUNC;
  ex.operation = string_to_operation.at(func_name);
  ex.subexpressions.push_back(arg1);
  ex.subexpressions.push_back(arg2);
  return ex;
}

Expression Expression::CreateBinOp(const Expression &lhs,
                                   const std::string &op_string,
                                   const Expression &rhs) {
  Expression ex;
  ex.type = BINOP;
  ex.subexpressions.push_back(lhs);
  ex.subexpressions.push_back(rhs);
  ex.operation = string_to_operation.at(op_string);
  return ex;
}

Expression Expression::Variable(const char *var_name, Driver* driver) {
  // Intentionally copying the name.
  Expression ex;
  ex.type = VARIABLE;
  ex.name = std::string(var_name);
  ex.variable_ptr = driver->GetVarFromName(ex.name);
  return ex;
}
// The parser seems to need many variants of Variable.
Expression Expression::Variable(const std::string &expr, Driver* driver) {
  // Intentionally copying the name.
  return Variable(expr.c_str(), driver);
}
// The parser seems to need many variants of Variable.
Expression::Expression(char * var_name, Driver* driver) {
  // Intentionally _copying_ the name.
  name = std::string(var_name);
  variable_ptr = driver->GetVarFromName(name);
}

Expression Expression::Variable(char* var_name, Driver* driver) {
  // Intentionally copying the name.
  return Variable(std::string(var_name).c_str(), driver);
}

float Expression::Compute() {
  switch (type) {
    case NUMBER: return float_value;
    case BINOP: return binop_compute();
    case VARIABLE: return *variable_ptr;
    case NOT: return (is_zero(subexpressions[0].Compute()) ? 1.0f : 0.0f);
    case ONEARGFUNC: {
      return one_arg_compute(subexpressions[0].Compute());
    }
    break;
    case TWOARGFUNC: {
      return two_arg_compute(subexpressions[0].Compute(),
                             subexpressions[1].Compute());
    }
    break;
    default: return 1.2345;
  }
}

bool Expression::Volatile(std::unordered_set<std::string>* volatile_deps) {
  switch (type) {
    case NUMBER: return false;
    case TWOARGFUNC:
    case BINOP: {
      // Must ensure both get called to complete volatile_deps!
      bool lhs = subexpressions[0].Volatile(volatile_deps);
      bool rhs = subexpressions[1].Volatile(volatile_deps);
      return lhs || rhs;
    }
    break;
    case VARIABLE: {
      if (volatile_inputs.find(name) == volatile_inputs.end()) {
        return false;
      } else {
        volatile_deps->insert(name);
        return true;
      }
    }
    break;
    case NOT: return subexpressions[0].Volatile(volatile_deps);
    case ONEARGFUNC: return subexpressions[0].Volatile(volatile_deps);
    default: return false;
  }
}

std::ostream& operator<<(std::ostream& os, const Expression &ex) {
  os << ex.to_string();
  return os;
}

std::string Expression::to_string() const {
  switch (type) {
    case NUMBER: return "NumberExpression(" + std::to_string(float_value) + ")";
    case BINOP: return "BinOpExpression(" + std::to_string(operation) + ", " +
        subexpressions[0].to_string() + ", " +
        subexpressions[1].to_string() + ")";
    case VARIABLE: return "VariableExpression(" + name + ")";
    default: return "Expression(some other type)";
  }
}

bool Expression::is_zero(float value) {
  return std::fabs(value) <= std::numeric_limits<float>::epsilon();
}

float Expression::bool_to_float(bool value) {
  return (value ? 1.0f : 0.0f);
}

float Expression::binop_compute() {
  float lhs = subexpressions[0].Compute();
  float rhs = subexpressions[1].Compute();
  switch (operation) {
    case AND: return !is_zero(lhs) && !is_zero(rhs);
    case OR: return !is_zero(lhs) || !is_zero(rhs);
    case PLUS: return lhs + rhs;
    case MINUS: return lhs - rhs;
    case TIMES: return lhs * rhs;
    case DIVIDE: {
      // If number is close enough to zero, don't divide by it.
      if (is_zero(rhs)) {
        return 0.0f;
      } else {
        return lhs / rhs;
      }
    }
    break;
    case EQUAL: return bool_to_float(lhs == rhs);
    case NOT_EQUAL: return bool_to_float(lhs != rhs);
    case GT: return bool_to_float(lhs > rhs);
    case GTE: return bool_to_float(lhs >= rhs);
    case LT: return bool_to_float(lhs < rhs);
    case LTE: return bool_to_float(lhs <= rhs);
    default: return -2.3456f;
  }
}

float Expression::one_arg_compute(float arg1) {
  switch (operation) {
    case ABS: return std::abs(arg1);
    case CEILING: return ceil(arg1);
    case FLOOR: return floor(arg1);
    case SIGN: return (std::signbit(arg1) ? -1.0f :
                       (Expression::is_zero(arg1) ? 0.0f: 1.0f));
    case SIN: return sin(arg1);
    default: return 3.45678f;
  }
}

float Expression::two_arg_compute(float arg1, float arg2) {
  switch (operation) {
    case MOD: return fmod(arg1, arg2);
    case MAX: return fmax(arg1, arg2);
    case MIN: return fmin(arg1, arg2);
    case POW: return pow(arg1, arg2);
    default: return 4.56789f;
  }
}

Line Line::Assignment(const std::string &variable_name, const Expression &expr, Driver* driver) {
  Line line;
  line.type = ASSIGNMENT;
  line.str1 = variable_name;
  line.variable_ptr = driver->GetVarFromName(variable_name);
  line.expr1 = expr;
  return line;
}

// loop_type is the string identifying the loop type; e.g., "for" or "all".
Line Line::Continue(const std::string &loop_type) {
  Line line;
  line.type = CONTINUE;
  line.str1 = loop_type;
  return line;
}

// loop_type is the string identifying the loop type; e.g., "for" or "all".
Line Line::Exit(const std::string &loop_type) {
  Line line;
  line.type = EXIT;
  line.str1 = loop_type;
  return line;
}

Line Line::ForNext(const Line &assign, const Expression &limit,
                   const Expression &step, const Statements &state) {
  Line line;
  line.type = FORNEXT;
  line.str1 = assign.str1;
  line.variable_ptr = assign.variable_ptr;
  line.expr1 = assign.expr1;
  line.expr2 = limit;
  line.expr3 = step;
  line.statements.push_back(state);
  return line;
}

Line Line::IfThen(const Expression &bool_expr,
                  const Statements &state1) {
  Line line;
  line.type = IFTHEN;
  line.expr1 = bool_expr;
  line.statements.push_back(state1);
  return line;
}

Line Line::IfThenElse(const Expression &bool_expr,
                      const Statements &state1,
                      const Statements &state2) {
  Line line;
  line.type = IFTHENELSE;
  line.expr1 = bool_expr;
  line.statements.push_back(state1);
  line.statements.push_back(state2);
  return line;
}

Line Line::Wait(const Expression &expr) {
  Line line;
  line.type = WAIT;
  line.expr1 = expr;
  return line;
}

std::ostream& operator<<(std::ostream& os, Line line) {
  os << "Line(" << line.str1 << ", " << line.expr1.to_string() << ")";
  return os;
}
