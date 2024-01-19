#pragma once

#include <string>

#include "plugin.hpp"

struct Buffer {
  float* left_array;   // make this std::shared_ptr.
  float* right_array;   // make this std::shared_ptr.
  double seconds;
  int length = 0;
};

struct BufferedModule : Module {
  Buffer buffer;

  Buffer* getBuffer() {
    return &buffer;
  }
};

Buffer* findClosestMemory(Module* leftModule);

struct TimestampField : TextField {
	TimestampField() {
    box.size = mm2px(Vec(14.0, 6.0));
    multiline = false;
  }

  virtual double getPosition() = 0;
  virtual double getSeconds() = 0;

  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      double sec_position = getPosition();
      double seconds = getSeconds();
      char text_buffer[10];
      if (seconds < 60) {
        // display "seconds.hundreths"
        int value = trunc(sec_position * 100);
        sprintf(text_buffer, "%02d.%02d", value / 100, value % 100);
      }
      std::string result(text_buffer);
      text = result;
    }
    Widget::drawLayer(args, layer);
  }
};
