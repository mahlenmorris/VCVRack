#pragma once

#include <string>
#include <vector>

#include "plugin.hpp"

#include "NoLockQueue.h"  // For Smooth events.

// To fade volume when near any other recording head.
// TODO: should be related to sample rate? Set by user?
const double FADE_DISTANCE = 50.0;

// Sample rate we use in MemoryCV. While possible we may let user
// pick rate at some point, not yet convinced it's all that valuable
// to allow that. 
const float CV_SAMPLE_RATE = 1000.0f;

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
  int position;  // Position in the buffer.
  int age;  // Approximate number of samples since this module_id has recorded.

  RecordHeadTrace(long long id, int pos) : module_id{id}, position{pos}, age{0} {}
};

// Number of lines in the waveforms.
// TODO: Test and see if this number is too large.
static const int WAVEFORM_SIZE = 1024;

// Number of samples the algorithm will look on either side to find the
// endpoints for smoothing.
// TODO: this should, maybe, be based on time and sample rate? Maybe set by user?
static const int MAX_SMOOTHING_DISTANCE = 25;

struct Smooth {
  int position;  // The position where the new write started.
  // If -1, then no need to wait. Otherwise, wait a small amount of time to allow the
  // record head to complete writes to the portion we will smooth.
  double creation_time;

  Smooth(int pos, bool immediate) : position{pos} {
    if (!immediate) {
      creation_time = system::getTime();
    } else {
      creation_time = -1.0;
    }
  }
};

struct SmoothQueue {
  // Consumer is obligated to delete the Smooth objects it has completed.
  SpScLockFreeQueue<Smooth*, 50> additions;
};

// Structs used in the process of replacing the contents of a Buffer.
// Adding Brainwash module required having this be more visible.

// My modified AudioFile.h uses this.
struct StringQueue {
  // 30 lines of log messages from File I/O operations.
  SpScLockFreeQueue<std::string, 30> lines;
};

// Data structures for background threads to communicate with the module.
// This is less something to DO and more of a channel of communications
// between two threads.
enum FileIOCompleted {
  IN_PROGRESS,
  LOAD_COMPLETED,
  SAVE_COMPLETED
};

struct FileOperationReporting {
  FileIOCompleted completed;
  StringQueue log_messages;

  FileOperationReporting() : completed{IN_PROGRESS} {}
};

// Class for scheduling large changes to the Buffer.
struct BufferTask {
  enum Type {
    SAVE_FILE,     // str1 is the full path and name. For this queue so as to prevent 
                   // REPLACE_AUDIO happening while saving.
    REPLACE_AUDIO  // new_left_array and new_right_array.
  };
  Type type;
  std::string str1;
  float* new_left_array;
  float* new_right_array;
  int sample_count;
  double seconds;
  bool smooth_endpoints;
  FileOperationReporting* status;  // Owned by module.
  
  BufferTask(const Type the_type) : type{the_type},
                                    new_left_array{nullptr}, new_right_array{nullptr},
                                    status{nullptr} {}

  ~BufferTask();

  static BufferTask* SaveFileTask(FileOperationReporting* status, const std::string& file_path);

  static BufferTask* ReplaceTask(float* new_left, float* new_right, FileOperationReporting* status,
                                 int sample_count, double seconds, bool smooth_ends);
};

struct BufferTaskQueue {
  // Consumer is obligated to delete the BufferTask objects it has completed.
  SpScLockFreeQueue<BufferTask*, 5> queue;
};

/****
 * Data for creating the drawing of the waveform in Depict.
 */
struct PointBuffer {
  // We just measure the amplitudes, not the min and max of the waves.
  // At the scale we show, a single channel is nearly certain to be symmetric.
  // I.e., we are closer to SoundCloud than Scope.
  float points[WAVEFORM_SIZE][2];
  double normalize_factor;
  std::string text_factor;
};

struct Buffer {
  // Consider making this a 2 x length array.
  float* left_array;   // make this std::shared_ptr.
  float* right_array;   // make this std::shared_ptr.

  // These two are the same for Memory modules, but different for MemoryCV modules.
  int length = 0;      // Length in audio rate samples (what the modules work in.)
  int true_length = 0; // Actually array length.

  double seconds;

