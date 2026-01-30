#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

struct Point {
  double x, y, value;
};

/*
// Simple bilinear interpolation.
// Take into account every point, which means that (for example) low points
// affect the entire field. 
double interpolate(const std::vector<Point>& points, double x, double y) {
  if (points.empty()) return 0.0;
  
  double sum = 0.0, weightSum = 0.0;
  
  for (const auto& p : points) {
    double dist = std::sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
    if (dist < 1e-6) return p.value;
    
    double weight = 1.0 / (dist * dist);
    sum += p.value * weight;
    weightSum += weight;
  }
  
  return weightSum > 0 ? sum / weightSum : 0.0;
}
*/

/*
// Only consider nearest point.
// Basically a Voronoi diagram approach. No actual interpolation.
double interpolate(const std::vector<Point>& points, double x, double y) {
  if (points.empty()) return 0.0;
  
  double value = 0.0;

  double prevDist = 1000000.0;
  for (const auto& p : points) {
    double dist = std::sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
    if (dist < 1e-6) return p.value;

    if (dist < prevDist) {
      prevDist = dist;
      value = p.value;
    }
  }
  return value;
}
*/

// Only consider nearest two points.
// This actually generalizes the other two functions (just vary the number
// of points considered).
// Pretty terrible at 2, honestly. Or 3, or 4.
double interpolate(const std::vector<Point>& points, double x, double y) {
  if (points.empty()) return 0.0;
  std::vector<Point> sorted_points = points;
  std::sort(sorted_points.begin(), sorted_points.end(), [&](const Point& a, const Point& b) {
    double distA = std::sqrt((x - a.x) * (x - a.x) + (y - a.y) * (y - a.y));
    double distB = std::sqrt((x - b.x) * (x - b.x) + (y - b.y) * (y - b.y));
    return distA < distB;
  });
  
  double sum = 0.0, weightSum = 0.0;

  for (int index = 0; index < 1; ++index) {
    const auto& p = sorted_points[index];
    double dist = std::sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
//    if (dist < 0.01) return p.value; // The size of the POINT, effectively.
    if (dist < 0.01) return 1- p.value; // The size of the POINT, effectively.

    double weight = 1.0 / (dist * dist);
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
  
  for (int py = 0; py < height; ++py) {
    for (int px = 0; px < width; ++px) {
      double x = minX + (px / static_cast<double>(width - 1)) * (maxX - minX);
      double y = minY + (py / static_cast<double>(height - 1)) * (maxY - minY);
      
      double value = interpolate(points, x, y);
      int r, g, b;
      valueToRGB(value, r, g, b);
      
      file << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
    }
  }
  file.close();
  std::cout << "Visualization saved to interpolation.bmp\n";
}

int main() {
  std::vector<Point> points = {
    {0.2, 0.3, 0.1},
    {0.7, 0.4, 0.9},
    {0.5, 0.8, 0.45},
    {0.3, 0.6, 0.7}
  };
  
  generateVisualization(points, 512, 512, 0.0, 1.0, 0.0, 1.0);
  
  return 0;
}