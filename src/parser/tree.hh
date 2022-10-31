#ifndef TREE_HH
#define TREE_HH

#include <iostream>
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

struct Line {
  enum Type {
    ASSIGNMENT,  // str1 = expr1
    WAIT         // wait expr1
  };
  Type type;
  std::string str1;
  Expression expr1;

  // identifiers on both right hand and left hand side of := look the same.
  // So the lhs will get turned into a VariableExpression. We need to pull
  // the name out of it.
  static Line Assignment(Expression variable_expr, const Expression &expr) {
    Line line;
    line.type = ASSIGNMENT;
    line.str1 = variable_expr.name;
    line.expr1 = expr;
    //  std::cout << "Creating Assignment(" << expr->to_string() << ")!\n";
    return line;
  }
  static Line Wait(const Expression &expr) {
    Line line;
    line.type = WAIT;
    line.expr1 = expr;
    //  std::cout << "Creating Wait(" << expr->to_string() << ")!\n";
    return line;
  }
  friend std::ostream& operator<<(
    std::ostream& os, Line line) {
    os << "Line(" << line.str1 << ", " << line.expr1.to_string() << ")";
    return os;
  }
};

#endif // TREE_HH
