#include <thread>

#include "plugin.hpp"
#include "buffered.hpp"

// Class devoted to handling the lengthy (compared to single sample)
// process of filling or replacing the current Buffer.
struct FillThread {
  Buffer* buffer;
  float sample_rate;
  bool running;  // TRUE if still compiling, false if completed.

  explicit FillThread(Buffer* the_buffer) : buffer{the_buffer} {
    running = false;
  }

  void SetRate(float rate) {
    sample_rate = rate;
  }

  // If compilation succeeds, sets flag saying so and prepares vectors
  // of main_blocks and expression_blocks for module to use later.
  void Fill() {
    running = true;
    int seconds = 15;
    int samples = std::round(seconds * sample_rate);
    float* new_left_array = new float[samples];
    float* new_right_array = new float[samples];

    // Wipe array before giving it to buffer.
    for (int i = 0; i < samples; ++i) {
      new_left_array[i] = 0.0f;
      new_right_array[i] = 0.0f;
    }

    buffer->left_array = new_left_array;
    buffer->right_array = new_right_array;
    buffer->length = samples;
    buffer->seconds = seconds;
    running = false;
  }
};


struct Memory : BufferedModule {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    INPUTS_LEN
  };
  enum OutputId {
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };


  bool buffer_valid = false;
  bool fill_in_progress = false;
  FillThread* filler;
  std::thread* fill_thread;

  // Just to debug record_heads.
  int sample_count = 0;

  Memory() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    filler = new FillThread(getBuffer());
  }

  // Save and retrieve menu choice(s).
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
  }

  void process(const ProcessArgs& args) override {
    // We can't fill the buffer until process() is called, since we don't know
    // what the sample rate is.
    if (!buffer_valid) {
      if (fill_in_progress) {
        if (filler->running) {
          // OK, just wait for it to finish.
        } else {
          // Filling done.
          fill_in_progress = false;
          buffer_valid = true;
        }
      } else {
        // Confirm that we can read the sample rate before starting a fill.
        if (args.sampleRate > 1.0) {
          fill_in_progress = true;
          filler->SetRate(args.sampleRate);
          fill_thread = new std::thread(&FillThread::Fill, filler);
        }
      }
    }
    /*
    sample_count++;
    // Every five seconds, print out the structure.
    if (sample_count > args.sampleRate * 5) {
      sample_count = 0;
      Buffer* buffer = getBuffer();
      for (RecordHeadTrace trace : buffer->record_heads) {
        WARN("module = %lld, position = %d, age = %d", trace.module_id, trace.position, trace.age);
      }
    }
    */
  }

};

struct MemoryWidget : ModuleWidget {
  MemoryWidget(Memory* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Memory.svg")));
    // TODO: Make a dark version.
    //    setPanel(createPanel(asset::plugin(pluginInstance, "res/Memory.svg"),
    //                         asset::plugin(pluginInstance, "res/Memory-dark.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }

  void appendContextMenu(Menu* menu) override {
    // Memory* module = dynamic_cast<Memory*>(this->module);
  }
};

Model* modelMemory = createModel<Memory, MemoryWidget>("Memory");
