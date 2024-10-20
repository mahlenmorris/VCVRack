// Code for the Diagram data structure being built.
#include "tree.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
#include "driver.h"

// TODO: remove unneeded items in this map.
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
  {"floor", Expression::FLOOR},
  {"log2", Expression::LOG2},
  {"loge", Expression::LOGE},
  {"log10", Expression::LOG10},
  {"sign", Expression::SIGN},
  {"sin", Expression::SIN},
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

void ToLower(const std::string &mixed, std::string *lower) {
  lower->resize(mixed.size());
  std::transform(mixed.begin(), mixed.end(),
                 lower->begin(), ::tolower);
}

void VennToLower(const std::string &mixed, std::string *lower) {
  lower->resize(mixed.size());
  std::transform(mixed.begin(), mixed.end(),
                 lower->begin(), ::tolower);
}

Circle Circle::NewCircle(const std::string& name, const Assignments& fields, VennDriver* driver) {
  Circle circle;
  circle.name.assign(name);
  circle.present = true;  // To allow me to read my development saves.
  for (Assignment assign : fields.assignments) {
    std::string lower_name;
    VennToLower(assign.field_name, &lower_name);
    if (lower_name.compare("x") == 0) {
      circle.x_center = assign.value;
    } else if (lower_name.compare("y") == 0) {
      circle.y_center = assign.value;
    } else if (lower_name.compare("radius") == 0) {
      circle.radius = assign.value;
    } else if (lower_name.compare("present") == 0) {
      circle.present = assign.value > 0;  // a boolean.
    } else if (lower_name.compare("name") == 0) {
      circle.name = assign.str_value;  // TODO: character conversion, like "\n" -> \n.
    } else {
      driver->AddError("I don't know what a '" + assign.field_name + "' is.");
    }
  }
  return circle;
}

