#include <thread>

#include "buffered.hpp"
#include "plugin.hpp"

struct BrainwashThread {
  static constexpr double PREROLL_SECONDS = 0.010;
  static constexpr int PREROLL_CV_FRAMES = 10;

  bool shutdown;
  float sample_rate = 0.0;
  // If set,  we merge a millisecond's worth of signal into one sample.
  bool cv_rate = false;
  int max_sample_count;
  int max_left_channels_seen = 1;
  int max_right_channels_seen = 1;

  // These are the fixed-size buffers that get recorded to initially.
  float* static_left;
  float* static_right;

  // Pre-roll ring buffer for Signal Activated Recording (SAR).
  // While waiting for the signal to commence, we record into this small buffer.
  // Once signal has actually started, we place this at the front of the
  // recording.
  int preroll_capacity;
  int preroll_write_index;
  int preroll_count;
  int preroll_audio_steps;
  float* preroll_left;
  float* preroll_right;
  float preroll_cv_left[PREROLL_CV_FRAMES * PORT_MAX_CHANNELS];
  float preroll_cv_right[PREROLL_CV_FRAMES * PORT_MAX_CHANNELS];

  // Setting this to a number above zero initiates a transfer.
  // The 0-offset of the last sample to copy.
  int endpoint_position;
  std::shared_ptr<Buffer> buffer;

  BrainwashThread()
      : shutdown{false},
        sample_rate{0.0f},
        max_sample_count{0},
        max_left_channels_seen{1},
        max_right_channels_seen{1},
        static_left{nullptr},
        static_right{nullptr},
        preroll_capacity{0},
        preroll_write_index{0},
        preroll_count{0},
        preroll_audio_steps{0},
        preroll_left{nullptr},
        preroll_right{nullptr},
        preroll_cv_left{},
        preroll_cv_right{},
        endpoint_position{-1} {}

  ~BrainwashThread() {
    Halt();
    if (static_left != nullptr) {
      delete[] static_left;
    }
    if (static_right != nullptr) {
      delete[] static_right;
    }
    if (preroll_left != nullptr) {
      delete[] preroll_left;
    }
    if (preroll_right != nullptr) {
      delete[] preroll_right;
    }
  }

  // Disallow copy and assignment.
  BrainwashThread(const BrainwashThread&) = delete;
  BrainwashThread& operator=(const BrainwashThread&) = delete;

  void Halt() { shutdown = true; }

  void SetRate(float rate) { sample_rate = rate; }

  void SetCV(bool cv) { cv_rate = cv; }

  void ResetChannels() {
    max_left_channels_seen = 1;
    max_right_channels_seen = 1;
  }

  void ResetPreRoll() {
    preroll_write_index = 0;
    preroll_count = 0;
    preroll_audio_steps = 0;
  }

  void RecordPreRollSample(const float* left_voltages, int left_chans,
                           const float* right_voltages, int right_chans) {
    if (static_right == nullptr) {
      return;
    }
    if (cv_rate) {
      if (sample_rate <= 1.0f) {
        // Not set yet, don't try to record anything.
        return;
      }
      int position = preroll_audio_steps++;
      // For CV, we only write when the fine-grained 'position' is at the center
      // of each millisecond portion of the memory. This is how we determine if
      // we are there.
      int cv_position = trunc((double)position * CV_SAMPLE_RATE / sample_rate);
      int writable_position =
          trunc((double)(cv_position + 0.5) * sample_rate / CV_SAMPLE_RATE);
      if (position == writable_position) {
        // Defensive clamping.
        left_chans = clamp(left_chans, 0, PORT_MAX_CHANNELS);
        right_chans = clamp(right_chans, 0, PORT_MAX_CHANNELS);
        max_left_channels_seen = std::max(max_left_channels_seen, left_chans);
        max_right_channels_seen =
            std::max(max_right_channels_seen, right_chans);
        int base = preroll_write_index * PORT_MAX_CHANNELS;
        for (int c = 0; c < PORT_MAX_CHANNELS; ++c) {
          preroll_cv_left[base + c] =
              (c < left_chans) ? left_voltages[c] : 0.0f;
          preroll_cv_right[base + c] =
              (c < right_chans) ? right_voltages[c] : 0.0f;
        }
        preroll_write_index = (preroll_write_index + 1) % PREROLL_CV_FRAMES;
        if (preroll_count < PREROLL_CV_FRAMES) {
          ++preroll_count;
        }
      }
    } else {
      // Don't store preroll if I have nowhere to put it.
      if (preroll_left == nullptr || preroll_right == nullptr ||
          preroll_capacity <= 0) {
        return;
      }
      preroll_left[preroll_write_index] =
          (left_chans > 0) ? left_voltages[0] : 0.0f;
      preroll_right[preroll_write_index] =
          (right_chans > 0) ? right_voltages[0] : 0.0f;
      // Circular buffer.
      preroll_write_index = (preroll_write_index + 1) % preroll_capacity;
      if (preroll_count < preroll_capacity) {
        ++preroll_count;
      }
    }
  }

