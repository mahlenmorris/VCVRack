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

Expression Expression::Not(const Expression &expr) {
  Expression ex;
  ex.type = NOT;
  ex.subexpressions.push_back(expr);
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
    case NOT: return (is_zero(subexpressions[0].Compute(env)) ? 1.0f : 0.0f);
    case ONEARGFUNC: {
      return one_arg_compute(subexpressions[0].Compute(env));
    }
    break;
    case TWOARGFUNC: {
      return two_arg_compute(subexpressions[0].Compute(env),
                             subexpressions[1].Compute(env));
    }
    break;
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
        subexpressions[0].to_string() + ", " + subexpressions[1].to_string() + ")";
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
  float lhs = subexpressions[0].Compute(env);
  float rhs = subexpressions[1].Compute(env);
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
