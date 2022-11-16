// Code for methods related to parsing data structures.
#include "tree.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <vector>
#include "environment.h"

std::map<std::string, Expression::Operation> Expression::string_to_operation = {
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
};

Expression Expression::Not(const Expression &expr) {
  Expression ex;
  ex.type = NOT;
  ex.left_right.push_back(expr);
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
  ex.name = func_name;
  ex.left_right.push_back(arg1);
  return ex;
}

Expression Expression::CreateBinOp(const Expression &lhs,
                                   const std::string &op_string,
                                   const Expression &rhs) {
  Expression ex;
  ex.type = BINOP;
  ex.left_right.push_back(lhs);
  ex.left_right.push_back(rhs);
  ex.operation = string_to_operation.at(op_string);
  return ex;
}

Expression Expression::Variable(const char *var_name) {
  // Intentionally copying the name.
  Expression ex;
  ex.type = VARIABLE;
  ex.name = std::string(var_name);
  return ex;
}
// The parser seems to need many variants of Variable.
Expression Expression::Variable(const Expression &expr) {
  // Intentionally copying the name.
  return Variable(expr.name.c_str());
}
// The parser seems to need many variants of Variable.
Expression::Expression(char * var_name) {
  // Intentionally copying the name.
  name = std::string(var_name);
}

Expression Expression::Variable(char* var_name) {
  // Intentionally copying the name.
  return Variable(std::string(var_name).c_str());
}

float Expression::Compute(Environment* env) {
  switch (type) {
    case NUMBER: return float_value;
    case BINOP: return binop_compute(env);
    case VARIABLE: {
      // TODO: This operation actually depends on the name. Make this correct.
      // If we compile as the user types, many spurious variable names will be
      // asked for. Make sure we don't create spurious entries.
      auto found = env->variables.find(name);
      if (found != env->variables.end()) {
        return found->second;
      } else {
        return 0.0f;
      }
    }
    break;
    case NOT: return (is_zero(left_right[0].Compute(env)) ? 1.0f : 0.0f);
    case ONEARGFUNC: {
      float arg1 = left_right[0].Compute(env);
      // TODO: do this more efficiently.
      if (name == "abs") return std::abs(arg1);
      if (name == "ceiling") return ceil(arg1);
      if (name == "floor") return floor(arg1);
      if (name == "sign") return (std::signbit(arg1) ? -1.0f : 1.0f);
      if (name == "sin") return sin(arg1);
      // TODO: Hey, shouldn't happen!
      return -0.98765f;
    }
    default: return 1.2345;
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
        left_right[0].to_string() + ", " + left_right[1].to_string() + ")";
    case VARIABLE: return "VariableExpression(" + name + ")";
    default: return "Expression(ERR: Unknown type!)";
  }
}

bool Expression::is_zero(float value) {
  return std::fabs(value) <= std::numeric_limits<float>::epsilon();
}

float Expression::bool_to_float(bool value) {
  return (value ? 1.0f : 0.0f);
}

float Expression::binop_compute(Environment* env) {
  float lhs = left_right[0].Compute(env);
  float rhs = left_right[1].Compute(env);
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
    default: return -2.3456;
  }
}