  // Copies the buffered pre-roll into static_left/static_right starting at 0,
  // and returns the recording_position of the last pre-roll sample (-1 if
  // empty).
  int FlushPreRollToStatic() {
    if (static_left == nullptr || static_right == nullptr ||
        preroll_count <= 0) {
      ResetPreRoll();
      return -1;
    }
    if (cv_rate) {
      int count = std::min(preroll_count, PREROLL_CV_FRAMES);
      int start_idx =
          (preroll_write_index - count + PREROLL_CV_FRAMES) % PREROLL_CV_FRAMES;
      for (int i = 0; i < count; ++i) {
        int src_base =
            ((start_idx + i) % PREROLL_CV_FRAMES) * PORT_MAX_CHANNELS;
        int dst_base = i * PORT_MAX_CHANNELS;
        memcpy(&static_left[dst_base], &preroll_cv_left[src_base],
               sizeof(float) * PORT_MAX_CHANNELS);
        memcpy(&static_right[dst_base], &preroll_cv_right[src_base],
               sizeof(float) * PORT_MAX_CHANNELS);
      }
      ResetPreRoll();
      return std::max(
          0, (int)std::round(count * sample_rate / CV_SAMPLE_RATE) - 1);
    } else {
      if (preroll_left == nullptr || preroll_right == nullptr ||
          preroll_capacity <= 0) {
        ResetPreRoll();
        return -1;
      }
      int count =
          std::min(preroll_count, std::min(preroll_capacity, max_sample_count));
      int start_idx =
          (preroll_write_index - count + preroll_capacity) % preroll_capacity;
      for (int i = 0; i < count; ++i) {
        int src_idx = (start_idx + i) % preroll_capacity;
        static_left[i] = preroll_left[src_idx];
        static_right[i] = preroll_right[src_idx];
      }
      ResetPreRoll();
      return count - 1;
    }
  }

  void InitiateReplacement(int endpoint_position,
                           std::shared_ptr<Buffer> buffer) {
    int endpoint = std::min(endpoint_position, max_sample_count - 1);
    this->endpoint_position =
        cv_rate ? trunc(endpoint * CV_SAMPLE_RATE / sample_rate) : endpoint;
    this->buffer = buffer;
  }

  void RecordSample(int position, const float* left, int left_chans,
                    const float* right, int right_chans) {
    if (position >= max_sample_count || static_right == nullptr) {
      return;
    }
    if (cv_rate) {
      // Because there are many positions for each CV position, we need to only
      // allow a write to occur on one position per CV position. I want it to be
      // the "center" of the CV position.
      int cv_position = trunc((double)position * CV_SAMPLE_RATE / sample_rate);
      int writable_position =
          trunc((double)(cv_position + 0.5) * sample_rate / CV_SAMPLE_RATE);
      if (position == writable_position) {
        left_chans = clamp(left_chans, 0, PORT_MAX_CHANNELS);
        right_chans = clamp(right_chans, 0, PORT_MAX_CHANNELS);
        max_left_channels_seen = std::max(max_left_channels_seen, left_chans);
        max_right_channels_seen =
            std::max(max_right_channels_seen, right_chans);
        int base = cv_position * PORT_MAX_CHANNELS;
        for (int c = 0; c < PORT_MAX_CHANNELS; ++c) {
          static_left[base + c] = (c < left_chans) ? left[c] : 0.0f;
          static_right[base + c] = (c < right_chans) ? right[c] : 0.0f;
        }
      }
    } else {
      static_left[position] = (left_chans > 0) ? left[0] : 0.0f;
      static_right[position] = (right_chans > 0) ? right[0] : 0.0f;
    }
  }

