#include "plugin.hpp"

#include <thread>

#include "buffered.hpp"

struct BrainwashThread {
  bool shutdown;
  float sample_rate = 0.0;
  // If set,  we merge a millisecond's worth of signal into one sample.
  bool cv_rate = false;
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

  void SetCV(bool cv) {
    cv_rate = cv;
  }

  void InitiateReplacement(int endpoint_position, std::shared_ptr<Buffer> buffer) {
    int endpoint = std::min(endpoint_position, max_sample_count - 1);
    this->endpoint_position = cv_rate ? trunc(endpoint * CV_SAMPLE_RATE / sample_rate) :
                                        endpoint;
    this->buffer = buffer;
  }

  void RecordSample(int position, float left, float right) {
    if (position >= max_sample_count || static_right == nullptr) {
      return;
    }
    if (cv_rate) {
      // Because there are many positions for each CV position, we need to only allow
      // a write to occur on one position per CV position. I want it to be the
      // "center" of the CV position.
      int cv_position = trunc((double) position * CV_SAMPLE_RATE / sample_rate);
      int writable_position = trunc((double) (cv_position + 0.5) * sample_rate / CV_SAMPLE_RATE);
      if (position == writable_position) {      
        static_left[cv_position] = left;
        static_right[cv_position] = right;
      }
    } else {
      static_left[position] = left;
      static_right[position] = right;
    }
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
        double seconds = cv_rate ? (endpoint_position + 1) / CV_SAMPLE_RATE
                                 : (endpoint_position + 1) / sample_rate;

        BufferTask* replace_task = BufferTask::ReplaceTask(
          new_left, new_right, nullptr,  // nullptr for status indicates we don't need FileIO status info.
          endpoint_position + 1, seconds, true);  
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
    CLOCK_COUNT_PARAM,
    ARM_BUTTON_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RECORD_GATE_INPUT,
    LEFT_INPUT,
    RIGHT_INPUT,
    CLOCK_TRIGGER_INPUT,
    ARM_TRIGGER_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUTPUTS_LEN
  };
  enum LightId {
    CONNECTED_LIGHT,
    RECORD_BUTTON_LIGHT,
    ARM_BUTTON_LIGHT,
    LIGHTS_LEN
  };

  enum TimedRecordingState {
    NOT_RECORDING,       // Not armed.
    WAITING_FOR_CLOCK,   // Armed and waiting for the next clock to start recording.
    RECORDING
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

  dsp::SchmittTrigger arm_trigger;
  TimedRecordingState timed_recording_state;
  // These can't be countdowns because COUNT knob might be changing, and I
  // should notice that.
  int clocks_so_far;  
  int fractional_samples_so_far;

  // For tracking the tempo of CLOCK.
  dsp::SchmittTrigger tempo_trigger;
  int64_t last_tempo_trigger_frame = -1;
  int64_t tempo_length_in_frames = -1;

  Brainwash() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(CLOCK_TRIGGER_INPUT, 
      "Clock for recording length; Arm recording and will record for COUNT clocks");
    configInput(ARM_TRIGGER_INPUT, "Trigger received here will arm recording for COUNT clocks");
    configSwitch(ARM_BUTTON_PARAM, 0, 1, 0, 
      "Press to arm recording to start on next CLOCK trigger",
      {"Disarmed", "Armed"});
    configParam(CLOCK_COUNT_PARAM, 0.001, 32, 4,
      "Number of repetitions of CLOCK to record for. May be fractional");
    configSwitch(RECORD_BUTTON_PARAM, 0, 1, 0, "Press to record/release to stop this recording",
                 {"Inactive", "Recording"});
    configInput(RECORD_GATE_INPUT, "Gate to start/stop recording");
    configInput(LEFT_INPUT, "Left");
    configInput(RIGHT_INPUT, "Right");

    recording_position = -1;
    was_recording = false;
    timed_recording_state = NOT_RECORDING;

    // I'm using the state of the arm button to indicate if we are armed, but we don't want
    // armed state to get saved.
    params[ARM_BUTTON_PARAM].setValue(0.0f);

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

