#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/interpolation_functions.h>
#include <CGAL/Interpolation_traits_2.h>
#include <CGAL/natural_neighbor_coordinates_2.h>
#include <vector>
#include <map>

// Setup CGAL types
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_2<K>           Delaunay;
typedef CGAL::Interpolation_traits_2<K>             Traits;
typedef K::FT                                       Coord_type;
typedef K::Point_2                                  Point_2;

double interpolate(const std::map<Point_2, Coord_type>& function_values, const Delaunay& T,
   double x, double y) {
  if (function_values.empty()) return 0.0;
    // 2. The point we want to interpolate at
    Point_2 query(x, y);

    // 3. Natural Neighbor Interpolation
    // We pass the triangulation, the query point, and a "Value" accessor
    std::vector<std::pair<Point_2, Coord_type>> neighbors;
    auto result_pair = CGAL::natural_neighbor_coordinates_2(
        T, query, std::back_inserter(neighbors));
    
    if (neighbors.empty()) return 0.0;
    
    Coord_type norm = result_pair.second;

    Coord_type result = CGAL::linear_interpolation(
        neighbors.begin(), neighbors.end(), norm,
        CGAL::Data_access<std::map<Point_2, Coord_type>>(function_values));
    return result;
}



struct Point {
  double x, y, value;
};

// Simple bilinear interpolation.
// Take into account every point, which means that (for example) low points
// affect the entire field. 
double interpolate_bilinear(const std::vector<Point>& points, double x, double y) {
  if (points.empty()) return 0.0;
  
  double sum = 0.0, weightSum = 0.0;
  
  for (const auto& p : points) {
    double dist = std::sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
    if (dist < 0.01) return 1- p.value; // The size of the POINT, effectively.
    
    double weight = 1.0 / (dist * dist);  // More "* dist"'s make the weight smaller more quickly.

    sum += p.value * weight;
    weightSum += weight;
  }
  
  return weightSum > 0 ? sum / weightSum : 0.0;
}

double findValueOfNearestPoint(const std::vector<Point>& points, double x, double y) {
  if (points.empty()) return 0.0;
  
  double value = 0.0;

  double prevDist = 1000000.0;
  for (const auto& p : points) {
    double dist = std::sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));

    if (dist < prevDist) {
      prevDist = dist;
      value = p.value;
    }
  }
  return value;
}


// Only consider nearest point.
// Basically a Voronoi diagram approach. No actual interpolation.
// Nice if you want to quickly change the values and don't want to be all
// that accurate in your movements.
double voronoi(const std::vector<Point>& points, double x, double y) {
  if (points.empty()) return 0.0;
  
  double value = 0.0;

  double prevDist = 1000000.0;
  for (const auto& p : points) {
    double dist = std::sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
    if (dist < 0.01) return 1- p.value; // The size of the POINT, effectively.

    if (dist < prevDist) {
      prevDist = dist;
      value = p.value;
    }
  }
  return value;
}

// Only consider nearest two points.
// This actually generalizes the other two functions (just vary the number
// of points considered).
// Pretty terrible at 2, honestly. Or 3, or 4.
// Well, nicer when we mirror the points around the bounding box, 
// and use dist^3. Maybe call this "spray paint interpolation"?
double interpolate_nearest_n(const std::vector<Point>& points, double x, double y, int n) {
  if (points.empty()) return 0.0;
  std::vector<Point> sorted_points = points;
  std::sort(sorted_points.begin(), sorted_points.end(), [&](const Point& a, const Point& b) {
    double distA = std::sqrt((x - a.x) * (x - a.x) + (y - a.y) * (y - a.y));
    double distB = std::sqrt((x - b.x) * (x - b.x) + (y - b.y) * (y - b.y));
    return distA < distB;
  });
  
  double sum = 0.0, weightSum = 0.0;

  for (int index = 0; index < n && index < (int) sorted_points.size(); ++index) {
    const auto& p = sorted_points[index];
    double dist = std::sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
    // This just shows the point nicely.
    if (dist < 0.01) return 1- p.value; // The size of the POINT, effectively.

    //double weight = 1.0 / (dist * dist);
    double weight = 1.0 / (dist * dist * dist);
    sum += p.value * weight;
    weightSum += weight;
  }
  return weightSum > 0 ? sum / weightSum : 0.0;
}

void valueToRGB(double value, int& r, int& g, int& b) {
  value = std::max(0.0, std::min(1.0, value));
  r = static_cast<int>(value * 255);
  g = static_cast<int>(value * 255);
  b = static_cast<int>(value * 255);
}

void generateVisualization(const std::vector<Point>& points, int width, int height, 
              double minX, double maxX, double minY, double maxY) {
  std::ofstream file("interpolation.bmp", std::ios::binary);
  
  // BMP header
  int fileSize = 54 + width * height * 3;
  unsigned char header[54] = {
    0x42, 0x4D, (unsigned char)(fileSize & 0xFF), (unsigned char)((fileSize >> 8) & 0xFF),
    (unsigned char)((fileSize >> 16) & 0xFF), (unsigned char)((fileSize >> 24) & 0xFF),
    0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
    (unsigned char)(width & 0xFF), (unsigned char)((width >> 8) & 0xFF), 0, 0,
    (unsigned char)(height & 0xFF), (unsigned char)((height >> 8) & 0xFF), 0, 0,
    1, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  file.write(reinterpret_cast<char*>(header), 54);
  file << "P6\n" << width << " " << height << "\n255\n";
  
  // When using a CGAL function, need to convert to their format. 
  std::map<Point_2, Coord_type> function_values;
  Delaunay T;

  // Set up points for CGAL.
  for(const auto& p : points) {
    Point_2 pt(p.x, p.y);
    T.insert(pt);
    function_values[pt] = p.value;
  }

  for (int py = 0; py < height; ++py) {
    for (int px = 0; px < width; ++px) {
      double x = minX + (px / static_cast<double>(width - 1)) * (maxX - minX);
      double y = minY + (py / static_cast<double>(height - 1)) * (maxY - minY);
      
      //double value = interpolate_bilinear(points, x, y);
      double value = interpolate_nearest_n(points, x, y, 4);
      //double value = voronoi(points, x, y);
      //double value = interpolate(function_values, T, x, y);
      int r, g, b;
      valueToRGB(value, r, g, b);
      file << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
    }
  }
  file.close();
  std::cout << "Visualization saved to interpolation.bmp\n";
}

int main() {
  double minX = 0.0, maxX = 1.0, minY = 0.0, maxY = 1.0;
    std::vector<Point> points = {
    {0.2, 0.3, 0.1},
    {0.7, 0.25, 0.9},
    {0.8, 0.8, 0.45},
    {0.3, 0.7, 0.7}
  };
  
  std::vector<Point> extended_points(points);
  // A hack to get around limitations of interpolation algorithms.
  // We mirror each point around the four sides of the bounding box.
  for(const auto& p : points) {
    extended_points.push_back({2 * minX - p.x, p.y, p.value});
    extended_points.push_back({2 * maxX - p.x, p.y, p.value});
    extended_points.push_back({p.x, 2 * minY - p.y, p.value});
    extended_points.push_back({p.x, 2 * maxY - p.y, p.value});
  }

  generateVisualization(extended_points, 512, 512, minX, maxX, minY, maxY);
  
  return 0;
}
