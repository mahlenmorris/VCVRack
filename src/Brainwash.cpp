#include "plugin.hpp"

#include <thread>

#include "buffered.hpp"

struct BrainwashThread {
  bool shutdown;
  float sample_rate = 0.0;
  int max_sample_count;

  // These are the fixed-size buffers that get recorded to initially.
  float* static_left;
  float* static_right;

  // Setting this to a number above zero initiates a transfer.
  // The 0-offset of the last sample to copy. 
  int endpoint_position;
  std::shared_ptr<Buffer> buffer;

  BrainwashThread() : shutdown{false}, sample_rate{0.0f}, max_sample_count{0},
                      static_left{nullptr}, static_right{nullptr},
                      endpoint_position{-1} {}

  ~BrainwashThread() {
    Halt();
    if (static_left != nullptr) {
      delete static_left;
    }
    if (static_right != nullptr) {
      delete static_right;
    }
  }

  void Halt() {
    shutdown = true;
  }

  void SetRate(float rate) {
    sample_rate = rate;
  }

  void InitiateReplacement(int endpoint_position, std::shared_ptr<Buffer> buffer) {
    this->endpoint_position = std::min(endpoint_position, max_sample_count - 1);
    this->buffer = buffer;
  }

  void RecordSample(int position, float left, float right) {
    if (position >= max_sample_count || static_right == nullptr) {
      return;
    }
    static_left[position] = left;
    static_right[position] = right;
  }

  void Work() {
    while (!shutdown) {
      if (static_left == nullptr) { // Not yet assigned.
        if (sample_rate > 1.0) {  // sample rate has been set
          // We can make the fixed buffers now.
          // Defaulting to 1 minute maximum.
          int sample_count = std::round(60 * sample_rate);
          max_sample_count = sample_count;
          static_left = new float[sample_count];
          static_right = new float[sample_count];
        }
      } else if (endpoint_position > 0) {
        // Create new buffer to send to Memory.
        size_t length = endpoint_position + 1;
        float* new_left = new float[length];
        float* new_right = new float[length];
        // These allocs can take a small bit of time, check that we aren't being shut down.
        if (shutdown) {
          delete[] new_left;
          delete[] new_right;
          return;
        }
        // Copies from zero to last position.
        memcpy(new_left, static_left, sizeof(float) * length);
        memcpy(new_right, static_right, sizeof(float) * length);

        // Now add to queue for Memory to consume.
        BufferTask* replace_task = BufferTask::ReplaceTask(
          new_left, new_right, nullptr,  // nullptr for status indicates we don't need FileIO status info.
          endpoint_position + 1, (endpoint_position + 1) / sample_rate, true);  
        if (!(buffer->replacements.queue.push(replace_task))) {
          delete replace_task->new_left_array;
          delete replace_task->new_right_array;
          delete replace_task;  // Queue is full, shed load.
        }
        endpoint_position = -1;
        //WARN("BufferTask sent!");
        //WARN("sender says replace_task->sample_count = %d", replace_task->sample_count);
      }

      // It seems like I need a tiny sleep here to allow join() to work
      // on this thread.
      if (!shutdown) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }
};

struct Brainwash : Module {
  enum ParamId {
    RECORD_BUTTON_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RECORD_GATE_INPUT,
    LEFT_INPUT,
    RIGHT_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUTPUTS_LEN
  };
  enum LightId {
    CONNECTED_LIGHT,
    RECORD_BUTTON_LIGHT,
    LIGHTS_LEN
  };

  // We look for the nearest Memory every NN samples. This saves CPU time.
  int find_memory_countdown = 0;
  std::shared_ptr<Buffer> buffer;

  // Where we are in the movement, before taking POSITION parameters into account.
  // Always 0.0 <= playback_position < length when Looping.
  // Always 0.0 <= playback_position < 2 * length when Bouncing.
  int recording_position;

  // For background thread.
  BrainwashThread* worker;
  std::thread* thread;

  // For detecting the Gate that controls recording.
  dsp::SchmittTrigger recordTrigger;

  // Set if we were recording in the previous process() call.
  bool was_recording;

  Brainwash() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch(RECORD_BUTTON_PARAM, 0, 1, 0, "Press to record/release to stop this recording",
                 {"Inactive", "Recording"});
    configInput(RECORD_GATE_INPUT, "Gate to start/stop recording");
    configInput(LEFT_INPUT, "Left");
    configInput(RIGHT_INPUT, "Right");

    recording_position = -1;

    was_recording = false;
    worker = new BrainwashThread();
    thread = new std::thread(&BrainwashThread::Work, worker);
  }

  ~Brainwash() {
    if (worker != nullptr) {
      worker->Halt();
      if (thread != nullptr) {
        thread->join();
        delete thread;
      }
      delete worker;
    }
  }

  void process(const ProcessArgs& args) override {
    // Only call this only every N samples, since the vast majority of
    // the time this won't change.
    // The number of modules it needs to go through does seem to increase the
    // CPU consummed by the module.
    if (args.sampleRate > 1.0) {
      worker->SetRate(args.sampleRate);
      if (--find_memory_countdown <= 0) {
        // One sixtieth of a second.
        find_memory_countdown = (int) (args.sampleRate / 60);

        buffer = findClosestMemory(getLeftExpander().module);
      }
    }

    bool connected = (buffer != nullptr) && buffer->IsValid();

    // If connected and buffer isn't empty.
    if (connected) {
      // Are we in motion or not?
      recordTrigger.process(rescale(
          inputs[RECORD_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
      // 'recording' just reflects the state of the button and the gate input.
      bool recording = (params[RECORD_BUTTON_PARAM].getValue() > 0.1f) ||
                     recordTrigger.isHigh();

      if (recording && !was_recording) {
        // Starting from the top.
        recording_position = -1;
      } else if (!recording && was_recording) {
        // Recording done, send it!
        //WARN("Initiated brainwash at position %d", recording_position);
        worker->InitiateReplacement(recording_position, buffer);
      }
      was_recording = recording;
      if (recording) {
        ++recording_position;
        worker->RecordSample(recording_position,
            inputs[LEFT_INPUT].getVoltage(), inputs[RIGHT_INPUT].getVoltage());
      }
      lights[RECORD_BUTTON_LIGHT].setBrightness(recording ? 1.0f : 0.0f);
    }
    lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
  }
};

struct BrainwashWidget : ModuleWidget {
  VCVLightSlider<WhiteLight>* adjust_slider;

  BrainwashWidget(Brainwash* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Brainwash.svg"),
                         asset::plugin(pluginInstance, "res/Brainwash-dark.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Record button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(19.05, 87.408)),
                                             module, Brainwash::RECORD_BUTTON_PARAM,
                                             Brainwash::RECORD_BUTTON_LIGHT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.35, 87.408)), module,
                                             Brainwash::RECORD_GATE_INPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.638, 107.525)), module,
                                             Brainwash::LEFT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.638, 117.844)), module,
                                             Brainwash::RIGHT_INPUT));

    // Our light is not colored, since we don't have a position in Depict.
    addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(12.7, 3.2)),
                 module, Brainwash::CONNECTED_LIGHT));
  }
};


Model* modelBrainwash = createModel<Brainwash, BrainwashWidget>("Brainwash");
