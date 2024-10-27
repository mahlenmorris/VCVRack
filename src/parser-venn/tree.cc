// Code for the Diagram data structure being built.
#include "tree.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
#include "driver.h"

// TODO: remove unneeded items in this map.
std::unordered_map<std::string, VennExpression::Operation> VennExpressionFactory::string_to_operation = {
  {"+", VennExpression::PLUS},
  {"-", VennExpression::MINUS},
  {"*", VennExpression::TIMES},
  {"/", VennExpression::DIVIDE},
  {"==", VennExpression::EQUAL},
  {"!=", VennExpression::NOT_EQUAL},
  {">", VennExpression::GT},
  {">=", VennExpression::GTE},
  {"<", VennExpression::LT},
  {"<=", VennExpression::LTE},
  {"and", VennExpression::AND},
  {"or", VennExpression::OR},
  {"abs", VennExpression::ABS},
  {"ceiling", VennExpression::CEILING},
  {"floor", VennExpression::FLOOR},
  {"log2", VennExpression::LOG2},
  {"loge", VennExpression::LOGE},
  {"log10", VennExpression::LOG10},
  {"sign", VennExpression::SIGN},
  {"sin", VennExpression::SIN},
  {"mod", VennExpression::MOD},
  {"max", VennExpression::MAX},
  {"min", VennExpression::MIN},
  {"pow", VennExpression::POW}
};

