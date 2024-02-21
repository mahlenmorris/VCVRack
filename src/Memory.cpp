#include <thread>

#include "plugin.hpp"
#include "buffered.hpp"

// Class devoted to handling the lengthy (compared to single sample)
// process of filling or replacing the current Buffer.
struct WorkThread {
  Buffer* buffer;
  float sample_rate;
  bool shutdown;
  bool initiateFill;
  int seconds;
  bool initiateWipe;
  bool running;  // TRUE if still compiling, false if completed.

  explicit WorkThread(Buffer* the_buffer) : buffer{the_buffer} {
    running = false;
    initiateFill = false;
    initiateWipe = false;
    shutdown = false;
  }

  void Halt() {
    shutdown = true;
  }

  void SetRate(float rate) {
    sample_rate = rate;
  }

  void InitiateFill(int new_seconds) {
    seconds = new_seconds;
    initiateFill = true;
  }

  // If compilation succeeds, sets flag saying so and prepares vectors
  // of main_blocks and expression_blocks for module to use later.
  void Work() {
    while (!shutdown) {
      if (initiateFill) {
        running = true;
        initiateFill = false;
        int samples = std::round(seconds * sample_rate);
        float* new_left_array = new float[samples];
        float* new_right_array = new float[samples];

        // Note that this data is still random. Needs to be wiped!

        // And mark every sector dirty.
        for (int i = 0; i < WAVEFORM_SIZE; ++i) {
          buffer->dirty[i] = true;
        }

        buffer->left_array = new_left_array;
        buffer->right_array = new_right_array;

        buffer->length = samples;
        buffer->seconds = seconds;
        running = false;
      } else if (initiateWipe) {
        running = true;
        initiateWipe = false;
        for (int i = 0; i < buffer->length && !shutdown; ++i) {
          buffer->left_array[i] = 0.0f;
          buffer->right_array[i] = 0.0f;
        }

        // And mark every sector dirty.
        for (int i = 0; i < WAVEFORM_SIZE && ! shutdown; ++i) {
          buffer->dirty[i] = true;
        }
        running = false;
      }
      // It seems like I need a tiny sleep here to allow join() to work
      // on this thread?
      if (!shutdown) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
    }
  }
};

struct Memory : BufferedModule {
  enum ParamId {
    WIPE_BUTTON_PARAM,
    SECONDS_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    WIPE_TRIGGER_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUTPUTS_LEN
  };
  enum LightId {
    WIPE_BUTTON_LIGHT,
    LIGHTS_LEN
  };

  bool buffer_filled = false;
  bool fill_in_progress = false;
  bool buffer_wiped = false;
  bool wipe_in_progress = false;
  WorkThread* worker;
  std::thread* work_thread;

  // For wiping contents.
  dsp::SchmittTrigger wipe_trigger;
  // Keeps lights on buttons lit long enough to see.
  int wipe_light_countdown = 0;


  // To do some tasks every NN samples. Some UI-related tasks are not as
  // latency-sensitive as the audio thread, and we don't need to do often.
  // TODO: consider putting these tasks on a background thread...
  int assign_color_countdown = 0;

