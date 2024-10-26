#ifndef TREE_H
#define TREE_H

#include <string>
#include <unordered_map>
#include <vector>

class VennDriver;  // Circular includes if we try to include driver.h here.

// Intermediate structures made while compiling.
struct Assignment {
  std::string field_name;
  float value;
  std::string str_value;
  bool numeric;

  static Assignment NumericAssignment(const std::string& attr, float value) {
    Assignment assign;
    assign.field_name.assign(attr);
    assign.value = value;
    assign.numeric = true;
    return assign;
  }

  static Assignment StringAssignment(const std::string& attr, const std::string& value) {
    Assignment assign;
    assign.field_name.assign(attr);

    // Remove quotes at the ends.
    if (!value.empty() && value.front() == '"' && value.back() == '"') {
        assign.str_value = value.substr(1, value.length() - 2);
    }

    assign.numeric = false;
    return assign;
  }

  // Bison seems to require this if I use const references; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const Assignment &ex);
};

struct Assignments {
  std::vector<Assignment> assignments;

  static Assignments NewAssignments(const Assignment& first) {
    Assignments assign;
    assign.assignments.push_back(first);
    return assign;
  }

  Assignments Add(const Assignment& next) {
    assignments.push_back(next);
    return *this;
  }

  // Bison seems to require this if I use const references; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const Assignments &ex);
};

struct Circle {
  float x_center, y_center;  // Values from [0, 10).
  // 0, 0 is lower left corner; 10, 10 is upper right.
  float radius;
  std::string name;
  bool present;  // Not deleted.

  Circle() : x_center{0.0}, y_center{0.0}, radius{0.0} {}

  // Must be defined in tree.cc, because it actually uses VennDriver.
  static Circle NewCircle(const std::string& name, const Assignments& fields, VennDriver* driver);

  // Bison seems to require this if I use const references; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const Circle &ex);

  std::string AnAssignment(const char* name, float value) {
    std::string result(name);
    result.append(" = ");
    result.append(std::to_string(value));
    result.append("\n");
    return result;
  }  

  // Replaces any \n in the text with a newline.
  // This allows us to do the reverse when we output the
  // text version of the diagram.
  void setNameFromDiagram(const char* new_name) {
    name = new_name;

    size_t pos = 0;
    while ((pos = name.find("\\n", pos)) != std::string::npos) {
        name.replace(pos, 2, "\n");
        pos += 1; // Move past the replaced part
    }
  }

  const std::string to_string() {
    std::string result("[");
    result.append("]\n");

    result.append(AnAssignment("x", x_center));
    result.append(AnAssignment("y", y_center));
    result.append(AnAssignment("radius", radius));
    result.append(AnAssignment("present", present ? 1 : 0));
    result.append("name = \"");
    std::string new_name = name;

    size_t pos = 0;
    while ((pos = new_name.find("\n", pos)) != std::string::npos) {
        new_name.replace(pos, 1, "\\n");
        pos += 2; // Move past the replaced part
    }
    result.append(new_name);
    result.append("\"\n");
    return result;
  }
};

struct CircleList {
  std::vector<Circle> circles;

  static CircleList NewCircleList(const Circle& first) {
    CircleList list;
    list.circles.push_back(first);
    return list;
  }

  CircleList Add(const Circle& next) {
    circles.push_back(next);
    return *this;
  }

  // Bison seems to require this if I use const references; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const CircleList &ex);
};

// Like "Venn Diagram", get it? Ehhh?
struct Diagram {
  std::vector<Circle> circles;

  // Bison seems to require this if I use const references; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const Diagram &ex);
};

class VennDriver;
// Base class for computing expressions.
class VennExpression {
 public:
  enum Type {
    NUMBER,  // 3, 4.5, -283823
    BINOP,   // plus, times
    VARIABLE, // in1, out1, foo
    NOT,      // not bool
    ONEARGFUNC, // operation (subexpressions[0])
    TWOARGFUNC, // func2(subexpressions[0], subexpressions[1])
    TERNARYFUNC, // subexpressions[0] ? subexpressions[1] : subexpressions[2]
  };
  Type type;
  // Which method/operation is this?
  enum Operation {
    PLUS,
    MINUS,
    TIMES,
    DIVIDE,
    EQUAL,
    NOT_EQUAL,
    GT,
    GTE,
    LT,
    LTE,
    AND,
    OR,
    ABS,
    CEILING,
    FLOOR,
    LOG2,
    LOGE,
    LOG10,
    SIGN,
    SIN,
    MOD,
    MAX,
    MIN,
    POW
  };
  Operation operation;
  float float_value;
  // Some variables are just pointers to a float (e.g., i, foo, etc.).
  float* variable_ptr;

  std::string name;
  std::vector<VennExpression> subexpressions;

  static std::unordered_map<std::string, float> note_to_volt_octave_4;
  VennExpression() : variable_ptr{nullptr} {}

  // Compute the float numeric result of this VennExpression.
  float Compute();

  static bool is_zero(float value);

  // Don't force users to understand that comparing floats is hard.
  static bool float_equal(float f1, float f2);

  // Bison seems to require this; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const VennExpression &ex);
  std::string to_string() const;
 private:
  // logX(y) functions don't have useful values for Y <= 0.
  // So we'll return 0. This function turns any Y <= 0 into 1, thus causing
  // a log function to return 0.
  float SafeLogArg(float arg);
  float bool_to_float(bool value);
  float binop_compute();
  float one_arg_compute(float arg1);
  float two_arg_compute(float arg1, float arg2);
};

class VennExpressionFactory {
 public:
  VennExpression Not(const VennExpression &expr);
  VennExpression Note(const std::string &note_name);
  VennExpression Number(float the_value);
  VennExpression OneArgFunc(const std::string &func_name,
                            const VennExpression &arg1);
  VennExpression TwoArgFunc(const std::string &func_name,
                            const VennExpression &arg1, const VennExpression &arg2);
  VennExpression TernaryFunc(const VennExpression &condition, const VennExpression &if_true,
                             const VennExpression &if_false);
  VennExpression CreateBinOp(const VennExpression &lhs,
                             const std::string &op_string,
                             const VennExpression &rhs);
  VennExpression Variable(const char *var_name, VennDriver* driver);
  // The parser seems to need many variants of Variable.
  VennExpression Variable(const std::string &expr, VennDriver* driver);
  VennExpression Variable(char * var_name, VennDriver* driver);
 private:
  static std::unordered_map<std::string, VennExpression::Operation> string_to_operation;
  static std::unordered_map<std::string, float> note_to_volt_same_octave;
};


#endif // TREE_H