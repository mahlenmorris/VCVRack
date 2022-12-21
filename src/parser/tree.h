#ifndef TREE_HH
#define TREE_HH

#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "environment.h"

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
    ZEROARGFUNC, // operation
    ONEARGFUNC, // operation (subexpressions[0])
    ONEPORTFUNC, // operation (port)
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
    CONNECTED,
    FLOOR,
    SAMPLE_RATE,
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
  // But other variables are Input or Output ports in the UI. We can avoid
  // always updating them by pointing to their location in the Environment.
  PortPointer port;
  Environment* env = nullptr;

  std::string name;
  std::vector<Expression> subexpressions;

  static std::unordered_map<std::string, float> note_to_volt_octave_4;
  Expression() {}

  // Compute the result of this Expression.
  float Compute();
  // Determine if this Expression can Compute() different results depending on
  // INn or any other volatile source (e.g., a random() function.)
  bool Volatile();

  // Bison seems to require this; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const Expression &ex);
  std::string to_string() const;
  static bool is_zero(float value);
 private:
  float bool_to_float(bool value);
  float binop_compute();
  float zero_arg_compute();
  float one_arg_compute(float arg1);
  float two_arg_compute(float arg1, float arg2);
};

class ExpressionFactory {
 public:
  Environment* env;  // Only public for my debugging.
  void SetEnvironment(Environment* the_env) { env = the_env; }
  Expression Not(const Expression &expr);
  Expression Note(const std::string &note_name);
  Expression Number(float the_value);
  Expression ZeroArgFunc(const std::string &func_name);
  Expression OneArgFunc(const std::string &func_name,
                               const Expression &arg1);
  Expression OnePortFunc(const std::string &func_name, const std::string &port1,
                         Driver* driver);
  Expression TwoArgFunc(const std::string &func_name,
                               const Expression &arg1, const Expression &arg2);
  Expression CreateBinOp(const Expression &lhs,
                                const std::string &op_string,
                                const Expression &rhs);
  Expression Variable(const char *var_name, Driver* driver);
  // The parser seems to need many variants of Variable.
  Expression Variable(const std::string &expr, Driver* driver);
  Expression Variable(char * var_name, Driver* driver);
 private:
  static std::unordered_map<std::string, Expression::Operation> string_to_operation;
};

struct Statements;

struct Line {
  enum Type {
    ASSIGNMENT,  // str1 = expr1
    CONTINUE,    // continue str1
    ELSEIF,      // elseif expr1 then state1
    EXIT,        // exit str1
    FORNEXT,     // for str1 = expr1 to expr2 state1 next
    IFTHEN,      // if expr1 then state1 [elseifs - state2] end if
    IFTHENELSE,  // if expr1 then state1 [elseifs - state3] else state2 end if
    WAIT         // wait expr1
  };
  Type type;
  std::string str1;

  // When assigning to a variable/port.
  float* variable_ptr;
  PortPointer assign_port;

  Expression expr1, expr2, expr3;
  std::vector<Statements> statements;

  // identifiers on both right hand and left hand side of := look the same.
  // So the lhs will get turned into a VariableExpression. We need to pull
  // the name out of it.
  static Line Assignment(const std::string &variable_name,
                         const Expression &expr, Driver* driver);

  // loop_type is the string identifying the loop type; e.g., "for" or "all".
  static Line Continue(const std::string &loop_type);

  static Line ElseIf(const Expression &bool_expr,
                     const Statements &state1);

  // loop_type is the string identifying the loop type; e.g., "for" or "all".
  static Line Exit(const std::string &loop_type);

  static Line ForNext(const Line &assign, const Expression &limit,
                      const Expression &step, const Statements &state,
                      Driver* driver);

  static Line IfThen(const Expression &bool_expr,
                     const Statements &then_state,
                     const Statements &elseifs);

  static Line IfThenElse(const Expression &bool_expr,
                         const Statements &then_state,
                         const Statements &else_state,
                         const Statements &elseifs);

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
