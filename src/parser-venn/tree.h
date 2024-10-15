#ifndef TREE_H
#define TREE_H

#include <string>
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

#endif // TREE_H