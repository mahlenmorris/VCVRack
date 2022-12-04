#ifndef TREE_HH
#define TREE_HH

#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Expression;
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
    TWOARGFUNC // func2(subexpressions[0], subexpressions[1])
  };
  Type type;
  // Which BinOp is this?
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
    SIGN,
    SIN,
    MOD,
    MAX,
    MIN,
    POW
  };
  Operation operation;
  float float_value;
  float* variable_ptr;
  std::string name;
  std::vector<Expression> subexpressions;

  static std::unordered_map<std::string, Operation> string_to_operation;
  static std::unordered_map<std::string, float> note_to_volt_octave_4;
  static std::unordered_set<std::string> volatile_inputs;
  Expression() {}

  static Expression Not(const Expression &expr);
  static Expression Note(const std::string &note_name);
  static Expression Number(float the_value);
  static Expression OneArgFunc(const std::string &func_name,
                               const Expression &arg1);
  static Expression TwoArgFunc(const std::string &func_name,
                               const Expression &arg1, const Expression &arg2);
  static Expression CreateBinOp(const Expression &lhs,
                                const std::string &op_string,
                                const Expression &rhs);
  static Expression Variable(const char *var_name, Driver* driver);
  // The parser seems to need many variants of Variable.
  static Expression Variable(const std::string &expr, Driver* driver);
  // The parser seems to need many variants of Variable.
  Expression(char * var_name, Driver* driver);

  static Expression Variable(char * var_name, Driver* driver);

  // Compute the result of this Expression.
  float Compute();
  // Determine if this Expression can Compute() different results depending on
  // INn or any other volatile source (e.g., a random() function.)
  // The names of the dependencies must be added to this set.
  bool Volatile(std::unordered_set<std::string>* volatile_deps);

  // Bison seems to require this; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const Expression &ex);
  std::string to_string() const;
  static bool is_zero(float value);
private:
  float bool_to_float(bool value);
  float binop_compute();
  float one_arg_compute(float arg1);
  float two_arg_compute(float arg1, float arg2);
};

struct Statements;

struct Line {
  enum Type {
    ASSIGNMENT,  // str1 = expr1
    CONTINUE,    // continue str1
    EXIT,        // exit str1
    FORNEXT,     // for str1 = expr1 to expr2 state1 next
    IFTHEN,      // if expr1 then state1 endif
    IFTHENELSE,  // if expr1 then state1 else state2 endif
    WAIT         // wait expr1
  };
  Type type;
  std::string str1;
  float* variable_ptr;
  Expression expr1, expr2, expr3;
  std::vector<Statements> statements;

  // identifiers on both right hand and left hand side of := look the same.
  // So the lhs will get turned into a VariableExpression. We need to pull
  // the name out of it.
  static Line Assignment(const std::string &variable_name,
                         const Expression &expr, Driver* driver);

  // loop_type is the string identifying the loop type; e.g., "for" or "all".
  static Line Continue(const std::string &loop_type);

  // loop_type is the string identifying the loop type; e.g., "for" or "all".
  static Line Exit(const std::string &loop_type);

  static Line ForNext(const Line &assign, const Expression &limit,
                      const Expression &step, const Statements &state);

  static Line IfThen(const Expression &bool_expr,
                     const Statements &state1);

  static Line IfThenElse(const Expression &bool_expr,
                         const Statements &state1,
                         const Statements &state2);

  static Line Wait(const Expression &expr);

  friend std::ostream& operator<<(std::ostream& os, Line line);
};

struct Statements {
  std::vector<Line> lines;

  Statements add(Line new_line) {
    lines.push_back(new_line);
    return *this;
  }
  int size() {
    return lines.size();
  }
  friend std::ostream& operator<<(std::ostream& os, Statements statements) {
    os << "Statements(" << std::to_string(statements.lines.size()) << " statements )";
    return os;
  }
};

#endif // TREE_HH
