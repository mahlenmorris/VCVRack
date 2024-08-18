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

  std::string to_string() {
    std::string result("[");
    result.append(name);
    result.append("]\n");

    if (present) {
      result.append(AnAssignment("x", x_center));
      result.append(AnAssignment("y", y_center));
      result.append(AnAssignment("radius", radius));
    }  // Leaving these fields as zero will result in it looking like a deleted circle.
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

#endif // TREE_H