#ifndef TREE_HH
#define TREE_HH

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include "environment.h"

// Base class for computing expressions.
class Expression;
class Expression {
public:
  enum Type {
    NUMBER,  // 3, 4.5, -283823
    BINOP,   // plus, times
    VARIABLE
  };
  Type type;
  enum Operation {
    PLUS,
    MINUS,
    TIMES,
    DIVIDE
  };
  // Which BinOp is this?
  Operation operation;  // Found in parser.hh.
  float float_value;
  std::string name;
  std::vector<Expression> left_right;

  Expression() {}

  static Expression Number(float the_value) {
    Expression ex;
    ex.type = NUMBER;
    ex.float_value = the_value;
    return ex;
  }
  static Expression Plus(const Expression &lhs, const Expression &rhs) {
    return CreateBinOp(lhs, rhs, PLUS);
  }
  static Expression Minus(const Expression &lhs, const Expression &rhs) {
    return CreateBinOp(lhs, rhs, MINUS);
  }
  static Expression Times(const Expression &lhs, const Expression &rhs) {
    return CreateBinOp(lhs, rhs, TIMES);
  }
  static Expression Divide(const Expression &lhs, const Expression &rhs) {
    return CreateBinOp(lhs, rhs, DIVIDE);
  }
  static Expression Variable(const char *var_name) {
    // Intentionally copying the name.
    Expression ex;
    ex.type = VARIABLE;
    ex.name = std::string(var_name);
    return ex;
  }
  // The parser seems to need many variants of Variable.
  static Expression Variable(const Expression &expr) {
    // Intentionally copying the name.
    return Variable(expr.name.c_str());
  }
  // The parser seems to need many variants of Variable.
  Expression(char * var_name) {
    // Intentionally copying the name.
    name = std::string(var_name);
  }

  static Expression Variable(char * var_name) {
    // Intentionally copying the name.
    return Variable(std::string(var_name).c_str());
  }

  float Compute(Environment* env) {
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
      default: return 1.2345;
    }
  }
  friend std::ostream& operator<<(std::ostream& os, const Expression &ex) {
    os << ex.to_string();
    return os;
  }
  std::string to_string() const {
    switch (type) {
      case NUMBER: return "NumberExpression(" + std::to_string(float_value) + ")";
      case BINOP: return "BinOpExpression(" + std::to_string(operation) + ", " +
          left_right[0].to_string() + ", " + left_right[1].to_string() + ")";
      case VARIABLE: return "VariableExpression(" + name + ")";
      default: return "Expression(ERR: Unknown type!)";
    }
  }
private:
  static Expression CreateBinOp(const Expression &lhs, const Expression &rhs,
                         Operation op) {
    Expression ex;
    ex.type = BINOP;
    ex.left_right.push_back(lhs);
    ex.left_right.push_back(rhs);
    ex.operation = op;
    return ex;
  }
  float binop_compute(Environment* env){
    float lhs = left_right[0].Compute(env);
    float rhs = left_right[1].Compute(env);
    switch (operation) {
      case PLUS: return lhs + rhs;
      case MINUS: return lhs - rhs;
      case TIMES: return lhs * rhs;
      case DIVIDE: return lhs / rhs;  // TODO: protect against division by zero.
      default: return -2.3456;
    }
  }
};

// Should this be folded into Expression? How?
class BoolExpression {
public:
  // The six boolean comparison operators can be composed from Direction
  // and equality.
  enum Direction {
    GT,
    LT,
    NONE
  };
  Direction direction;
  bool equality;
  std::vector<Expression> left_right;

  BoolExpression() {}

  BoolExpression(const Expression &lhs, const std::string &op,
                 const Expression &rhs) {
    left_right.push_back(lhs);
    left_right.push_back(rhs);
    // Deduce direction and equality.
    if (op == "<" || op == "<=") {
      direction = LT;
      equality = op == "<=";
    } else if (op == ">" || op == ">=") {
      direction = GT;
      equality = op == ">=";
    } else {
      direction = NONE;
      equality = op == "==";
    }
  }

  bool Compute(Environment* env) {
    float lhs = left_right[0].Compute(env);
    float rhs = left_right[1].Compute(env);
    bool are_equal = std::fabs(lhs - rhs) <=
                     std::numeric_limits<float>::epsilon();
    if (equality && are_equal) {
      return true;
    }
    switch (direction) {
      case LT: return lhs < rhs;
      case GT: return lhs > rhs;
      case NONE: {
        if (equality) {
          return false;
        } else {
          return !are_equal;
        }
      }
      default: return false;
    }
  }
  friend std::ostream& operator<<(std::ostream& os, const BoolExpression &ex) {
    os << ex.to_string();
    return os;
  }
  std::string to_string() const {
  return "BoolExpression(" + std::to_string(direction) + ", " +
      std::to_string(equality) + ", " +
      left_right[0].to_string() + ", " + left_right[1].to_string() + ")";
  }
};

struct Statements;

struct Line {
  enum Type {
    ASSIGNMENT,  // str1 = expr1
    CONTINUE,    // continue str1
    EXIT,        // exit str1
    FORNEXT,     // for str1 = expr1 to expr2 state1 next
    IFTHEN,      // if bool1 then state1 endif
    IFTHENELSE,  // if bool1 then state1 else state2 endif
    WAIT         // wait expr1
  };
  Type type;
  std::string str1;
  Expression expr1, expr2, expr3;
  BoolExpression bool1;
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

  static Line IfThen(const BoolExpression &bool_expr,
                     const Statements &state1) {
    Line line;
    line.type = IFTHEN;
    line.bool1 = bool_expr;
    line.statements.push_back(state1);
    return line;
  }

  static Line IfThenElse(const BoolExpression &bool_expr,
                         const Statements &state1,
                         const Statements &state2) {
    Line line;
    line.type = IFTHENELSE;
    line.bool1 = bool_expr;
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