std::unordered_map<std::string, float> VennExpressionFactory::note_to_volt_same_octave = {
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
      circle.name = ReplaceWithNewline(assign.str_value.c_str());
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

float VennExpression::Compute() {
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
    case LIMIT: {
      float low = subexpressions[1].Compute();
      float high = subexpressions[2].Compute();
      if (low > high) {
        float temp = high;
        high = low;
        low = temp;
      }
      return fmax(fmin(subexpressions[0].Compute(), high), low);
    }
    break;
    case SCALE: {
      float originStart = subexpressions[1].Compute();
      float originEnd = subexpressions[2].Compute();
      float destStart = subexpressions[3].Compute();
      float destEnd = subexpressions[4].Compute();
      double scale = SafeDivide(destEnd - destStart, originEnd - originStart);
      double offset = -originStart * scale + destStart;
      return subexpressions[0].Compute() * scale + offset;
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
std::ostream& operator<<(std::ostream& os, const VennExpression &ex) {
  os << ex.to_string();
  return os;
}

// TODO: Not clear I need this.
std::string VennExpression::to_string() const {
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

bool VennExpression::is_zero(float value) {
  return std::fabs(value) <= std::numeric_limits<float>::epsilon();
}

bool VennExpression::float_equal(float f1, float f2) {
    static constexpr auto epsilon = 1.0e-05f;
    if (std::fabs(f1 - f2) <= epsilon)
        return true;
    return std::fabs(f1 - f2) <= epsilon * fmax(std::fabs(f1), std::fabs(f2));
}

float VennExpression::bool_to_float(bool value) {
  return (value ? 1.0f : 0.0f);
}

float VennExpression::SafeDivide(float lhs, float rhs) {
  // If number is close enough to zero, don't divide by it.
  if (is_zero(rhs)) {
    return 0.0f;
  } else {
    return lhs / rhs;
  }
}

float VennExpression::binop_compute() {
  float lhs = subexpressions[0].Compute();
  float rhs = subexpressions[1].Compute();
  switch (operation) {
    case AND: return !is_zero(lhs) && !is_zero(rhs);
    case OR: return !is_zero(lhs) || !is_zero(rhs);
    case PLUS: return lhs + rhs;
    case MINUS: return lhs - rhs;
    case TIMES: return lhs * rhs;
    case DIVIDE: return SafeDivide(lhs, rhs);
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
float VennExpression::SafeLogArg(float arg) {
  return (arg < 0.0f || VennExpression::is_zero(arg)) ? 1.0f : arg;
}

float VennExpression::one_arg_compute(float arg1) {
  switch (operation) {
    case ABS: return std::abs(arg1);
    case CEILING: return ceil(arg1);
    case FLOOR: return floor(arg1);
    case LOG2: return log2(SafeLogArg(arg1));
    case LOGE: return log(SafeLogArg(arg1));
    case LOG10: return log10(SafeLogArg(arg1));
    case SIGN: return (std::signbit(arg1) ? -1.0f :
                       (VennExpression::is_zero(arg1) ? 0.0f: 1.0f));
    case SIN: return sin(arg1);
    default: return 3.45678f;
  }
}

float VennExpression::two_arg_compute(float arg1, float arg2) {
  switch (operation) {
    case MOD: return fmod(arg1, arg2);
    case MAX: return fmax(arg1, arg2);
    case MIN: return fmin(arg1, arg2);
    case POW: return pow(arg1, arg2);
    default: return 4.56789f;
  }
}

VennExpression VennExpressionFactory::Not(const VennExpression &expr) {
  VennExpression ex;
  ex.type = VennExpression::NOT;
  ex.subexpressions.push_back(expr);
  return ex;
}

VennExpression VennExpressionFactory::Note(const std::string &note_name) {
  VennExpression ex;
  ex.type = VennExpression::NUMBER;
  std::string lower;
  VennToLower(note_name, &lower);
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

VennExpression VennExpressionFactory::Number(float the_value) {
  VennExpression ex;
  ex.type = VennExpression::NUMBER;
  ex.float_value = the_value;
  return ex;
}

VennExpression VennExpressionFactory::OneArgFunc(const std::string &func_name,
                                         const VennExpression &arg1) {
  VennExpression ex;
  ex.type = VennExpression::ONEARGFUNC;
  std::string lower;
  VennToLower(func_name, &lower);
  ex.operation = string_to_operation.at(lower);
  ex.subexpressions.push_back(arg1);
  return ex;
}

VennExpression VennExpressionFactory::TwoArgFunc(const std::string &func_name,
                                         const VennExpression &arg1,
                                         const VennExpression &arg2) {
  VennExpression ex;
  ex.type = VennExpression::TWOARGFUNC;
  std::string lower;
  VennToLower(func_name, &lower);
  ex.operation = string_to_operation.at(lower);
  ex.subexpressions.push_back(arg1);
  ex.subexpressions.push_back(arg2);
  return ex;
}

VennExpression VennExpressionFactory::Limit(const VennExpression &value, const VennExpression &start,
                      const VennExpression &end) {
  VennExpression ex;
  ex.type = VennExpression::LIMIT;
  ex.subexpressions.push_back(value);
  ex.subexpressions.push_back(start);
  ex.subexpressions.push_back(end);
  return ex;
}

VennExpression VennExpressionFactory::Scale(const VennExpression &value,
                     const VennExpression &originStart, const VennExpression &originEnd,
                     const VennExpression &destStart, const VennExpression &destEnd) {
  VennExpression ex;
  ex.type = VennExpression::SCALE;
  ex.subexpressions.push_back(value);
  ex.subexpressions.push_back(originStart);
  ex.subexpressions.push_back(originEnd);
  ex.subexpressions.push_back(destStart);
  ex.subexpressions.push_back(destEnd);
  return ex;
}

VennExpression VennExpressionFactory::TernaryFunc(const VennExpression &condition, const VennExpression &if_true,
                                          const VennExpression &if_false){
  VennExpression ex;
  ex.type = VennExpression::TERNARYFUNC;
  ex.subexpressions.push_back(condition);
  ex.subexpressions.push_back(if_true);
  ex.subexpressions.push_back(if_false);
  return ex;
}

VennExpression VennExpressionFactory::CreateBinOp(const VennExpression &lhs,
                                          const std::string &op_string,
                                          const VennExpression &rhs) {
  VennExpression ex;
  ex.type = VennExpression::BINOP;
  ex.subexpressions.push_back(lhs);
  ex.subexpressions.push_back(rhs);
  std::string lower;
  VennToLower(op_string, &lower);
  ex.operation = string_to_operation.at(lower);
  return ex;
}

// Venn has a restricted number of variables, unlike BASICally.
VennExpression VennExpressionFactory::Variable(const char *var_name, VennDriver* driver) {
  VennExpression ex;
  ex.type = VennExpression::VARIABLE;
  // Intentionally copying the name.
  std::string copied(var_name);
  VennToLower(copied, &(ex.name));
  if (driver->IsVariableName(ex.name.c_str())) {
    ex.variable_ptr = driver->GetVarFromName(ex.name.c_str());
  } else {
    // Make a compile error. Though this shouldn't happen if the compiler is correct.
    driver->AddError("There is no variable called '" + copied + "'.");
  }
  return ex;
}

// The parser seems to need many variants of Variable.
VennExpression VennExpressionFactory::Variable(const std::string &expr, VennDriver* driver) {
  // Intentionally copying the name.
  return Variable(expr.c_str(), driver);
}

VennExpression VennExpressionFactory::Variable(char* var_name, VennDriver* driver) {
  // Intentionally copying the name, since I'm not confident the char* is
  // valid for long.
  return Variable(std::string(var_name).c_str(), driver);
}

