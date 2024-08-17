// Code for the Diagram data structure being built.
#include "tree.h"

#include <vector>
#include "driver.h"

Circle Circle::NewCircle(const std::string& name, const Assignments& fields, Driver* driver) {
  Circle circle;
  circle.present = true;  // A new circle has never been deleted.
  circle.name.assign(name);
  for (NumericAssignment assign : fields.assignments) {
    if (assign.field_name.compare("x") == 0) {
      circle.x_center = assign.value;
    } else if (assign.field_name.compare("y") == 0) {
      circle.y_center = assign.value;
    } else if (assign.field_name.compare("radius") == 0) {
      circle.radius = assign.value;
    } else {
      driver->AddError("I don't know what a '" + assign.field_name + "' is.");
    }
  }
  return circle;
}

std::ostream& operator<<(std::ostream& os, const NumericAssignment &ex) {
  os << ex.field_name << " = " << ex.value;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Assignments &ex) {
  for (auto assign : ex.assignments) {
    os << assign;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Circle &ex) {
  os << ex.name << ", " << ex.x_center << ", " << ex.y_center << ", " << ex.radius;
  return os;
}

std::ostream& operator<<(std::ostream& os, const CircleList &ex) {
  for (auto assign : ex.circles) { 
    os << assign;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Diagram &ex) {
  for (auto assign : ex.circles) { 
    os << assign;
  }
  return os;
}