  void Work() {
    while (!shutdown) {
      if (static_left == nullptr) {  // Not yet assigned.
        if (sample_rate > 1.0) {     // sample rate has been set
          // We can make the fixed buffers now.
          // Defaulting to 1 minute maximum.
          int sample_count = std::round(60 * sample_rate);
          int cv_max = std::round(60 * CV_SAMPLE_RATE) * PORT_MAX_CHANNELS;
          int alloc_size = std::max(sample_count, cv_max);
          max_sample_count = sample_count;
          static_left = new float[alloc_size]();
          static_right = new float[alloc_size]();
          preroll_capacity =
              std::max(1, (int)std::round(PREROLL_SECONDS * sample_rate));
          preroll_left = new float[preroll_capacity]();
          preroll_right = new float[preroll_capacity]();
        }
      } else if (endpoint_position > 0) {
        // Create new buffer to send to Memory.
        size_t length = endpoint_position + 1;
        // Since MemoryCV modules have multiple channels per position,
        // 'stride' is how many floats there are for each position.
        size_t stride = cv_rate ? PORT_MAX_CHANNELS : 1;
        float* new_left = new float[length * stride];
        float* new_right = new float[length * stride];
        // These allocs can take a small bit of time, check that we aren't being
        // shut down.
        if (shutdown) {
          delete[] new_left;
          delete[] new_right;
          return;
        }
        // Copies from zero to last position.
        memcpy(new_left, static_left, sizeof(float) * length * stride);
        memcpy(new_right, static_right, sizeof(float) * length * stride);

        // Now add to queue for Memory to consume.
        double seconds = cv_rate ? (endpoint_position + 1) / CV_SAMPLE_RATE
                                 : (endpoint_position + 1) / sample_rate;

        BufferTask* replace_task = BufferTask::ReplaceTask(
            new_left, new_right, nullptr,  // nullptr for status indicates we
                                           // don't need FileIO status info.
            endpoint_position + 1, seconds, true, max_left_channels_seen,
            max_right_channels_seen);
        if (!(buffer->replacements.queue.push(replace_task))) {
          delete[] replace_task->new_left_array;
          delete[] replace_task->new_right_array;
          delete replace_task;  // Queue is full, shed load.
        }
        endpoint_position = -1;
        // WARN("BufferTask sent!");
        // WARN("sender says replace_task->sample_count = %d",
        // replace_task->sample_count);
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
    THRESH_PARAM,
    STOP_AFTER_PARAM,
    SAR_BUTTON_PARAM,
    CANCEL_BUTTON_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RECORD_GATE_INPUT,
    LEFT_INPUT,
    RIGHT_INPUT,
    CLOCK_TRIGGER_INPUT,
    ARM_TRIGGER_INPUT,
    SAR_TRIGGER_INPUT,
    INPUTS_LEN
  };
  enum OutputId { OUTPUTS_LEN };
  enum LightId {
    CONNECTED_LIGHT,
    RECORD_BUTTON_LIGHT,
    ARM_BUTTON_LIGHT,
    SAR_BUTTON_LIGHT,
    CANCEL_BUTTON_LIGHT,
    LIGHTS_LEN
  };

  enum TimedRecordingState {
    NOT_RECORDING,      // Not armed.
    WAITING_FOR_CLOCK,  // Armed and waiting for the next clock to start
                        // recording.
    RECORDING
  };

  enum SARRecordingState {
    SAR_NOT_RECORDING,       // Not armed.
    SAR_WAITING_FOR_SIGNAL,  // Armed and waiting for signal above threshold.
    SAR_RECORDING            // Recording until STOP_AFTER seconds of silence.
  };

  // We look for the nearest Memory every NN samples. This saves CPU time.
  int find_memory_countdown = 0;
  std::shared_ptr<Buffer> buffer;

  // Where we are in the movement, before taking POSITION parameters into
  // account. Always 0.0 <= playback_position < length when Looping. Always 0.0
  // <= playback_position < 2 * length when Bouncing.
  int recording_position;

  // For background thread.
  BrainwashThread* worker;
  std::thread* thread;

  // For detecting the Gate that controls recording.
  dsp::SchmittTrigger recordTrigger;

  // Set if we were recording in the previous process() call.
  // TODO: make this an enum to parallel the other recording states.
  bool was_recording;
  // Set if a gate-driven manual recording was canceled while the gate was high.
  bool gate_recording_canceled = false;

  dsp::SchmittTrigger arm_trigger;
  TimedRecordingState timed_recording_state;
  // These can't be countdowns because COUNT knob might be changing, and I
  // should notice that.
  int clocks_so_far;
  int fractional_samples_so_far;

  // For Signal Activated Recording (SAR).
  dsp::SchmittTrigger sar_trigger;
  SARRecordingState sar_recording_state;
  int silent_samples_so_far;
  bool unchanging_means_silent = true;
  // The "SAR Anchors" are the set of voltages our inputs need to stay near to
  // be considered "unchanged".
  bool sar_anchor_initialized = false;
  float sar_anchor_left[PORT_MAX_CHANNELS] = {};
  float sar_anchor_right[PORT_MAX_CHANNELS] = {};

  // For tracking the tempo of CLOCK.
  dsp::SchmittTrigger tempo_trigger;
  int64_t last_tempo_trigger_frame = -1;
  int64_t tempo_length_in_frames = -1;

  Brainwash()
      : clocks_so_far{0},
        fractional_samples_so_far{0},
        sar_recording_state{SAR_NOT_RECORDING},
        silent_samples_so_far{0} {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(THRESH_PARAM, 0.0f, 1.0f, 0.02f,
                "Silence threshold for Signal Activated Recording", "V");
    configParam(STOP_AFTER_PARAM, 0.05f, 10.0f, 1.0f,
                "Length of silence needed to end Signal Activated Recording",
                " s");
    configSwitch(SAR_BUTTON_PARAM, 0, 1, 0,
                 "Press to activate Signal Activated Recording; press again "
                 "before completion to cancel",
                 {"Inactive", "Active"});
    configButton(CANCEL_BUTTON_PARAM,
                 "Press while recording to cancel and discard the current "
                 "recording");
    configInput(SAR_TRIGGER_INPUT,
                "Trigger received here will activate Signal Activated "
                "Recording");
    configInput(CLOCK_TRIGGER_INPUT,
                "Clock for recording length; Arm recording and will record for "
                "COUNT clocks");
    configInput(ARM_TRIGGER_INPUT,
                "Trigger received here will arm recording for COUNT clocks");
    configSwitch(ARM_BUTTON_PARAM, 0, 1, 0,
                 "Press to arm recording to start on next CLOCK trigger",
                 {"Disarmed", "Armed"});
    configParam(
        CLOCK_COUNT_PARAM, 0.001, 32, 4,
        "Number of repetitions of CLOCK to record for. May be fractional");
    configSwitch(RECORD_BUTTON_PARAM, 0, 1, 0,
                 "Press to record/release to stop this recording",
                 {"Inactive", "Recording"});
    configInput(RECORD_GATE_INPUT, "Gate to start/stop recording");
    configInput(LEFT_INPUT, "Left");
    configInput(RIGHT_INPUT, "Right");

    recording_position = -1;
    was_recording = false;
    timed_recording_state = NOT_RECORDING;

    // I'm using the state of the arm/SAR buttons to indicate if we are armed,
    // but we don't want armed state to get saved.
    params[ARM_BUTTON_PARAM].setValue(0.0f);
    params[SAR_BUTTON_PARAM].setValue(0.0f);

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

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "cv_change_detection",
                        json_boolean(unchanging_means_silent));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* cv_changeJ = json_object_get(rootJ, "cv_change_detection");
    if (cv_changeJ) {
      unchanging_means_silent = json_is_true(cv_changeJ);
    }
    // In case the button states were saved while any of these were triggered.
    params[RECORD_BUTTON_PARAM].setValue(0.0f);
    params[ARM_BUTTON_PARAM].setValue(0.0f);
    params[SAR_BUTTON_PARAM].setValue(0.0f);
  }

  // Business logic determining if signals being received in L+R mean that SAR
  // recording is currently considered "active".
  // Returns 'true' when definitely active, returns 'false' when *possibly* the
  // signal has become inactive.
  bool isSignalActive(const float* left_v, int l_chans, const float* right_v,
                      int r_chans, float threshold, bool is_cv) {
    if (is_cv && unchanging_means_silent) {
      if (!sar_anchor_initialized) {
        // Set the anchor voltages.
        for (int c = 0; c < PORT_MAX_CHANNELS; ++c) {
          sar_anchor_left[c] = (c < l_chans) ? left_v[c] : 0.0f;
          sar_anchor_right[c] = (c < r_chans) ? right_v[c] : 0.0f;
        }
        sar_anchor_initialized = true;
        return false;
      }
      bool changed = false;
      // Check left side.
      for (int c = 0; c < l_chans; ++c) {
        if (std::fabs(left_v[c] - sar_anchor_left[c]) > threshold) {
          changed = true;
          break;
        }
      }
      if (!changed) {
        // Check right side.
        for (int c = 0; c < r_chans; ++c) {
          if (std::fabs(right_v[c] - sar_anchor_right[c]) > threshold) {
            changed = true;
            break;
          }
        }
      }
      if (changed) {
        // If either side changed, reset the anchor voltages.
        for (int c = 0; c < PORT_MAX_CHANNELS; ++c) {
          sar_anchor_left[c] = (c < l_chans) ? left_v[c] : 0.0f;
          sar_anchor_right[c] = (c < r_chans) ? right_v[c] : 0.0f;
        }
      }
      return changed;
    } else {
      for (int c = 0; c < l_chans; ++c) {
        if (std::fabs(left_v[c]) > threshold) {
          return true;
        }
      }
      for (int c = 0; c < r_chans; ++c) {
        if (std::fabs(right_v[c]) > threshold) {
          return true;
        }
      }
      return false;
    }
  }

  void process(const ProcessArgs& args) override {
    // Confirm we're still talking to a Memory.
    // Only call this only every N samples, since the vast majority of
    // the time this won't change.
    // The number of modules it needs to go through does seem to increase the
    // CPU consummed by the module.
    if (args.sampleRate > 1.0) {
      worker->SetRate(args.sampleRate);
      if (--find_memory_countdown <= 0) {
        // One sixtieth of a second.
        find_memory_countdown = (int)(args.sampleRate / 60);

        buffer = findClosestMemory(getLeftExpander().module);
      }
    }

    // Even if not connected, we might still be getting tempo triggers.
    // So we'll track them just in case.
    bool clock_event = false;
    if (inputs[CLOCK_TRIGGER_INPUT].isConnected()) {
      bool tempo_was_high = tempo_trigger.isHigh();
      tempo_trigger.process(rescale(inputs[CLOCK_TRIGGER_INPUT].getVoltage(),
                                    0.1f, 2.f, 0.f, 1.f));
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

      int l_chan_count =
          buffer->cv_rate ? std::max(1, inputs[LEFT_INPUT].getChannels()) : 1;
      int r_chan_count =
          buffer->cv_rate ? std::max(1, inputs[RIGHT_INPUT].getChannels()) : 1;

      // Process trigger inputs. For some, we save the result in the button
      // itself.
      //
      // Are we armed to start recording on the next clock trigger?
      bool arm_was_triggered = arm_trigger.isHigh();
      arm_trigger.process(
          rescale(inputs[ARM_TRIGGER_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
      if (!arm_was_triggered && arm_trigger.isHigh()) {
        params[ARM_BUTTON_PARAM].setValue(1.0f);
      }

      // Are we armed for Signal Activated Recording (SAR)?
      bool sar_was_triggered = sar_trigger.isHigh();
      sar_trigger.process(
          rescale(inputs[SAR_TRIGGER_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
      if (!sar_was_triggered && sar_trigger.isHigh()) {
        params[SAR_BUTTON_PARAM].setValue(1.0f);
      }

      // Always keep recordTrigger updated so gate state is tracked accurately.
      recordTrigger.process(
          rescale(inputs[RECORD_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
      if (!recordTrigger.isHigh()) {
        gate_recording_canceled = false;
      }

      // Check if the user pressed red CANCEL button to abort any recording in
      // progress.
      if (params[CANCEL_BUTTON_PARAM].getValue() > 0.1f) {
        if (timed_recording_state != NOT_RECORDING) {
          timed_recording_state = NOT_RECORDING;
          params[ARM_BUTTON_PARAM].setValue(0.0f);
        }
        if (sar_recording_state != SAR_NOT_RECORDING) {
          sar_recording_state = SAR_NOT_RECORDING;
          params[SAR_BUTTON_PARAM].setValue(0.0f);
          worker->ResetPreRoll();
        }
        if (was_recording || params[RECORD_BUTTON_PARAM].getValue() > 0.1f) {
          params[RECORD_BUTTON_PARAM].setValue(0.0f);
          if (recordTrigger.isHigh()) {
            gate_recording_canceled = true;
          }
          was_recording = false;
        }
      }

      // Handle CLOCK-timed recording state machine.
      // May pass through multiple states in a single process() call, as this
      // makes for less repetition of the logic.
      if (timed_recording_state == NOT_RECORDING) {
        if (params[ARM_BUTTON_PARAM].getValue() > 0.1f) {
          // We are now armed.
          timed_recording_state = WAITING_FOR_CLOCK;
        }
      } else if (params[ARM_BUTTON_PARAM].getValue() <= 0.1f) {
        // User un-clicked ARM while waiting or recording: cancel!
        timed_recording_state = NOT_RECORDING;
      }
      if (timed_recording_state == WAITING_FOR_CLOCK) {
        if (clock_event) {
          // Start recording now.
          timed_recording_state = RECORDING;
          recording_position = -1;
          worker->ResetChannels();
          clocks_so_far =
              -1;  // Will be bumped to zero on this clock in next block.
          fractional_samples_so_far = 0;
        }
      }

      // Handle SAR state machine transitions.
      if (sar_recording_state == SAR_NOT_RECORDING) {
        if (params[SAR_BUTTON_PARAM].getValue() > 0.1f &&
            timed_recording_state != RECORDING) {
          sar_recording_state = SAR_WAITING_FOR_SIGNAL;
          worker->ResetChannels();
          worker->ResetPreRoll();
          sar_anchor_initialized = false;
          silent_samples_so_far = 0;
        }
      } else if (params[SAR_BUTTON_PARAM].getValue() <= 0.1f) {
        // User un-clicked SAR button before recording ended: cancel without
        // saving!
        sar_recording_state = SAR_NOT_RECORDING;
        worker->ResetPreRoll();
      }

      // Work through all of the states where we are or might be actually
      // recording.
      //
      // manual_recording is true when RECORD button or gate is invoked.
      bool manual_recording = false;
      // A small array of voltages.
      const float* left_voltages = inputs[LEFT_INPUT].getVoltages();
      const float* right_voltages = inputs[RIGHT_INPUT].getVoltages();
      // CLOCK-based recording.
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
            clocks_so_far +
                ((double)fractional_samples_so_far /
                 (tempo_length_in_frames > 0 ? tempo_length_in_frames : 1))) {
          // Still recording.
          ++recording_position;
          worker->RecordSample(recording_position, left_voltages, l_chan_count,
                               right_voltages, r_chan_count);
        } else {
          // CLOCK-based is done recording.
          worker->InitiateReplacement(recording_position, buffer);
          timed_recording_state = NOT_RECORDING;
          params[ARM_BUTTON_PARAM].setValue(0.0f);
        }
      } else if (sar_recording_state != SAR_NOT_RECORDING) {
        // SAR-based recording.
        bool signal_active = isSignalActive(
            left_voltages, l_chan_count, right_voltages, r_chan_count,
            params[THRESH_PARAM].getValue(), buffer->cv_rate);
        if (sar_recording_state == SAR_WAITING_FOR_SIGNAL) {
          if (signal_active) {
            // Stop waiting, we have signal.
            recording_position = worker->FlushPreRollToStatic();
            sar_recording_state = SAR_RECORDING;
            silent_samples_so_far = 0;
          } else {
            worker->RecordPreRollSample(left_voltages, l_chan_count,
                                        right_voltages, r_chan_count);
          }
        }
        if (sar_recording_state == SAR_RECORDING) {
          ++recording_position;
          worker->RecordSample(recording_position, left_voltages, l_chan_count,
                               right_voltages, r_chan_count);
          if (signal_active) {
            silent_samples_so_far = 0;
          } else {
            ++silent_samples_so_far;
          }
          int max_silent_samples =
              std::max(1, (int)std::round(params[STOP_AFTER_PARAM].getValue() *
                                          args.sampleRate));
          bool hit_max_length =
              (worker->max_sample_count > 0) &&
              (recording_position >= worker->max_sample_count - 1);
          if (silent_samples_so_far >= max_silent_samples || hit_max_length) {
            int endpoint =
                std::max(0, recording_position - silent_samples_so_far);
            worker->InitiateReplacement(endpoint, buffer);
            sar_recording_state = SAR_NOT_RECORDING;
            params[SAR_BUTTON_PARAM].setValue(0.0f);
          }
        }
      } else {
        // The logic for recording via COUNT/CLOCK and SAR is different enough
        // that it makes sense to handle it separately from the gate/button
        // recording.

        // If we're not already recording due to timed or SAR recording, see if
        // 'recording' just reflects the state of the button and the gate input.
        bool recording = (params[RECORD_BUTTON_PARAM].getValue() > 0.1f) ||
                         (recordTrigger.isHigh() && !gate_recording_canceled);

        if (recording && !was_recording) {
          // Starting from the top.
          recording_position = -1;
          worker->ResetChannels();
        } else if (!recording && was_recording) {
          // Recording done, send it!
          // WARN("Initiated brainwash at position %d", recording_position);
          worker->InitiateReplacement(recording_position, buffer);
        }
        was_recording = recording;
        manual_recording = recording;
        if (recording) {
          ++recording_position;
          worker->RecordSample(recording_position, left_voltages, l_chan_count,
                               right_voltages, r_chan_count);
        }
      }

      bool any_recording = (timed_recording_state == RECORDING) ||
                           (sar_recording_state == SAR_RECORDING) ||
                           manual_recording;
      lights[CANCEL_BUTTON_LIGHT].setBrightness(any_recording ? 1.0f : 0.0f);
      lights[RECORD_BUTTON_LIGHT].setBrightness(manual_recording ? 1.0f : 0.0f);
      lights[ARM_BUTTON_LIGHT].setBrightness(
          params[ARM_BUTTON_PARAM].getValue() > 0.1f ? 1.0f : 0.0f);
      lights[SAR_BUTTON_LIGHT].setBrightness(
          params[SAR_BUTTON_PARAM].getValue() > 0.1f ? 1.0f : 0.0f);
    } else {
      lights[CANCEL_BUTTON_LIGHT].setBrightness(0.0f);
      lights[RECORD_BUTTON_LIGHT].setBrightness(0.0f);
      lights[ARM_BUTTON_LIGHT].setBrightness(0.0f);
      lights[SAR_BUTTON_LIGHT].setBrightness(0.0f);
    }
    lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
  }
};

// For the big red cancel button.
template <typename TLightBase = RedLight>
struct LightPB61303 : PB61303 {
  app::ModuleLightWidget* light;

  LightPB61303() {
    light = new PB61303Light<TLightBase>;
    // Move center of light to center of box.
    light->box.pos = box.size.div(2).minus(light->box.size.div(2));
    addChild(light);
  }

  app::ModuleLightWidget* getLight() { return light; }
};

struct BrainwashWidget : ModuleWidget {
  VCVLightSlider<WhiteLight>* adjust_slider;

  explicit BrainwashWidget(Brainwash* module) : adjust_slider{nullptr} {
    setModule(module);
    setPanel(
        createPanel(asset::plugin(pluginInstance, "res/Brainwash.svg"),
                    asset::plugin(pluginInstance, "res/Brainwash-dark.svg")));

    // Big red CANCEL button at the top.
    addParam(createLightParamCentered<LightPB61303<RedLight>>(
        mm2px(Vec(12.7, 20.0)), module, Brainwash::CANCEL_BUTTON_PARAM,
        Brainwash::CANCEL_BUTTON_LIGHT));

    // Signal Activated Recording (SAR): THRESH, STOP AFTER, SAR trigger/button.
    addParam(createParamCentered<RoundSmallBlackKnob>(
        mm2px(Vec(6.35, 41.0)), module, Brainwash::THRESH_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(
        mm2px(Vec(19.05, 41.0)), module, Brainwash::STOP_AFTER_PARAM));
    addParam(
        createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(
            mm2px(Vec(19.05, 53.0)), module, Brainwash::SAR_BUTTON_PARAM,
            Brainwash::SAR_BUTTON_LIGHT));
    addInput(createInputCentered<ThemedPJ301MPort>(
        mm2px(Vec(6.35, 53.0)), module, Brainwash::SAR_TRIGGER_INPUT));

    // CLOCK and clock counting.
    addParam(createParamCentered<RoundSmallBlackKnob>(
        mm2px(Vec(6.35, 65.745)), module, Brainwash::CLOCK_COUNT_PARAM));
    addInput(createInputCentered<ThemedPJ301MPort>(
        mm2px(Vec(19.05, 65.745)), module, Brainwash::CLOCK_TRIGGER_INPUT));

    // ARM button and trigger.
    addParam(
        createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(
            mm2px(Vec(19.05, 77.745)), module, Brainwash::ARM_BUTTON_PARAM,
            Brainwash::ARM_BUTTON_LIGHT));
    addInput(createInputCentered<ThemedPJ301MPort>(
        mm2px(Vec(6.35, 77.745)), module, Brainwash::ARM_TRIGGER_INPUT));

    // Record button and trigger.
    addParam(
        createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(
            mm2px(Vec(19.05, 90.0)), module, Brainwash::RECORD_BUTTON_PARAM,
            Brainwash::RECORD_BUTTON_LIGHT));
    addInput(createInputCentered<ThemedPJ301MPort>(
        mm2px(Vec(6.35, 90.0)), module, Brainwash::RECORD_GATE_INPUT));

    // Signal Inputs.
    addInput(createInputCentered<ThemedPJ301MPort>(
        mm2px(Vec(14.638, 107.525)), module, Brainwash::LEFT_INPUT));
    addInput(createInputCentered<ThemedPJ301MPort>(
        mm2px(Vec(14.638, 117.844)), module, Brainwash::RIGHT_INPUT));

    // Our light is not colored, since we don't have a position in Depict.
    addChild(createLightCentered<MediumLight<WhiteLight>>(
        mm2px(Vec(12.7, 3.2)), module, Brainwash::CONNECTED_LIGHT));
  }

  void appendContextMenu(Menu* menu) override {
    Brainwash* module = dynamic_cast<Brainwash*>(this->module);
    if (module) {
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem(
          "SAR recording ends when all signals stop changing (for MemoryCV)",
          "", &module->unchanging_means_silent));
    }
    // Be a little clearer how to make this module do anything.
    menu->addChild(new MenuSeparator);
    menu->addChild(
        createMenuLabel("Brainwash only works when touching a group of modules "
                        "with a Memory or MemoryCV"));
    menu->addChild(
        createMenuLabel("module to the left. See my User Manual for details "
                        "and usage videos."));
  }
};

Model* modelBrainwash = createModel<Brainwash, BrainwashWidget>("Brainwash");
