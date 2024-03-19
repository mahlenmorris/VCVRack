#include "plugin.hpp"

#include "buffered.hpp"

struct Embellish : PositionedModule {
	enum ParamId {
		BOUNCE_PARAM,
		ADJUST_PARAM,
		RECORD_BUTTON_PARAM,
		INIT_POSITION_PARAM,
		REVERSE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RECORD_GATE_INPUT,
		LEFT_INPUT,
		RIGHT_INPUT,
		ABS_POSITION_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		NOW_POSITION_OUTPUT,
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		CONNECTED_LIGHT,
		RECORD_BUTTON_LIGHT,
		BOUNCE_LIGHT,
		REVERSE_LIGHT,
		LIGHTS_LEN
	};

  enum RecordState {
    // We have a few states we could be in.
		ADJUSTING,  // * Not recording, but actively moving.
		NO_RECORD,	// * Not recording at all.
		FADE_UP,  	// * Starting to record.
		RECORDING, 	// * Continuing to record.
		FADE_DOWN		// * Fading out the recording.
		    				// * And back to not recording at all.

		// When starting and stopping the record head, the sequence is:
		// * NO_RECORD -> FADE_UP -> RECORDING -> FADE_DOWN -> NO_RECORD.
		// If user starts adjusting in FADE_UP or RECORDING, we move to FADE_DOWN and then NO_RECORD.
		// If user is adjusting when in NO_RECORD, we move to ADJUSTING, and stay there until
		// user stops adjusting.
	};

	// We look for the nearest Memory every NN samples. This saves CPU time.
  int find_memory_countdown = 0;
  std::shared_ptr<Buffer> buffer;

	// Where we are in the movement, before taking POSITION parameters into account.
	// Always 0.0 <= playback_position < length when Looping.
	// Always 0.0 <= playback_position < 2 * length when Bouncing.
	double recording_position;

	// Where we are in memory (for the timestamp indicator).
  int display_position;

  // To detect when the ABS_POSITION_PARAM changes.
	double prev_abs_position;
	bool abs_changed; // Set when we detect movement, but only cleared when we use it.

  dsp::SchmittTrigger recordTrigger;

	// To display timestamps correctly.
	double seconds = 0.0;
	int length = 0;

	RecordState record_state;

	Embellish() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(BOUNCE_PARAM, 0, 1, 0, "Endpoint Behavior",
								 {"Loop around", "Bounce"});
		configSwitch(REVERSE_PARAM, 0, 1, 0, "Recording Direction",
								 {"Forward", "Reverse"});
		configParam(ADJUST_PARAM, -10.f, 10.f, 0.f, "Adjust position");
		configSwitch(RECORD_BUTTON_PARAM, 0, 1, 0, "Press to start/stop this record head",
	               {"Inactive", "Recording"});
		configParam(INIT_POSITION_PARAM, 0.f, 10.f, 0.f, "Initial position (0-10v) when loading patch");

		// TODO: fill all of these in!
		configInput(ABS_POSITION_INPUT, "Slider to move this within Memory.");
		configInput(RECORD_GATE_INPUT, "Gate to start/stop recording");
		configInput(LEFT_INPUT, "");
		configInput(RIGHT_INPUT, "");

		configOutput(LEFT_OUTPUT, "");
		configOutput(RIGHT_OUTPUT, "");
		configOutput(NOW_POSITION_OUTPUT, "0 - 10V point in Memory this is now reading/writing");

