#ifndef TREE_H
#define TREE_H

#include <iostream>
#include "environment.h"

// Base class for computing expressions.
class ExpressionBase {
public:
  ExpressionBase() {}
  virtual float Compute(Environment env) {
    return -1.0;
  }
  friend std::ostream& operator<<(
    std::ostream& os, const ExpressionBase &dummy) {
    os << "ExpressionBase";
    return os;
  }
};

class NumberExpression : public ExpressionBase {
  float value = -2.3456;
public:
  NumberExpression(float the_value) {
    value = the_value;
  }
  float Compute(Environment env) override {
    return value;
  }
  friend std::ostream& operator<<(
    std::ostream& os, const NumberExpression &expr) {
    os << "NumberExpression(" << expr.value << ")";
    return os;
  }
};

// A class that specifies a binary operation that should be performed later.
class BinOpExpression : public ExpressionBase {
  // What type is a Token?
  int operation;  // Found in parser.hh.
  // Should these be pointers? Or const references?
  ExpressionBase left = ExpressionBase();
  ExpressionBase right = ExpressionBase();
public:
  BinOpExpression(int binop, ExpressionBase &lhs, ExpressionBase &rhs) {
    operation = binop;
    left = lhs;
    right = rhs;
  }
  float Compute(Environment env) override {
    float lhs = left.Compute(env);
    float rhs = right.Compute(env);
    switch (operation) {
      TOK_PLUS: return lhs + rhs;
      TOK_MINUS: return lhs - rhs;
      TOK_STAR: return lhs * rhs;
      TOK_SLASH: return lhs / rhs;  // TODO: protect against division by zero.
      default: return -1.23456;
    }
  }
  friend std::ostream& operator<<(
    std::ostream& os, const BinOpExpression &expr) {
    os << "BinOpExpression(" << expr.operation << ")";
    return os;
  }
};

class VariableExpression : public ExpressionBase {
public:
  VariableExpression(std::string name) {
    // Intentionally copying the name.
    variable_name = name;
  }
  VariableExpression(char * name) {
    // Intentionally copying the name.
    variable_name = std::string(name);
  }
  VariableExpression() {
    // Intentionally copying the name.
    variable_name = "No variable name provided! Why?";
  }

  VariableExpression(const VariableExpression &var_expr) {
    // Intentionally copying the name.
    variable_name = var_expr.GetName();
  }
  float Compute(Environment env) override {
    // TODO: This operation actually depends on the name. Make this correct.
    return env.variables.at(variable_name);
  }
  friend std::ostream& operator<<(
    std::ostream& os, const VariableExpression &expr) {
    os << "NumberExpression(" << expr.variable_name << ")";
    return os;
  }
  const std::string GetName() const {
    return variable_name;
  }

private:
  std::string variable_name;
};

struct Line {
  enum Type {
    ASSIGNMENT  // str1 = expr1
  };
  Type type;
  std::string str1;
  ExpressionBase expr1;

  // identifiers on both right hand and left hand side of := look the same.
  // So the lhs will get turned into a VariableExpression. We need to pull
  // the name out of it.
  static Line Assignment(VariableExpression name, ExpressionBase expr) {
    Line line;
    line.type = ASSIGNMENT;
    line.str1 = name.GetName();
    line.expr1 = expr;
    return line;
  }
};

#endif // TREE_H
