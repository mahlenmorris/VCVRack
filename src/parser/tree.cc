// Code for methods related to parsing data structures.
#include "tree.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <vector>
#include "driver.hh"

std::unordered_map<std::string, Expression::Operation> ExpressionFactory::string_to_operation = {
  {"+", Expression::PLUS},
  {"-", Expression::MINUS},
  {"*", Expression::TIMES},
  {"/", Expression::DIVIDE},
  {"==", Expression::EQUAL},
  {"!=", Expression::NOT_EQUAL},
  {">", Expression::GT},
  {">=", Expression::GTE},
  {"<", Expression::LT},
  {"<=", Expression::LTE},
  {"and", Expression::AND},
  {"or", Expression::OR},
  {"abs", Expression::ABS},
  {"ceiling", Expression::CEILING},
  {"connected", Expression::CONNECTED},
  {"floor", Expression::FLOOR},
  {"log2", Expression::LOG2},
  {"loge", Expression::LOGE},
  {"log10", Expression::LOG10},
  {"normal", Expression::NORMAL},
  {"random", Expression::RANDOM},
  {"sample_rate", Expression::SAMPLE_RATE},
  {"sign", Expression::SIGN},
  {"sin", Expression::SIN},
  {"start", Expression::START},
  {"trigger", Expression::TRIGGER},
  {"mod", Expression::MOD},
  {"max", Expression::MAX},
  {"min", Expression::MIN},
  {"pow", Expression::POW}
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

float Expression::Compute() {
  switch (type) {
    case NUMBER: return float_value;
    case BINOP: return binop_compute();
    case ARRAY_VARIABLE: {
      int index = (int) floor(subexpressions[0].Compute());
      if ((index < 0) || (index >= (int) array_ptr->size())) {
        return 0.0f;  // The default value if not in the array.
      }
      return array_ptr->at(index);
    }
    break;
    case VARIABLE: {
      if (port.port_type == PortPointer::NOT_PORT) {
        return *variable_ptr;
      } else {
        // Rather than constantly refreshing the INn variable names, even when
        // they may not be read, we just allow them to be read directly from
        // the module.
        return env->GetVoltage(port);
      }
    }
    break;
    case NOT: return (is_zero(subexpressions[0].Compute()) ? 1.0f : 0.0f);
    case ZEROARGFUNC: {
      return zero_arg_compute();
    }
    break;
    case ONEARGFUNC: {
      return one_arg_compute(subexpressions[0].Compute());
    }
    break;
    case ONEPORTFUNC: {
      switch (operation) {
        case CONNECTED: return env->Connected(port);
        case TRIGGER: return env->Trigger(port) ? 1.0f : 0.0f;
        default: return -8.642f;
      }
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

bool Expression::Volatile() {
  switch (type) {
    case NUMBER: return false;
    case TWOARGFUNC:
    case BINOP: {
      // Must ensure both get called to complete volatile_deps!
      bool lhs = subexpressions[0].Volatile();
      bool rhs = subexpressions[1].Volatile();
      return lhs || rhs;
    }
    break;
    case ARRAY_VARIABLE: return subexpressions[0].Volatile();
    case VARIABLE: {
      return port.port_type == PortPointer::INPUT;
    }
    break;
    case NOT: return subexpressions[0].Volatile();
    // sample_rate() doesn't seem to change immediately? But that might be
    // a bug or Windows-specific. And Start() is volatile.
    case ZEROARGFUNC: return true;
    case ONEARGFUNC: return subexpressions[0].Volatile();
    // Yes, both conneted() and trigger are volatile.
    case ONEPORTFUNC: return true;
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
    case ARRAY_VARIABLE: return "ArrayVariable(" + name + ")";
    case VARIABLE: return "VariableExpression(" + name + ")";
    default: return "Expression(type = " + std::to_string(type) +
                    ", operation = " + std::to_string(operation) + ")";
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

float Expression::zero_arg_compute() {
  switch (operation) {
    case SAMPLE_RATE: return env->SampleRate();
    case START: return env->Start() ? 1.0f : 0.0f;
    default: return -9.87654f;
  }
}

// logX(y) functions don't have useful values for Y <= 0.
// So we'll return 0. This function turns any Y <= 0 into 1, thus causing
// a log function to return 0.
float SafeLogArg(float arg) {
  return (arg < 0.0f || Expression::is_zero(arg)) ? 1.0f : arg;
}

float Expression::one_arg_compute(float arg1) {
  switch (operation) {
    case ABS: return std::abs(arg1);
    case CEILING: return ceil(arg1);
    case FLOOR: return floor(arg1);
    case LOG2: return log2(SafeLogArg(arg1));
    case LOGE: return log(SafeLogArg(arg1));
    case LOG10: return log10(SafeLogArg(arg1));
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
    case NORMAL: return env->Normal(arg1, arg2);
    case POW: return pow(arg1, arg2);
    case RANDOM: return env->Random(arg1, arg2);
    default: return 4.56789f;
  }
}

Expression ExpressionFactory::Not(const Expression &expr) {
  Expression ex;
  ex.type = Expression::NOT;
  ex.subexpressions.push_back(expr);
  return ex;
}

Expression ExpressionFactory::Note(const std::string &note_name) {
  Expression ex;
  ex.type = Expression::NUMBER;
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

Expression ExpressionFactory::Number(float the_value) {
  Expression ex;
  ex.type = Expression::NUMBER;
  ex.float_value = the_value;
  return ex;
}

Expression ExpressionFactory::ZeroArgFunc(const std::string &func_name) {
  Expression ex;
  ex.type = Expression::ZEROARGFUNC;
  ex.operation = string_to_operation.at(func_name);
  ex.env = env;  // We know sample_rate() requires this.
  return ex;
}

Expression ExpressionFactory::OneArgFunc(const std::string &func_name,
                                         const Expression &arg1) {
  Expression ex;
  ex.type = Expression::ONEARGFUNC;
  ex.operation = string_to_operation.at(func_name);
  ex.subexpressions.push_back(arg1);
  return ex;
}

Expression ExpressionFactory::OnePortFunc(const std::string &func_name,
                                          const std::string &port1,
                                          Driver* driver) {
  Expression ex;
  ex.type = Expression::ONEPORTFUNC;
  ex.operation = string_to_operation.at(func_name);
  ex.port = driver->GetPortFromName(port1);  // TODO: Make parser do this?
  if (ex.operation == Expression::TRIGGER) {
    // So that Environment can know to inspect these ports every sample.
    driver->trigger_port_indexes.insert(ex.port.index);
  }
  ex.env = env;
  return ex;
}

Expression ExpressionFactory::TwoArgFunc(const std::string &func_name,
                                         const Expression &arg1,
                                         const Expression &arg2) {
  Expression ex;
  ex.type = Expression::TWOARGFUNC;
  ex.operation = string_to_operation.at(func_name);
  ex.env = env;  // Sometimes we need this.
  ex.subexpressions.push_back(arg1);
  ex.subexpressions.push_back(arg2);
  return ex;
}

Expression ExpressionFactory::CreateBinOp(const Expression &lhs,
                                          const std::string &op_string,
                                          const Expression &rhs) {
  Expression ex;
  ex.type = Expression::BINOP;
  ex.subexpressions.push_back(lhs);
  ex.subexpressions.push_back(rhs);
  ex.operation = string_to_operation.at(op_string);
  return ex;
}

Expression ExpressionFactory::ArrayVariable(const std::string &array_name,
                                            const Expression &arg1,
                                            Driver* driver) {
  Expression ex;
  ex.type = Expression::ARRAY_VARIABLE;
  ex.array_ptr = driver->GetArrayFromName(array_name);
  ex.subexpressions.push_back(arg1);
  return ex;
}

// TODO: Now that compiler knows if var_name is a port or not, could
// avoid deciding here. Or make a new kind of Expression called PORT!?
Expression ExpressionFactory::Variable(const char *var_name, Driver* driver) {
  Expression ex;
  ex.type = Expression::VARIABLE;
  // Intentionally copying the name.
  ex.name = std::string(var_name);
  if (driver->VarHasPort(var_name)) {
    ex.port = driver->GetPortFromName(ex.name);
    ex.env = env;
  } else {
    ex.variable_ptr = driver->GetVarFromName(ex.name);
  }
  return ex;
}
// The parser seems to need many variants of Variable.
Expression ExpressionFactory::Variable(const std::string &expr, Driver* driver) {
  // Intentionally copying the name.
  return Variable(expr.c_str(), driver);
}

Expression ExpressionFactory::Variable(char* var_name, Driver* driver) {
  // Intentionally copying the name.
  return Variable(std::string(var_name).c_str(), driver);
}

Line Line::ArrayAssignment(const std::string &variable_name,
                     const Expression &index,
                     const Expression &value, Driver* driver) {
  Line line;
  line.type = ARRAY_ASSIGNMENT;
  line.str1 = variable_name;  // Not required, but handy for troubleshooting.
  line.array_ptr = driver->GetArrayFromName(variable_name);
  line.expr1 = index;
  line.expr2 = value;
  return line;
}

Line Line::ArrayAssignment(const std::string &variable_name,
                     const Expression &index,
                     const ExpressionList &values, Driver* driver) {
  Line line;
  line.type = ARRAY_ASSIGNMENT;
  line.str1 = variable_name;  // Not required, but handy for troubleshooting.
  line.array_ptr = driver->GetArrayFromName(variable_name);
  line.expr1 = index;
  line.expr_list = values;
  return line;
}

Line Line::Assignment(const std::string &variable_name, const Expression &expr,
                      Driver* driver) {
  Line line;
  line.type = ASSIGNMENT;
  line.str1 = variable_name;
  if (driver->VarHasPort(variable_name)) {
    line.assign_port = driver->GetPortFromName(variable_name);
    line.variable_ptr = nullptr;
  } else {
    line.variable_ptr = driver->GetVarFromName(variable_name);
  }
  line.expr1 = expr;
  return line;
}

Line Line::ClearAll() {
  Line line;
  line.type = CLEAR;
  // ALL is the only type of CLEAR we have at the moment, so
  // no need to qualify.
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
                   const Expression &step, const Statements &state,
                   Driver* driver) {
  Line line;
  line.type = FORNEXT;
  line.str1 = assign.str1;
  if (driver->VarHasPort(line.str1)) {
    line.assign_port = driver->GetPortFromName(line.str1);
    line.variable_ptr = nullptr;
  } else {
    line.variable_ptr = driver->GetVarFromName(line.str1);
  }
  line.expr1 = assign.expr1;
  line.expr2 = limit;
  line.expr3 = step;
  line.statements.push_back(state);
  return line;
}

Line Line::ElseIf(const Expression &bool_expr,
                  const Statements &state1) {
  Line line;
  line.type = ELSEIF;
  line.expr1 = bool_expr;
  line.statements.push_back(state1);
  return line;
}

Line Line::IfThen(const Expression &bool_expr,
                  const Statements &then_state,
                  const Statements &elseifs) {
  Line line;
  line.type = IFTHEN;
  line.expr1 = bool_expr;
  line.statements.push_back(then_state);
  line.statements.push_back(elseifs);
  return line;
}

Line Line::IfThenElse(const Expression &bool_expr,
                      const Statements &then_state,
                      const Statements &else_state,
                      const Statements &elseifs) {
  Line line;
  line.type = IFTHENELSE;
  line.expr1 = bool_expr;
  line.statements.push_back(then_state);
  line.statements.push_back(else_state);
  line.statements.push_back(elseifs);
  return line;
}

Line Line::Reset() {
  Line line;
  line.type = RESET;
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
