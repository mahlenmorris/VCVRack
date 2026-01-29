#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

struct Point {
  double x, y, value;
};

// Simple bilinear interpolation
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
    {0.5, 0.8, 0.5},
    {0.3, 0.6, 0.7}
  };
  
  generateVisualization(points, 512, 512, 0.0, 1.0, 0.0, 1.0);
  
  return 0;
}