#ifndef TREE_HH
#define TREE_HH

#include <iostream>
#include <map>
#include <vector>
#include "environment.h"

class Expression;
// Base class for computing expressions.
class Expression {
public:
  enum Type {
    NUMBER,  // 3, 4.5, -283823
    BINOP,   // plus, times
    VARIABLE, // in1, out1, foo
    NOT,      // not bool
    ONEARGFUNC // func1(left_right[0])
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
    OR
  };
  Operation operation;
  float float_value;
  std::string name;
  double (*func1)(double);
  std::vector<Expression> left_right;

  static std::map<std::string, Operation> string_to_operation;
  static std::map<std::string, double (*)(double)> string_to_onearg_func;
  Expression() {}

  static Expression Not(const Expression &expr);
  static Expression Number(float the_value);
  static Expression OneArgFunc(const std::string &func_name,
                               const Expression &arg1);
  static Expression CreateBinOp(const Expression &lhs,
                                const std::string &op_string,
                                const Expression &rhs);
  static Expression Variable(const char *var_name);
  // The parser seems to need many variants of Variable.
  static Expression Variable(const Expression &expr);
  // The parser seems to need many variants of Variable.
  Expression(char * var_name);

  static Expression Variable(char * var_name);

  float Compute(Environment* env);
  friend std::ostream& operator<<(std::ostream& os, const Expression &ex);
  std::string to_string() const;
  static bool is_zero(float value);
private:
  float bool_to_float(bool value);
  float binop_compute(Environment* env);
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
  Expression expr1, expr2, expr3;
  std::vector<Statements> statements;

  // identifiers on both right hand and left hand side of := look the same.
  // So the lhs will get turned into a VariableExpression. We need to pull
  // the name out of it.
  static Line Assignment(Expression variable_expr, const Expression &expr) {
    Line line;
    line.type = ASSIGNMENT;
    line.str1 = variable_expr.name;
    line.expr1 = expr;
    return line;
  }

  // loop_type is the string identifying the loop type; e.g., "for" or "all".
  static Line Continue(const std::string &loop_type) {
    Line line;
    line.type = CONTINUE;
    line.str1 = loop_type;
    return line;
  }

  // loop_type is the string identifying the loop type; e.g., "for" or "all".
  static Line Exit(const std::string &loop_type) {
    Line line;
    line.type = EXIT;
    line.str1 = loop_type;
    return line;
  }

  static Line ForNext(const Line &assign, const Expression &limit,
                      const Expression &step, const Statements &state) {
    Line line;
    line.type = FORNEXT;
    line.str1 = assign.str1;
    line.expr1 = assign.expr1;
    line.expr2 = limit;
    line.expr3 = step;
    line.statements.push_back(state);
    return line;
  }

  static Line IfThen(const Expression &bool_expr,
                     const Statements &state1) {
    Line line;
    line.type = IFTHEN;
    line.expr1 = bool_expr;
    line.statements.push_back(state1);
    return line;
  }

  static Line IfThenElse(const Expression &bool_expr,
                         const Statements &state1,
                         const Statements &state2) {
    Line line;
    line.type = IFTHENELSE;
    line.expr1 = bool_expr;
    line.statements.push_back(state1);
    line.statements.push_back(state2);
    return line;
  }

  static Line Wait(const Expression &expr) {
    Line line;
    line.type = WAIT;
    line.expr1 = expr;
    return line;
  }

  friend std::ostream& operator<<(
    std::ostream& os, Line line) {
    os << "Line(" << line.str1 << ", " << line.expr1.to_string() << ")";
    return os;
  }
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
