#pragma once

#include <string>
#include <vector>

#include "plugin.hpp"

// Just to make transmiting data easier, but might not need?
struct FloatPair {
  float left;
  float right;

  FloatPair() : left{0.0f}, right{0.0f} {}
};

// A record of a moment that a record head called Set(). Used by Play heads
// to see if they need to duck their volume to prevent a click when passing
// over the path of a Record head.
struct RecordHeadTrace {
  long long module_id;  // Unique (I think) ID for each module instance.
  int position;  // Position in the byffer.
  int age;  // Approximate number of samples since this module_id has recorded.

  RecordHeadTrace(long long id, int pos) : module_id{id}, position{pos}, age{0} {}
};

// Number of lines in the waveforms.
// TODO: Test and see if this number is too large.
static const int WAVEFORM_SIZE = 1024;

struct Buffer {
  // Consider making this a 2 x length array.
  float* left_array;   // make this std::shared_ptr.
  float* right_array;   // make this std::shared_ptr.
  int length = 0;
  double seconds;

  // For marking blocks of the waveform that Display shows as needing to be updated.
  bool dirty[WAVEFORM_SIZE];

  std::vector<RecordHeadTrace> record_heads;

  Buffer() : left_array{nullptr}, right_array{nullptr}, length{0},
             seconds{0.0} {}

  bool IsValid() {
    return (length > 0) && (seconds > 0.0) && (left_array != nullptr) &&
           (right_array != nullptr);
  }

  bool NearHead(int position) {
    // TODO: not sure if correct when near the ends of the buffer.
    for (int i = 0; i < (int) record_heads.size(); ++i) {
      if (abs(record_heads[i].position - position) < 60) {
        // WARN("head = %d, pos = %d", record_heads[i].position, position);
        return true;
      }
      if (abs(record_heads[i].position + length - position) < 60) {
        // WARN("head = %d, pos = %d", record_heads[i].position, position);
        return true;
      }
      if (abs(record_heads[i].position - (position + length)) < 60) {
        // WARN("head = %d, pos = %d", record_heads[i].position, position);
        return true;
      }
    }
    return false;
  }

  void SetDirty(int position) {
    dirty[(int) floor(position / ((double) length / WAVEFORM_SIZE))] = true;
  }

  // Returns true if near a recording head, except for module_id.
  // Typically called for the benefit of play heads that are part of recording
  // heads.
  bool NearHeadButNotThisModule(int position, long long module_id) {
    // TODO: not sure if correct when near the ends of the buffer.
    for (int i = 0; i < (int) record_heads.size(); ++i) {
      if (record_heads[i].module_id == module_id) {
        continue;
      }
      if (abs(record_heads[i].position - position) < 60) {
        // WARN("head = %d, pos = %d", record_heads[i].position, position);
        return true;
      }
      if (abs(record_heads[i].position + length - position) < 60) {
        // WARN("head = %d, pos = %d", record_heads[i].position, position);
        return true;
      }
      if (abs(record_heads[i].position - (position + length)) < 60) {
        // WARN("head = %d, pos = %d", record_heads[i].position, position);
        return true;
      }
    }
    return false;
  }

  // Caller is responsible for only calling this when IsValid() is true.
  void Get(FloatPair *pair, double position) {
    assert(position < length);
    int playback_start = trunc(position);
    int playback_end = trunc(playback_start + 1);
    if (playback_end >= length) {
      playback_end -= length;  // Should be zero.
    }
    float start_fraction = position - playback_start;
    pair->left = left_array[playback_start] * (1.0 - start_fraction) +
                 left_array[playback_end] * (start_fraction);
    pair->right = right_array[playback_start] * (1.0 - start_fraction) +
                  right_array[playback_end] * (start_fraction);
  }

  // Caller is responsible for only calling this when IsValid() is true.
  void Set(int position, float left, float right, long long module_id) {
    assert(position < length);
    left_array[position] = left;
    right_array[position] = right;
    SetDirty(position);

    // Update (or create) a trace for this Set() call.
    // TODO: Memory should create/update this list during the module scan in
    // process().
    bool found = false;
    for (int i = 0; i < (int) record_heads.size(); ++i) {
      if (record_heads[i].module_id == module_id) {
        found = true;
        record_heads[i].position = position;
        record_heads[i].age = 0;
      }
    }
    if (!found) {
      // TODO: this is dangerous!
      record_heads.push_back(RecordHeadTrace(module_id, position));
    }
  }
};

// Module that has a buffer that others can access. Probably only Memory
// will be this.
struct BufferedModule : Module {
  Buffer buffer;

  Buffer* getBuffer() {
    return &buffer;
  }
};

// Module that has an associated position. Play and record heads have these.

enum ModuleType {
	RECALL,
	REMEMBER
	// More later.
};

struct LineRecord {
	double position;  // Set by module. Read by Display.
	NVGcolor color;  // Set by Memory. Read by Display and the module.
	ModuleType type;  // Set by module class. Read by Display.
  // Number of modules away from Memory this is. One-indexed.
  int distance;  // Set by Memory, Read by Display.

  LineRecord() {
    // A color the system should never set. Indicates something is wrong.
    color = nvgRGBA(255, 0, 255, 255);
  }
};

struct PositionedModule : Module {
  LineRecord line_record;
};

Buffer* findClosestMemory(Module* leftModule);

struct TimestampField : OpaqueWidget {
	TimestampField() {
    box.size = mm2px(Vec(10.0, 5.0));
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
      } else {
        // TODO: this hasn't really been tested.
        int value = trunc(sec_position);
        sprintf(text_buffer, "%d:%02d", value / 60, value % 60);
      }
      std::string result(text_buffer);

      // Draw the timestamp result in the box.
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();

      // Save previous state.
      nvgSave(args.vg);

      // Draw background color.
      nvgBeginPath(args.vg);
      nvgRect(args.vg, 0.0, 0.0, bounding_box.x, bounding_box.y);
      nvgFillColor(args.vg, SCHEME_DARK_GRAY);
      nvgFill(args.vg);

      nvgBeginPath(args.vg);
      nvgFillColor(args.vg, SCHEME_WHITE);
      nvgFontSize(args.vg, 11);
      // Do I need this? nvgFontFaceId(args.vg, font->handle);
      nvgTextLetterSpacing(args.vg, -1);

      // Place on the line just off the left edge.
      nvgText(args.vg, 3, 11, result.c_str(), NULL);

      // Restore previous state.
      nvgRestore(args.vg);
    }
    Widget::drawLayer(args, layer);
  }
};


// Common UI elements.
struct ConnectedLight : MediumLight<GreenLight> {
	PositionedModule* module;

  void step() override {
		if (module) {
			baseColors[0] = module->line_record.color;
		}
		MediumLight::step();
	}
};
