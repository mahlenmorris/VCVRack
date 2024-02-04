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

    // And mark every sector dirty.
    for (int i = 0; i < WAVEFORM_SIZE; ++i) {
      buffer->dirty[i] = true;
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

  // To do some tasks every NN samples. Some UI-related tasks are not as
  // latency-sensitive as the audio thread, and we don't need to do often.
  // TODO: consider putting these tasks on a background thread...
  int assign_color_countdown = 0;

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

  static constexpr int COLOR_COUNT = 6;

  // TODO: need something with no limit to the colors.
	NVGcolor colors[COLOR_COUNT] = {
		SCHEME_RED,
		SCHEME_BLUE,
		SCHEME_YELLOW,
		SCHEME_PURPLE,
		SCHEME_GREEN,
		SCHEME_ORANGE
	};

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
    if (--assign_color_countdown <= 0) {
      // One hundredth of a second.
      assign_color_countdown = (int) (args.sampleRate / 100);

      Module* next_module = getRightExpander().module;
      int color_index = -1;
      int distance = 0;
      while (next_module) {
        if ((next_module->model == modelRecall) ||
            (next_module->model == modelRemember)) {
          // Assign a Color.
          distance++;
          color_index = (color_index + 1) % COLOR_COUNT;
          PositionedModule* pos_module = dynamic_cast<PositionedModule*>(next_module);
          pos_module->line_record.color = colors[color_index];
          pos_module->line_record.distance = distance;
        }
        // If we are still in our module list, move to the right.
        auto m = next_module->model;
        if ((m == modelRecall) ||
            (m == modelRemember) ||
            (m == modelDisplay)) {  // This will be a list soon...
          next_module = next_module->getRightExpander().module;
        } else {
          break;
        }
      }
    }
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
