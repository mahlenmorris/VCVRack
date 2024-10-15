// Code for the Diagram data structure being built.
#include "tree.h"

#include <algorithm>
#include <vector>
#include "driver.h"

void VennToLower(const std::string &mixed, std::string *lower) {
  lower->resize(mixed.size());
  std::transform(mixed.begin(), mixed.end(),
                 lower->begin(), ::tolower);
}

Circle Circle::NewCircle(const std::string& name, const Assignments& fields, VennDriver* driver) {
  Circle circle;
  circle.name.assign(name);
  circle.present = true;  // To allow me to read my development saves.
  for (Assignment assign : fields.assignments) {
    std::string lower_name;
    VennToLower(assign.field_name, &lower_name);
    if (lower_name.compare("x") == 0) {
      circle.x_center = assign.value;
    } else if (lower_name.compare("y") == 0) {
      circle.y_center = assign.value;
    } else if (lower_name.compare("radius") == 0) {
      circle.radius = assign.value;
    } else if (lower_name.compare("present") == 0) {
      circle.present = assign.value > 0;  // a boolean.
    } else if (lower_name.compare("name") == 0) {
      circle.name = assign.str_value;  // TODO: character conversion, like "\n" -> \n.
    } else {
      driver->AddError("I don't know what a '" + assign.field_name + "' is.");
    }
  }
  return circle;
}

std::ostream& operator<<(std::ostream& os, const Assignment &ex) {
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
