#ifndef TREE_H
#define TREE_H

#include <string>
#include <vector>

class VennDriver;  // Circular includes if we try to include driver.h here.

// Intermediate structures made while compiling.
struct NumericAssignment {
  std::string field_name;
  float value;

  static NumericAssignment NewAssignment(const std::string& attr, float value) {
    NumericAssignment assign;
    assign.field_name.assign(attr);
    assign.value = value;
    return assign;
  }

  // Bison seems to require this if I use const references; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const NumericAssignment &ex);
};

struct Assignments {
  std::vector<NumericAssignment> assignments;

  static Assignments NewAssignments(const NumericAssignment& first) {
    Assignments assign;
    assign.assignments.push_back(first);
    return assign;
  }

  Assignments Add(const NumericAssignment& next) {
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

  // Must be defined in tree.cc, because it actually uses Driver.
  static Circle NewCircle(const std::string& name, const Assignments& fields, VennDriver* driver);

  // Bison seems to require this if I use const references; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const Circle &ex);
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

#endif // TREE_H