  // For marking blocks of the waveform that Depict shows as needing to be updated.
  bool dirty[WAVEFORM_SIZE];
  // Shortcut to mark all blocks dirty.
  bool full_scan;

  // Since Recording heads of any kind are a source of discontinuities (and thus
  // clicks), we track where they are and make sure to duck the volume of a play
  // head encountering one.
  // This list is maintained by Memory, and consulted with by NearHead().
  std::vector<RecordHeadTrace> record_heads;

  // Embellish and other "recording" modules add to this, Memory's Work queue acts on them
  // and removes them. They correspond to locations in the buffer that will need to be smoothed.
  SmoothQueue smooths;

  // Brainwash module can send calls to replace the entire contents of the buffer via this queue
  // that Memory reads.
  BufferTaskQueue replacements;

  // The waveform that Depict's will display.
  PointBuffer waveform;
  // Let's not bother to update depict_buffer if there aren't any Depict's
  // looking at it.
  bool freshen_waveform;

  // Memory and MemoryCV differ only slightly, and most of the difference is how Buffer behaves.
  // This flag tells the code which to behave like.
  bool cv_rate;

  Buffer() : left_array{nullptr}, right_array{nullptr}, length{0},
             seconds{0.0}, full_scan{false}, freshen_waveform{true}, cv_rate{false} {}

  ~Buffer() {
    if (left_array) {
      delete left_array;
    }
    if (right_array) {
      delete right_array;
    }
  }

  bool IsValid();

  // How many samples away from another head this position is. If less than some value,
  // caller may decide to fade playback.
  int NearHead(int position);
  // Returns distance if near a recording head, except for the recording head
  // with 'module_id', or INT_MAX if not considered "near".
  // Typically called for the benefit of recording heads.
  int NearHeadButNotThisModule(int position, long long module_id);

  void SetDirty(int position);

  // Caller is responsible for only calling this when IsValid() is true.
  // 'position' is realtime sample units.
  void Get(FloatPair *pair, double position);
  // 'position' is 1:1 with memory, not with time.
  // Used by SAVE_FILE.
  void GetDirect(FloatPair *pair, double position);
  // Caller is responsible for only calling this when IsValid() is true.
  void Set(int position, float left, float right, long long module_id);
};

struct BufferHandle {
  std::shared_ptr<Buffer> buffer;
};

// Module that has a buffer that others can access. Probably only Memory
// will be this.
// 
struct BufferedModule : Module {
  BufferHandle handle;

  BufferedModule() {
    // Setting an initial Buffer object, just so there always is one.
    std::shared_ptr<Buffer> temp = std::make_shared<Buffer>();
    handle.buffer.swap(temp);
  }

  BufferHandle* getHandle() {
    return &handle;
  }
};

// Module that has an associated position. Play and record heads have these.

enum ModuleType {
  RUMINATE,
  EMBELLISH,
  FIXATION
  // More later.
};

struct LineRecord {
	double position;  // Set by module. Read by Depict.
	NVGcolor color;  // Set by Memory. Read by Depict and the module.
	ModuleType type;  // Set by module class. Read by Depict.
  // Number of modules away from Memory this is. One-indexed.
  int distance;  // Set by Memory, Read by Depict.

  LineRecord() {
    // A color the system should never set. Indicates something is wrong.
    color = nvgRGBA(255, 0, 255, 255);
  }

  LineRecord(double the_position, NVGcolor the_color,
             ModuleType the_type, int the_distance) :
    position{the_position}, color{the_color}, type(the_type), distance{the_distance}
  {}
};

struct PositionedModule : Module {
  LineRecord line_record;
};

bool ModelHasColor(Model* model);
bool IsMemoryEnsembleModel(Model* model);
bool IsNonMemoryEnsembleModel(Model* model);
std::shared_ptr<Buffer> findClosestMemory(Module* leftModule);

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
        snprintf(text_buffer, 10, "%02u.%02u", (value / 100), value % 100);
      } else {
        // TODO: this hasn't really been tested.
        int value = trunc(sec_position);
        snprintf(text_buffer, 10, "%u:%02u", value / 60, value % 60);
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
struct ConnectedLight : LargeLight<GreenLight> {
  PositionedModule* pos_module;

  void step() override {
    if (pos_module) {
      baseColors[0] = pos_module->line_record.color;
    }
    LargeLight::step();
  }
};