    // Even if not connected, we might still be getting tempo triggers.
    // So we'll track them just in case.
    bool clock_event = false;
    if (inputs[CLOCK_TRIGGER_INPUT].isConnected()) {
      bool tempo_was_high = tempo_trigger.isHigh();
      tempo_trigger.process(rescale(inputs[CLOCK_TRIGGER_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
      if (!tempo_was_high && tempo_trigger.isHigh()) {
        if (last_tempo_trigger_frame >= 0) {
          tempo_length_in_frames = args.frame - last_tempo_trigger_frame;
        }
        // Set the length based on the new trigger.
        last_tempo_trigger_frame = args.frame;
        clock_event = true;
      }
    } else {
      // Reset the tempo tracking.
      last_tempo_trigger_frame = -1;
      tempo_length_in_frames = -1;
      tempo_trigger.reset();
    }

    bool connected = (buffer != nullptr) && buffer->IsValid();

    // If connected and buffer isn't empty.
    if (connected) {
      worker->SetCV(buffer->cv_rate);

      // Are we armed to start recording on the next clock trigger?
      bool arm_was_triggered = arm_trigger.isHigh();
      arm_trigger.process(
        rescale(inputs[ARM_TRIGGER_INPUT].getVoltage(),
          0.1f, 2.f, 0.f, 1.f));
      if (!arm_was_triggered && arm_trigger.isHigh()) {
        params[ARM_BUTTON_PARAM].setValue(1.0f); 
      }

      // Handle timed recording state machine.
      // May pass through multiple states in a single process() call, as this
      // makes for less repetition of the logic.
      if (timed_recording_state == NOT_RECORDING) {
        if (params[ARM_BUTTON_PARAM].getValue() > 0.1f) {
          // We are now armed.
          timed_recording_state = WAITING_FOR_CLOCK;
        }
      }
      if (timed_recording_state == WAITING_FOR_CLOCK) {
        if (clock_event) {
          // Start recording now.
          timed_recording_state = RECORDING;
          recording_position = -1;
          params[ARM_BUTTON_PARAM].setValue(0.0f);  // Not armed now that we've started.
          clocks_so_far = -1;  // Will be bumped to zero on this clock in next block.
          fractional_samples_so_far = 0;
        }
      }
      if (timed_recording_state == RECORDING) {
        if (clock_event) {
          ++clocks_so_far;
          fractional_samples_so_far = 0;
        } else {
          // Advance fractional samples.
          ++fractional_samples_so_far;
        }
        // See if we have finished recording yet.
        // This is complicated by the fact that:
        // * The tempo can change while recording.
        // * the COUNT knob can change while recording.
        // So we don't know if we are done until we are done.
        if (params[CLOCK_COUNT_PARAM].getValue() >
          clocks_so_far + ((double) fractional_samples_so_far / (tempo_length_in_frames > 0 ? tempo_length_in_frames : 1))) {
          // Still recording.
          ++recording_position;
          worker->RecordSample(recording_position,
              inputs[LEFT_INPUT].getVoltage(), inputs[RIGHT_INPUT].getVoltage());
          lights[RECORD_BUTTON_LIGHT].setBrightness(1.0f);
        } else {
          // Done recording.
          worker->InitiateReplacement(recording_position, buffer);
          timed_recording_state = NOT_RECORDING;
          lights[RECORD_BUTTON_LIGHT].setBrightness(0.0f);
        }
      } else {
        // The logic for recording via COUNT and CLOCK is different enough that
        // it makes sense to handle it separately from the gate/button recording.
        // TODO: What if both are used at the same time?

        // Are we recording or not?
        recordTrigger.process(rescale(
            inputs[RECORD_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
        
        // If we're not already recording due to timed recording, see if
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
        lights[ARM_BUTTON_LIGHT].setBrightness(params[ARM_BUTTON_PARAM].getValue() > 0.1f ? 1.0f : 0.0f);
      }
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

    // CLOCK and clock counting.
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.35, 63.0)), module, Brainwash::CLOCK_COUNT_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.05, 63.0)), module, Brainwash::CLOCK_TRIGGER_INPUT));

    // ARM button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(19.05, 75.0)),
                                             module, Brainwash::ARM_BUTTON_PARAM,
                                             Brainwash::ARM_BUTTON_LIGHT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.35, 75.0)), module,
                                             Brainwash::ARM_TRIGGER_INPUT));

    // Record button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(19.05, 90.0)),
                                             module, Brainwash::RECORD_BUTTON_PARAM,
                                             Brainwash::RECORD_BUTTON_LIGHT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.35, 90.0)), module,
                                             Brainwash::RECORD_GATE_INPUT));

    // Signal Inputs.                                         
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.638, 107.525)), module,
                                             Brainwash::LEFT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.638, 117.844)), module,
                                             Brainwash::RIGHT_INPUT));

    // Our light is not colored, since we don't have a position in Depict.
    addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(12.7, 3.2)),
                 module, Brainwash::CONNECTED_LIGHT));
  }

  void appendContextMenu(Menu* menu) override {
    // Be a little clearer how to make this module do anything.
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel(
      "Brainwash only works when touching a group of modules with a Memory or MemoryCV"));
    menu->addChild(createMenuLabel(
      "module to the left. See my User Manual for details and usage videos."));
  }
};


Model* modelBrainwash = createModel<Brainwash, BrainwashWidget>("Brainwash");
