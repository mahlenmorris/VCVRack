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
  virtual std::string to_string() const {
    return "ExpressionBase(empty)";
  }
};

class NumberExpression : public ExpressionBase {
public:
  NumberExpression(float the_value) {
    value = the_value;
    std::cout << "Creating NumberExpression!\n";
  }
  float Compute(Environment env) override {
    return value;
  }
  friend std::ostream& operator<<(
    std::ostream& os, const NumberExpression &expr) {
    os << expr.to_string();
    return os;
  }
  std::string to_string() const override {
    return "NumberExpression(" + std::to_string(value) + ")";
  }
  float value = -2.3456;
};

// A class that specifies a binary operation that should be performed later.
class BinOpExpression : public ExpressionBase {
  enum Operation {
    PLUS,
    MINUS,
    TIMES,
    DIVIDE
  };

  // What type is a Token?
  Operation operation;  // Found in parser.hh.
  // Should these be pointers? Or const references?
  ExpressionBase* left;
  ExpressionBase* right;
  BinOpExpression(ExpressionBase* lhs, ExpressionBase* rhs) {
    left = lhs;
    right = rhs;
    std::cout << "Creating BinOpExpression!\n";
    std::cout << "left = " << left->to_string() << "\n";
  }
public:
  static ExpressionBase* Plus(ExpressionBase* lhs, ExpressionBase* rhs) {
    BinOpExpression* binop = new BinOpExpression(lhs, rhs);
    binop->operation = PLUS;
    return binop;
  }
  static ExpressionBase* Minus(ExpressionBase* lhs, ExpressionBase* rhs) {
    BinOpExpression* binop = new BinOpExpression(lhs, rhs);
    binop->operation = MINUS;
    return binop;
  }
  static ExpressionBase* Times(ExpressionBase* lhs, ExpressionBase* rhs) {
    BinOpExpression* binop = new BinOpExpression(lhs, rhs);
    binop->operation = TIMES;
    return binop;
  }
  static ExpressionBase* Divide(ExpressionBase* lhs, ExpressionBase* rhs) {
    BinOpExpression* binop = new BinOpExpression(lhs, rhs);
    binop->operation = DIVIDE;
    return binop;
  }

  float Compute(Environment env) override {
    float lhs = left->Compute(env);
    float rhs = right->Compute(env);
    switch (operation) {
      case PLUS: return lhs + rhs;
      case MINUS: return lhs - rhs;
      case TIMES: return lhs * rhs;
      case DIVIDE: return lhs / rhs;  // TODO: protect against division by zero.
      default: return -1.23456;
    }
  }
  friend std::ostream& operator<<(
    std::ostream& os, const BinOpExpression &expr) {
    os << expr.to_string();
    return os;
  }
  std::string to_string() const override {
    return "BinOpExpression(" + std::to_string(operation) + ", " +
           left->to_string() + ", " + right->to_string() + ")";
  }
};

class VariableExpression : public ExpressionBase {
public:
  VariableExpression(std::string name) {
    // Intentionally copying the name.
    variable_name = name;
    std::cout << "Creating VariableExpression(string)!\n";
  }
  VariableExpression(char * name) {
    // Intentionally copying the name.
    variable_name = std::string(name);
    std::cout << "Creating VariableExpression(char *)!\n";
  }
  VariableExpression() {
    // Intentionally copying the name.
    variable_name = "No variable name provided! Why?";
    std::cout << "Creating VariableExpression(void)!\n";
  }

  VariableExpression(const VariableExpression &var_expr) {
    // Intentionally copying the name.
    variable_name = var_expr.GetName();
    std::cout << "Creating VariableExpression(VE)!\n";
  }
  float Compute(Environment env) override {
    // TODO: This operation actually depends on the name. Make this correct.
    return env.variables.at(variable_name);
  }
  friend std::ostream& operator<<(
    std::ostream& os, const VariableExpression &expr) {
    os << expr.to_string();
    return os;
  }
  const std::string GetName() const {
    return variable_name;
  }
  std::string to_string() const override {
    return "VariableExpression(" + variable_name + ")";
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
  ExpressionBase* expr1;

  // identifiers on both right hand and left hand side of := look the same.
  // So the lhs will get turned into a VariableExpression. We need to pull
  // the name out of it.
  static Line Assignment(VariableExpression name, ExpressionBase* expr) {
    Line line;
    line.type = ASSIGNMENT;
    line.str1 = name.GetName();
    line.expr1 = expr;
    std::cout << "Creating Assignment(" << expr->to_string() << ")!\n";
    return line;

  }
  friend std::ostream& operator<<(
    std::ostream& os, Line line) {
    os << "Line(" << line.str1 << ", " << line.expr1->to_string() << ")";
    return os;
  }
};

#endif // TREE_H
