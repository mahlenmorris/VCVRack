#include <thread>

#include "plugin.hpp"
#include "buffered.hpp"
#include "smoother.h"

// We auto-scale the view of the waveform in Depict, but snap to these values
// to make it less confusing.
static const float WIDTHS[] = {
  0.01, 0.02, 0.05,
  0.1, 0.2, 0.5,
  1, 2, 5,
  10, 20, 50};
// The string versions of the WIDTHS above.
static const char* TEXTS[] = {
  "0.01V", "0.02V", "0.05V",
  "0.1V", "0.2V", "0.5V",
  "1V", "2V", "5V",
  "10V", "20V", "50V"
};

// Class devoted to handling the lengthy (compared to single sample)
// process of filling or replacing the current Buffer.
struct WorkThread {
  BufferHandle* handle;
  float sample_rate;
  bool shutdown;
  bool initiateFill;
  int seconds;
  bool initiateWipe;
  bool running;  // TRUE if still compiling, false if completed.

  explicit WorkThread(BufferHandle* the_handle) : handle{the_handle} {
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

  void RefreshWaveform(std::shared_ptr<Buffer> buffer) {
    int sector_size = buffer->length / WAVEFORM_SIZE;
    float peak_value = 0.0f;
    bool full_scan = buffer->full_scan;

    // For now, do this the most brute-force way; scan from bottom to top.
    for (int p = 0; !shutdown && p < WAVEFORM_SIZE; p++) {
      if (full_scan || buffer->dirty[p]) {
        float left_amplitude = 0.0, right_amplitude = 0.0;
        for (int i = p * sector_size;
              !shutdown && i < std::min((p + 1) * sector_size, buffer->length);
              i++) {
          left_amplitude = std::max(left_amplitude,
                                    std::fabs(buffer->left_array[i]));
          right_amplitude = std::max(right_amplitude,
                                      std::fabs(buffer->right_array[i]));
        }
        buffer->waveform.points[p][0] = left_amplitude;
        buffer->waveform.points[p][1] = right_amplitude;
        // WARN("%d: left = %f, right = %f", p, left_amplitude, right_amplitude);

        // Not stricly speaking correct (the sector may have been written
        // to during the scan). But correctness is not critical, and the new
        // values will be caught if any further writes to this sector occur,
        // which, since writes are sequential, is quite likely.
        buffer->dirty[p] = false;
      }
      peak_value = std::max(peak_value, buffer->waveform.points[p][0]);
      peak_value = std::max(peak_value, buffer->waveform.points[p][1]);
    }
    if (full_scan) {
      buffer->full_scan = false;
    }
    float window_size;
    for (int i = 0; i < 12; i++) {
      float f = WIDTHS[i];
      if (peak_value <= f || i == 11) {
        window_size = f;
        buffer->waveform.text_factor.assign(TEXTS[i]);
        break;
      }
    }
    buffer->waveform.normalize_factor = 10.0f / window_size;
  }

  // If compilation succeeds, sets flag saying so and prepares vectors
  // of main_blocks and expression_blocks for module to use later.
  void Work() {
    while (!shutdown) {
      std::shared_ptr<Buffer> buffer = handle->buffer;
      if (buffer) {
        if (buffer->smooths.additions.size() > 0) {
          Smooth* item;
          while (buffer->smooths.additions.pop(item)) {
            // Check that creation_time is long enough ago that we're
            // confident that the new section is written.
            if (item->creation_time < 0 || (system::getTime() - item->creation_time > 0.001)) {
        	    smooth(buffer->left_array, buffer->right_array, item->position, 25, buffer->length);
              delete item;
            } else {
              buffer->smooths.additions.push(item);
              break;
            }
	        }
        }

        if (buffer->freshen_waveform) {
          RefreshWaveform(buffer);
        }

        if (initiateFill) {
          initiateFill = false;
          running = true;
          // Invalidate existing contents, if any.
          buffer->length = 0;
          buffer->seconds = 0;

          int samples = std::round(seconds * sample_rate);
          float* new_left_array = new float[samples];
          float* new_right_array = new float[samples];

          // Note that this data is still random. Needs to be wiped!

          // And mark every sector dirty.
          buffer->full_scan = true;

          if (buffer->left_array != nullptr) {
            delete buffer->left_array;
          }
          buffer->left_array = new_left_array;
          if (buffer->right_array != nullptr) {
            delete buffer->right_array;
          }
          buffer->right_array = new_right_array;

          buffer->length = samples;
          buffer->seconds = seconds;
          running = false;
          // A fill always prompts a wipe, let's just do that here.
          initiateWipe = true;
        }
        if (initiateWipe) {
          running = true;
          initiateWipe = false;
          for (int i = 0; i < buffer->length && !shutdown; ++i) {
            buffer->left_array[i] = 0.0f;
            buffer->right_array[i] = 0.0f;
          }

          // And mark every sector dirty.
          buffer->full_scan = true;
          running = false;
        }
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
    RESET_BUTTON_PARAM,
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
    RESET_BUTTON_LIGHT,
    LIGHTS_LEN
  };

  // Initialization happens on startup, but also during a reset.
  bool buffer_initialized = false;
  bool init_in_progress = false;
  WorkThread* worker;
  std::thread* work_thread;

  // For wiping contents.
  dsp::SchmittTrigger wipe_trigger;
  // Keeps lights on buttons lit long enough to see.
  int wipe_light_countdown = 0;

  // Do some tasks every NN samples. Some UI-related tasks are not as
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
    configButton(RESET_BUTTON_PARAM, "Press to reset length and wipe contents to 0.0V");

    // Setting an initial Buffer object.
    std::shared_ptr<Buffer> temp = std::make_shared<Buffer>();
    (*getHandle()).buffer.swap(temp);

    worker = new WorkThread(getHandle());
    work_thread = new std::thread(&WorkThread::Work, worker);
  }

  ~Memory() {
    (*getHandle()).buffer.reset();
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
    if (!buffer_initialized) {
      if (!init_in_progress) {
        // Confirm that we can read the sample rate before starting a fill.
        // Sometimes during startup, sampleRate is still zero.
        if (args.sampleRate > 1.0) {
          worker->SetRate(args.sampleRate);
          worker->InitiateFill(params[SECONDS_PARAM].getValue());
          // TODO: Should the worker thread be part of the Buffer itself?
          // Should it just be waiting for the sample rate to be filled, and then
          // start itself?
          init_in_progress = true;
        }
      } else {
        if (!worker->running) {
          // Filling+wiping done.
          init_in_progress = false;
          buffer_initialized = true;
        }
      }
    } else {
      // We don't pay any attention to the buttons, etc, until the buffer is initialized.
      // Some lights are lit by triggers; these enable them to be
      // lit long enough to be seen by humans.
      if (wipe_light_countdown > 0) {
        wipe_light_countdown--;
      }

      // Periodically assign colors to each connected module's light.
      // TODO: Maybe put this into a background thread too?
      if (--assign_color_countdown <= 0) {
        // One hundredth of a second.
        assign_color_countdown = (int) (args.sampleRate / 100);

        std::shared_ptr<Buffer> buffer = getHandle()->buffer;
        if (buffer) {  // Checks for null.
          Module* next_module = getRightExpander().module;
          int color_index = -1;
          int distance = 0;
          bool found_depict = false;
          while (next_module) {
            if ((next_module->model == modelRecall) ||
                (next_module->model == modelEmbellish)) {
              // Assign a Color.
              distance++;
              color_index = (color_index + 1) % COLOR_COUNT;
              PositionedModule* pos_module = dynamic_cast<PositionedModule*>(next_module);
              pos_module->line_record.color = colors[color_index];
              pos_module->line_record.distance = distance;
              if (next_module->model == modelEmbellish) {
                // Make sure it's on the list of record heads.
                bool found = false;
                for (int i = 0; i < (int) buffer->record_heads.size(); ++i) {
                  if (buffer->record_heads[i].module_id == next_module->getId()) {
                    found = true;
                    break;
                  }
                }
                if (!found) {
                  buffer->record_heads.push_back(RecordHeadTrace(next_module->getId(),
                                                      pos_module->line_record.position));
                }
              }
            }
            // If we are still in our module list, move to the right.
            auto m = next_module->model;
            if (m == modelDepict) {
              // If there is a Depict, then make sure the waveform is being updated.
              found_depict = true;
            }
            if ((m == modelRecall) ||
                (m == modelEmbellish) ||
                (m == modelDepict)) {  // This will be a list soon...
              next_module = next_module->getRightExpander().module;
            } else {
              break;
            }
          }
          buffer->freshen_waveform = found_depict;
        }
        // TODO: add a process that eliminates very old RecordHead records.
        // We are possibly not connected to them any more.
        // Or in some other way, remove them.
      }

      // We may be being asked to wipe and/or reset. Both of these take more than one
      // process() call to complete.
      // If asked to do neither, we do neither, although we may be waiting for
      // a previous action to complete.
      // A wipe doesn't (currently) require us to watch the state progress, and it
      // doesn't prevent using the Buffer. So we don't particularly attend to it.
      // If only a reset is requested, then we start a reset. This can take while,
      // so we make sure not to allow another reset to interrupt it or another wipe
      // to occur while in progress.
      // If a reset AND a wipe are requested at the same process() call, only the reset
      // call is acted on.

      // Have we been asked to wipe?
      bool wipe_was_low = !wipe_trigger.isHigh();
      wipe_trigger.process(rescale(
          inputs[WIPE_TRIGGER_INPUT].getVoltage(), 0.1f, 2.0f, 0.0f, 1.0f));
      if (wipe_was_low && wipe_trigger.isHigh()) {
        // Flash the wpie light for a tenth of second.
        // Compute how many samples to show the light.
        wipe_light_countdown = std::floor(args.sampleRate / 10.0f);
      }
      bool reset = (params[RESET_BUTTON_PARAM].getValue() > 0.1f);
      // Note that we don't bother to set wipe_light_countdown when the user
      // presses the button; we just light up the button while it's
      // being pressed.
      bool wipe = (params[WIPE_BUTTON_PARAM].getValue() > 0.1f) ||
        (wipe_was_low && wipe_trigger.isHigh());

      if (reset) {
        if (buffer_initialized) {
          buffer_initialized = false;  // Should rerun the code at the top of process()?
          // TODO: Honestly, probably better to swap in a new Buffer instead...
        }
      } else {

        if (wipe) {
          // TODO: Decide (or menu options) to pause all recording and playing
          // during a wipe? Or at least fade the players?
          worker->initiateWipe = true;
        }
      }
      // Set lights.
      lights[WIPE_BUTTON_LIGHT].setBrightness(
        wipe || wipe_light_countdown > 0 ? 1.0f : 0.0f);
      lights[RESET_BUTTON_LIGHT].setBrightness(reset ? 1.0f : 0.0f);
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
    // RESET button.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 48.456)),
                                             module, Memory::RESET_BUTTON_PARAM,
                                             Memory::RESET_BUTTON_LIGHT));
  }

  void appendContextMenu(Menu* menu) override {
    // Memory* module = dynamic_cast<Memory*>(this->module);
  }
};

Model* modelMemory = createModel<Memory, MemoryWidget>("Memory");