std::ostream& operator<<(std::ostream& os, const Assignment &ex) {
  os << ex.field_name << " = " << ex.value;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Assignments &ex) {
  for (auto assign : ex.assignments) {
    os << assign;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Circle &ex) {
  os << ex.name << ", " << ex.x_center << ", " << ex.y_center << ", " << ex.radius;
  return os;
}

std::ostream& operator<<(std::ostream& os, const CircleList &ex) {
  for (auto assign : ex.circles) { 
    os << assign;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Diagram &ex) {
  for (auto assign : ex.circles) { 
    os << assign;
  }
  return os;
}

float Expression::Compute() {
  switch (type) {
    case NUMBER: return float_value;
    case BINOP: return binop_compute();
    case VARIABLE: return *variable_ptr;
    case NOT: return (is_zero(subexpressions[0].Compute()) ? 1.0f : 0.0f);
    break;
    case ONEARGFUNC: {
      return one_arg_compute(subexpressions[0].Compute());
    }
    break;
    case TWOARGFUNC: {
      return two_arg_compute(subexpressions[0].Compute(),
                             subexpressions[1].Compute());
    }
    break;
    case TERNARYFUNC: {
      if (is_zero(subexpressions[0].Compute())) {
        return subexpressions[2].Compute();
      } else {
        return subexpressions[1].Compute();
      }
    }
    break;
    default: return 1.2345;
  }
}

// TODO: Not clear I need this.
std::ostream& operator<<(std::ostream& os, const Expression &ex) {
  os << ex.to_string();
  return os;
}

// TODO: Not clear I need this.
std::string Expression::to_string() const {
  switch (type) {
    case NUMBER: return "NumberExpression(" + std::to_string(float_value) + ")";
    case BINOP: return "BinOpExpression(" + std::to_string(operation) + ", " +
        subexpressions[0].to_string() + ", " +
        subexpressions[1].to_string() + ")";
    case VARIABLE: return "VariableExpression(" + name + ")";
    default: return "Expression(type = " + std::to_string(type) +
                    ", operation = " + std::to_string(operation) + ")";
  }
}

bool Expression::is_zero(float value) {
  return std::fabs(value) <= std::numeric_limits<float>::epsilon();
}

bool Expression::float_equal(float f1, float f2) {
    static constexpr auto epsilon = 1.0e-05f;
    if (std::fabs(f1 - f2) <= epsilon)
        return true;
    return std::fabs(f1 - f2) <= epsilon * fmax(std::fabs(f1), std::fabs(f2));
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
    case EQUAL: return bool_to_float(float_equal(lhs, rhs));
    case NOT_EQUAL: return bool_to_float(!float_equal(lhs, rhs));
    case GT: return bool_to_float(lhs > rhs);
    case GTE: return bool_to_float(lhs >= rhs);
    case LT: return bool_to_float(lhs < rhs);
    case LTE: return bool_to_float(lhs <= rhs);
    default: return -2.3456f;
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
    case POW: return pow(arg1, arg2);
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
  std::string lower;
  ToLower(note_name, &lower);
  // split into name and octave
  int octave;
  std::string name;
  // Number at end might be two chars long, in the case of -1 or 10.
  if (lower.size() == 4 || (lower.size() == 3 && (
      lower[1] != '#' && lower[1] != 'b'))) {
    name = lower.substr(0, lower.size() - 2);
    octave = strtol(lower.c_str() + lower.size() - 2, NULL, 10);
  } else {
    name = lower.substr(0, lower.size() - 1);
    octave = strtol(lower.c_str() + lower.size() - 1, NULL, 10);
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

Expression ExpressionFactory::OneArgFunc(const std::string &func_name,
                                         const Expression &arg1) {
  Expression ex;
  ex.type = Expression::ONEARGFUNC;
  std::string lower;
  ToLower(func_name, &lower);
  ex.operation = string_to_operation.at(lower);
  ex.subexpressions.push_back(arg1);
  return ex;
}

Expression ExpressionFactory::TwoArgFunc(const std::string &func_name,
                                         const Expression &arg1,
                                         const Expression &arg2) {
  Expression ex;
  ex.type = Expression::TWOARGFUNC;
  std::string lower;
  ToLower(func_name, &lower);
  ex.operation = string_to_operation.at(lower);
  ex.subexpressions.push_back(arg1);
  ex.subexpressions.push_back(arg2);
  return ex;
}

Expression ExpressionFactory::TernaryFunc(const Expression &condition, const Expression &if_true,
                                          const Expression &if_false){
  Expression ex;
  ex.type = Expression::TERNARYFUNC;
  ex.subexpressions.push_back(condition);
  ex.subexpressions.push_back(if_true);
  ex.subexpressions.push_back(if_false);
  return ex;
}

Expression ExpressionFactory::CreateBinOp(const Expression &lhs,
                                          const std::string &op_string,
                                          const Expression &rhs) {
  Expression ex;
  ex.type = Expression::BINOP;
  ex.subexpressions.push_back(lhs);
  ex.subexpressions.push_back(rhs);
  std::string lower;
  ToLower(op_string, &lower);
  ex.operation = string_to_operation.at(lower);
  return ex;
}

// Venn has a restricted number of variables, unlike BASICally.
Expression ExpressionFactory::Variable(const char *var_name, VennDriver* driver) {
  Expression ex;
  ex.type = Expression::VARIABLE;
  // Intentionally copying the name.
  std::string copied(var_name);
  ToLower(copied, &(ex.name));
  if (driver->IsVariableName(ex.name.c_str())) {
    ex.variable_ptr = driver->GetVarFromName(ex.name.c_str());
  } else {
    // Make a compile error. Though this shouldn't happen if the compiler is correct.
    driver->AddError("There is no variable called '" + copied + "'.");
  }
  return ex;
}

// The parser seems to need many variants of Variable.
Expression ExpressionFactory::Variable(const std::string &expr, VennDriver* driver) {
  // Intentionally copying the name.
  return Variable(expr.c_str(), driver);
}

Expression ExpressionFactory::Variable(char* var_name, VennDriver* driver) {
  // Intentionally copying the name, since I'm not confident the char* is
  // valid for long.
  return Variable(std::string(var_name).c_str(), driver);
}