		line_record.position = 0.0;
		line_record.type = EMBELLISH;
		recording_position = -1;
		prev_abs_position = -20.0;
		abs_changed = false;
		record_state = NO_RECORD;
	}

	void process(const ProcessArgs& args) override {
		// Only call this only every N samples, since the vast majority of
		// the time this won't change.
		// The number of modules it needs to go through does seem to increase the
		// CPU consummed by the module.
		if (--find_memory_countdown <= 0) {
      // One hundredth of a second.
      find_memory_countdown = (int) (args.sampleRate / 100);

			buffer = findClosestMemory(getLeftExpander().module);
		}

		bool connected = (buffer != nullptr) && buffer->IsValid();
		int loop_type = (int) params[BOUNCE_PARAM].getValue();
		bool reverse = params[REVERSE_PARAM].getValue() > 0.1;

		// If connected and buffer isn't empty.
		if (connected) {
			// These help the Timestamps UI widgets on this module.
			// While we could have Timestamp only pick these up from the Buffer,
			// This means that disconnecting the module doesn't zero-out the
			// Timestamp displays.
			length = buffer->length;
			seconds = buffer->seconds;

			// Are we in motion or not?
			recordTrigger.process(rescale(
					inputs[RECORD_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
			// 'recording' just reflects the state of the button and the gate input.
			bool recording = (params[RECORD_BUTTON_PARAM].getValue() > 0.1f) ||
			               recordTrigger.isHigh();
			// User (or input) is adjusting the position.
			if (inputs[ABS_POSITION_INPUT].getVoltage() != prev_abs_position) {
				if (prev_abs_position == -20.0) {
					// Getting an initial value, if any.
					prev_abs_position = inputs[ABS_POSITION_INPUT].getVoltage();
				} else {
					prev_abs_position = inputs[ABS_POSITION_INPUT].getVoltage();
					abs_changed = true;
				}
			}
			bool adjusting = abs_changed ||
			    std::fabs(params[ADJUST_PARAM].getValue()) > std::numeric_limits<float>::epsilon(); // i.e., is not zero.

			// Let's figure out what RecordState to be in.
			// Take into account when the user starts or stops adjusting.
			if (record_state == NO_RECORD && adjusting) {
				record_state = ADJUSTING;
			} else if (record_state == ADJUSTING && !adjusting) {
				record_state = NO_RECORD;
			}

			switch (record_state) {
				case NO_RECORD:
				case FADE_DOWN: {
					if (recording && !adjusting) {
						record_state = FADE_UP;
					}
				}
				break;
				case FADE_UP:
				case RECORDING:  {
					if (!recording || adjusting) {
						record_state = FADE_DOWN;
					}
				}
				break;
				case ADJUSTING:
				break;
			}

			// Ending a recording means we need a Smooth.
      if (record_state == FADE_DOWN) {
				if (buffer->smooths.additions.size() < buffer->smooths.additions.max_size()) {
					Smooth* new_smooth = new Smooth(display_position + 1, true);
					// This isn't strictly kosher, since multiple Embellish modules could be pushing
					// a Smooth onto the queue at the same time, and the NoLockQueue is rated as safe
					// for only one writer.
					// TODO: address this by having each Embellish create it's own NoLockQueue, which
					// Memory dumps into it's queue. Or else let an Embellish worker thread handle the smoothing?
					buffer->smooths.additions.push(new_smooth);
				}
				record_state = NO_RECORD;
			}

			// This is all to figure out the next position in the memory to go to.
			if (record_state != NO_RECORD) {  // We're still moving, either foward or because user is adjusting.
				if (recording_position == -1) { // Starting.
				  // Value of "start recording position indicator".
					recording_position = (int) (params[INIT_POSITION_PARAM].getValue() * length / 10.0);
				}

				double adjust = reverse ? -1 : 1;
				if (record_state == ADJUSTING) {
					// Either the Adjust slider is non-zero or the ABS POSITION input has changed.
					// we'll let the human slider override the ABS input.
					if (std::fabs(params[ADJUST_PARAM].getValue()) > std::numeric_limits<float>::epsilon()) {
					  // i.e., is not zero.
	  				// zero -> no movement.
		  			// 10 -> move entirety of length of buffer in two seconds.
						adjust = (params[ADJUST_PARAM].getValue() / 20.0) * length / args.sampleRate;
					} else {
						// We'll just move directly to the specified spot.
						double abs = inputs[ABS_POSITION_INPUT].getVoltage();
						while (abs < 0.0) {
							abs += 10.0;
						}
						while (abs > 10.0) {
							abs -= 10.0;
						}
						recording_position = (int) (abs * length / 10.0);
						adjust = 0;
						abs_changed = false;
					}
				}

				recording_position += adjust;
				switch (loop_type) {
					case 0: {  // Loop around.
						if (recording_position < 0) {
							recording_position += length;
						} else if (recording_position >= length) {
							recording_position -= length;
						}
					}
					break;
					case 1: {  // Bounce.
						if (recording_position < 0) {
							recording_position += 2 * length;
						} else if (recording_position >= 2 * length) {
							recording_position -= 2 * length;
						}
					}
					break;
				}
			}

			display_position = (int) floor(recording_position);

			if (record_state == FADE_UP) {
				if (buffer->smooths.additions.size() < buffer->smooths.additions.max_size()) {
					Smooth* new_smooth = new Smooth(display_position, false);
					buffer->smooths.additions.push(new_smooth);
				}
				record_state = RECORDING;
			}

			while (display_position > 2 * length) {
				display_position -= 2 * length;
			}

			if (display_position >= length) {
				switch (loop_type) {
					case 0: {  // Loop around.
						display_position -= length;
					}
					break;
					case 1: {  // Bounce.
						// There might be simpler math for this, it just escapes me now.
						if (display_position < 2 * length) {
							display_position = std::max(0, length * 2 - display_position - 1);
						} else {
							display_position -= length * 2;
						}
					}
					break;
				}
			}
			outputs[NOW_POSITION_OUTPUT].setVoltage(display_position * 10.0 / length);
			// So Display knows where we are.
			line_record.position = (double) display_position;

			if (record_state != NO_RECORD && record_state != ADJUSTING) {  // Still recording.
				FloatPair gotten;
				buffer->Get(&gotten, display_position);

				// TODO(clicks): Should do fade on outputs if we detect we're
				// near *another* record head (we're always near this one!).
				outputs[LEFT_OUTPUT].setVoltage(gotten.left);
				outputs[RIGHT_OUTPUT].setVoltage(gotten.right);

				// This module is optimized for recording one sample to one integral position
				// in array. Later modules can figure out how to do fancier stuff.
				buffer->Set(display_position,
					inputs[LEFT_INPUT].getVoltage(),
					inputs[RIGHT_INPUT].getVoltage(),
					getId());
				lights[RECORD_BUTTON_LIGHT].setBrightness(1.0f);
			} else {
				lights[RECORD_BUTTON_LIGHT].setBrightness(0.0f);
			}
		} else {
			// Can only be recording if connected.
			outputs[LEFT_OUTPUT].setVoltage(0.0f);
			outputs[RIGHT_OUTPUT].setVoltage(0.0f);
			lights[RECORD_BUTTON_LIGHT].setBrightness(0.0f);
		}
    lights[REVERSE_LIGHT].setBrightness(reverse);
    lights[BOUNCE_LIGHT].setBrightness(loop_type == 1);
		lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
	}
};

struct NowEmbellishTimestamp : TimestampField {
	NowEmbellishTimestamp() {
  }

  Embellish* module;

  double getPosition() override {
    if (module && module->length > 0) {
			return module->display_position * module->seconds / module->length;
		}
		return 0.00;  // Dummy display value.
  }

	double getSeconds() override {
    if (module && module->seconds > 0.0) {
			return module->seconds;
		}
		return 2.0;
	}
};

struct AdjustSlider : VCVSlider {
  void onDragEnd(const DragEndEvent& e) override {
    getParamQuantity()->setValue(0.0);
	}
};


struct EmbellishWidget : ModuleWidget {
  VCVLightSlider<WhiteLight>* adjust_slider;

	EmbellishWidget(Embellish* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Embellish.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(6.35, 14.0)),
                                             module, Embellish::BOUNCE_PARAM,
                                             Embellish::BOUNCE_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(19.05, 14.0)),
                                             module, Embellish::REVERSE_PARAM,
                                             Embellish::REVERSE_LIGHT));

    addParam(createParamCentered<AdjustSlider>(mm2px(Vec(6.35, 43.0)),
		   module, Embellish::ADJUST_PARAM));

    // TODO: make this a tiny attenuator knob?
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(19.05, 50.8)),
		   module, Embellish::INIT_POSITION_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.05, 34.396)),
		   module, Embellish::ABS_POSITION_INPUT));

		// Record button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(19.05, 87.408)),
                                             module, Embellish::RECORD_BUTTON_PARAM,
                                             Embellish::RECORD_BUTTON_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.35, 87.408)), module,
		                                         Embellish::RECORD_GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.7, 65.0)),
		                                           module, Embellish::NOW_POSITION_OUTPUT));
		// A timestamp is 10 wide.
		NowEmbellishTimestamp* now_timestamp = createWidget<NowEmbellishTimestamp>(mm2px(
        Vec(12.7 - (10.0 / 2.0), 69.0)));
    now_timestamp->module = module;
    addChild(now_timestamp);

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.35, 103.646)),
		                                           module, Embellish::LEFT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.35, 113.965)),
		                                           module, Embellish::RIGHT_OUTPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.05, 103.646)), module,
		                                         Embellish::LEFT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.05, 113.965)), module,
		                                         Embellish::RIGHT_INPUT));

		ConnectedLight* connect_light = createLightCentered<ConnectedLight>(
			mm2px(Vec(12.7, 3.0)), module, Embellish::CONNECTED_LIGHT);
    connect_light->pos_module = module;
		addChild(connect_light);
	}

/*
  void step() override {
		// Kinda silly to do this every step(), but checking whether or not
		// we should copy the value would take longer, I suspect.
		if (module) {
		  adjust_slider->getLight()->baseColors[0] = dynamic_cast<PositionedModule*>(module)->line_record.color;
		}
		ModuleWidget::step();
	}
*/
};


Model* modelEmbellish = createModel<Embellish, EmbellishWidget>("Embellish");
