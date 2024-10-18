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

  // Must be defined in tree.cc, because it actually uses Driver.
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

  const std::string to_string() {
    std::string result("[");
    result.append("]\n");

    result.append(AnAssignment("x", x_center));
    result.append(AnAssignment("y", y_center));
    result.append(AnAssignment("radius", radius));
    result.append(AnAssignment("present", present ? 1 : 0));
    result.append("name = \"");
    result.append(name);  // TODO: do some character conversion, like \n -> "\n"?
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

class Driver;
// Base class for computing expressions.
class Expression {
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
  std::vector<Expression> subexpressions;

  static std::unordered_map<std::string, float> note_to_volt_octave_4;
  Expression() : variable_ptr{nullptr} {}

  // Compute the float numeric result of this Expression.
  float Compute();

  // Compute the string version of this expression.
  std::string ComputeString();
  // Determine if this Expression can Compute() different results depending on
  // INn or any other volatile source (e.g., a random() function.)
  bool Volatile();

  static bool is_zero(float value);

  // Don't force users to understand that comparing floats is hard.
  static bool float_equal(float f1, float f2);

  // Bison seems to require this; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const Expression &ex);
  std::string to_string() const;
 private:
  float bool_to_float(bool value);
  float binop_compute();
  float one_arg_compute(float arg1);
  float two_arg_compute(float arg1, float arg2);
};

class ExpressionFactory {
 public:
 /*
  Expression Not(const Expression &expr);
  Expression Note(const std::string &note_name);
 */
  Expression Number(float the_value);
 /*
  Expression Quoted(const std::string &quoted);
  Expression OneArgFunc(const std::string &func_name,
                        const Expression &arg1);
  Expression TwoArgFunc(const std::string &func_name,
                        const Expression &arg1, const Expression &arg2);
  Expression TernaryFunc(const Expression &condition, const Expression &if_true,
                         const Expression &if_false);
  Expression CreateBinOp(const Expression &lhs,
                         const std::string &op_string,
                         const Expression &rhs);
  Expression Variable(const char *var_name, Driver* driver);
  // The parser seems to need many variants of Variable.
  Expression Variable(const std::string &expr, Driver* driver);
  Expression Variable(char * var_name, Driver* driver);
 */
 private:
  static std::unordered_map<std::string, Expression::Operation> string_to_operation;
};


#endif // TREE_H