  Memory() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configButton(WIPE_BUTTON_PARAM, "Press to wipe contents to 0.0V");
    configInput(WIPE_TRIGGER_INPUT, "A trigger wipes contents to 0.0V");
    configParam(SECONDS_PARAM, 1, 1000, 15,
        "Length of Memory in seconds.");
    // This is really an integer.
    getParamQuantity(SECONDS_PARAM)->snapEnabled = true;
    worker = new WorkThread(getBuffer());
  }

  ~Memory() {
    if (worker != nullptr) {
      worker->Halt();
      worker->initiateFill = false;
      worker->initiateWipe = false;
      if (work_thread != nullptr) {
        work_thread->join();
        delete work_thread;
      }
      delete worker;
    }
  }

  // Save and retrieve menu choice(s).
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
  }

  // TODO: need something with no limit to the colors.
  static constexpr int COLOR_COUNT = 6;
	NVGcolor colors[COLOR_COUNT] = {
		SCHEME_RED,
		SCHEME_BLUE,
		SCHEME_YELLOW,
		SCHEME_PURPLE,
		SCHEME_GREEN,
		SCHEME_ORANGE
	};

  void process(const ProcessArgs& args) override {
    // This is just for the initial creation of the buffer.
    // We can't fill the buffer until process() is called, since we don't know
    // what the sample rate is.
    if (!buffer_filled || !buffer_wiped) {
      if (!buffer_filled) {
        if (!fill_in_progress) {
          // Confirm that we can read the sample rate before starting a fill.
          if (args.sampleRate > 1.0) {
            worker->SetRate(args.sampleRate);
            worker->InitiateFill(params[SECONDS_PARAM].getValue());
            // TODO: Should the worker thread be part of the Buffer itself?
            // Should it just be waiting for the sample rate to be filled, and then
            // start itself?
            work_thread = new std::thread(&WorkThread::Work, worker);
            fill_in_progress = true;
          }
        } else {
          if (worker->running) {
            // OK, just wait for it to finish.
          } else {
            // Filling done.
            fill_in_progress = false;
            buffer_filled = true;
          }
        }
      }
      if (buffer_filled) {
        if (!wipe_in_progress) {
          worker->initiateWipe = true;
          wipe_in_progress = true;
        } else {
          if (worker->running) {
            // OK, just wait for it to finish.
          } else {
            // Wiping done.
            wipe_in_progress = false;
            buffer_wiped = true;
          }
        }
      }
    }

    // Some lights are lit by triggers; these enable them to be
    // lit long enough to be seen by humans.
    if (wipe_light_countdown > 0) {
      wipe_light_countdown--;
    }

    // Periodically assign colors to each connected module's light.
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
          if (next_module->model == modelRemember) {
            // Make sure it's on the list of record heads.
            bool found = false;
            for (int i = 0; i < (int) getBuffer()->record_heads.size(); ++i) {
              if (getBuffer()->record_heads[i].module_id == next_module->getId()) {
                found = true;
                break;
              }
            }
            if (!found) {
              getBuffer()->record_heads.push_back(RecordHeadTrace(next_module->getId(),
                                                  pos_module->line_record.position));
            }
          }
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
      // TODO: add a process that eliminates very old RecordHead records.
      // We are possibly not connected to them any more.
      // Or in some other way, remove them.
    }

    // Have we been asked to wipe?
    bool wipe_was_low = !wipe_trigger.isHigh();
    wipe_trigger.process(rescale(
        inputs[WIPE_TRIGGER_INPUT].getVoltage(), 0.1f, 2.0f, 0.0f, 1.0f));
    if (wipe_was_low && wipe_trigger.isHigh()) {
      // Flash the wpie light for a tenth of second.
      // Compute how many samples to show the light.
      wipe_light_countdown = std::floor(args.sampleRate / 10.0f);
    }
    // Note that we don't bother to set wipe_light_countdown when the user
    // presses the button; we just light up the button while it's
    // being pressed.
    bool wipe = (params[WIPE_BUTTON_PARAM].getValue() > 0.1f) ||
      (wipe_was_low && wipe_trigger.isHigh());

    if (wipe) {
      // TODO: Decide (or menu options) to pause all recording and playing
      // during a wipe? Or at least fade the players?
      worker->initiateWipe = true;
    }

    // Set lights.
    lights[WIPE_BUTTON_LIGHT].setBrightness(
      wipe || wipe_light_countdown > 0 ? 1.0f : 0.0f);
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
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH,
                                           RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                                           RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // WIPE button and trigger.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 18.918)),
                                             module, Memory::WIPE_BUTTON_PARAM,
                                             Memory::WIPE_BUTTON_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 18.918)), module,
                                             Memory::WIPE_TRIGGER_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(14.441, 32.837)),
             module, Memory::SECONDS_PARAM));
  }

  void appendContextMenu(Menu* menu) override {
    // Memory* module = dynamic_cast<Memory*>(this->module);
  }
};

Model* modelMemory = createModel<Memory, MemoryWidget>("Memory");
