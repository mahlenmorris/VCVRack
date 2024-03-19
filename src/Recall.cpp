#include <cmath>

#include "plugin.hpp"
#include "buffered.hpp"

struct Recall : PositionedModule {
	enum ParamId {
		BOUNCE_PARAM,
		SPEED_PARAM,
		ADJUST_PARAM,
		PLAY_BUTTON_PARAM,
		INIT_POSITION_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PLAY_GATE_INPUT,
		SPEED_INPUT,
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
		BOUNCE_LIGHT,
		CONNECTED_LIGHT,
		PLAY_BUTTON_LIGHT,
		LIGHTS_LEN
	};

	// We look for the nearest Memory every NN samples. This saves CPU time.
  int find_memory_countdown = 0;
  std::shared_ptr<Buffer> buffer;

	// Where we are in the movement, before taking POSITION parameters into account.
	// Always 0.0 <= playback_position < length when Looping.
	// Always 0.0 <= playback_position < 2 * length when Bouncing.
  double playback_position;

  // Where we are in memory (for the timestamp indicator).
  double display_position;

   // To detect when the ABS_POSITION_PARAM changes.
	double prev_abs_position;
	bool abs_changed; // Set when we detect movement, but only cleared when we use it.

  dsp::SchmittTrigger playTrigger;

  // To display timestamps correctly.
	double seconds = 0.0;
	int length = 0;

	// To fade volume when near a recording head.
	double fade = 1.0f;

	Recall() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(BOUNCE_PARAM, 0, 1, 0, "Endpoint Behavior",
								 {"Loop around", "Bounce"});
	  configParam(SPEED_PARAM, -10.f, 10.f, 1.f, "Playback speed/direction");
		configParam(ADJUST_PARAM, -10.f, 10.f, 0.f, "Adjust position");
		configSwitch(PLAY_BUTTON_PARAM, 0, 1, 0, "Press to start/stop this play head",
	               {"Silent", "Playing"});
		configParam(INIT_POSITION_PARAM, 0.f, 10.f, 0.f, "Initial position (0-10v) when loading patch");

		configInput(ABS_POSITION_INPUT, "Slider to move this within Memory.");
		configInput(SPEED_INPUT, "Playback speed (added to knob value)");
		configInput(PLAY_GATE_INPUT, "Gate to start/stop playing");
	
		configOutput(NOW_POSITION_OUTPUT, "0 - 10V point in Memory this is now reading");
		configOutput(LEFT_OUTPUT, "");
		configOutput(RIGHT_OUTPUT, "");

    line_record.position = 0.0;
		line_record.type = RECALL;
		prev_abs_position = -20.0;
		abs_changed = false;
		playback_position = -1;
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
		// If connected and buffer isn't empty.
		if (connected) {
			// These help the Timestamps UI widgets on this module.
			// While we could have Timestamp only pick these up from the Buffer,
			// This means that disconnecting the module doesn't zero-out the
			// Timestamp displays.
			// Bad things happen if these are zero, which sometimes happens on startup.
			length = std::max(buffer->length, 10);
			seconds = std::max(buffer->seconds, 1.0);

			if (playback_position == -1) { // Starting.
			  // Value of "start recording position indicator".
				playback_position = (int) (params[INIT_POSITION_PARAM].getValue() * length / 10.0);
			}

			// Are we in motion or not?
			playTrigger.process(rescale(
					inputs[PLAY_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));

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

			bool playing = !adjusting && 
			               ((params[PLAY_BUTTON_PARAM].getValue() > 0.1f) || playTrigger.isHigh());
			if (playing) {
				if (playback_position == -1) { // Starting.
					playback_position = 0;
				}
				playback_position +=
					inputs[SPEED_INPUT].getVoltage() + params[SPEED_PARAM].getValue();
			} else if (adjusting) {
     		// Even if we're not playing, we want to show movement caused by POSITION movement,
				// so user can see where playback will pick up.
				// 
				// Either the Adjust slider is non-zero or the ABS POSITION input has changed.
				// we'll let the human slider override the ABS input.
				if (std::fabs(params[ADJUST_PARAM].getValue()) > std::numeric_limits<float>::epsilon()) {
					// i.e., is not zero.
					// zero -> no movement.
					// 10 -> move entirety of length of buffer in two seconds.
					playback_position += (params[ADJUST_PARAM].getValue() / 20.0) * length / args.sampleRate;
				} else {
					// We'll just move directly to the specified spot.
					double abs = inputs[ABS_POSITION_INPUT].getVoltage();
					// Correct for values outside of 0-10.
					while (abs < 0.0) {
						abs += 10.0;
					}
					while (abs > 10.0) {
						abs -= 10.0;
					}
					playback_position = (abs * length / 10.0);
					abs_changed = false;
				}
			}

			// Fix the position, now that the adjustments have occured.
			switch (loop_type) {
				case 0: {  // Loop around.
					if (playback_position < 0.0) {
						playback_position += length;
					} else if (playback_position >= length) {
						playback_position -= length;
					}
				}
				break;
				case 1: {  // Bounce.
					if (playback_position < 0.0) {
						playback_position += 2 * length;
					} else if (playback_position >= 2 * length) {
						playback_position -= 2 * length;
					}
				}
				break;
			}

			display_position = playback_position;

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
							display_position = std::max(0.0, length * 2 - display_position - 1);
						} else {
							display_position -= length * 2;
						}
					}
					break;
				}
			}

			outputs[NOW_POSITION_OUTPUT].setVoltage(display_position * 10.0 / length);
			line_record.position = display_position;

			if (playing) {
				// Determine values to emit.
				if (buffer->NearHead(display_position)) {
					fade = std::max(fade - 0.02, 0.0);
				} else {
					if (fade < 1.0) {
						fade = std::min(fade + 0.02, 1.0);
					}
				}

				FloatPair gotten;
				buffer->Get(&gotten, display_position);
				outputs[LEFT_OUTPUT].setVoltage(fade * gotten.left);
				outputs[RIGHT_OUTPUT].setVoltage(fade * gotten.right);
				lights[PLAY_BUTTON_LIGHT].setBrightness(1.0f);
			} else {
				outputs[LEFT_OUTPUT].setVoltage(0.0f);
				outputs[RIGHT_OUTPUT].setVoltage(0.0f);
				lights[PLAY_BUTTON_LIGHT].setBrightness(0.0f);
			}
		} else {
			// Can only be playing if connected.
			lights[PLAY_BUTTON_LIGHT].setBrightness(0.0f);
		}
    lights[BOUNCE_LIGHT].setBrightness(loop_type == 1);
		lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
	}
};

struct NowTimestamp : TimestampField {
	NowTimestamp() {
  }

  Recall* module;

  double getPosition() override {
    if (module && module->length > 0 && module->seconds > 0.0) {
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

struct RecallWidget : ModuleWidget {
	RecallWidget(Recall* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Recall.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(6.035, 14.0)),
                                             module, Recall::BOUNCE_PARAM,
                                             Recall::BOUNCE_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.035, 97.087)), module, Recall::SPEED_INPUT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(19.05, 97.087)), module, Recall::SPEED_PARAM));

    addParam(createParamCentered<AdjustSlider>(mm2px(Vec(6.35, 43.0)),
		   module, Recall::ADJUST_PARAM));
    // TODO: make this a tiny attenuator knob?
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(19.05, 50.8)),
		   module, Recall::INIT_POSITION_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.05, 34.396)),
		   module, Recall::ABS_POSITION_INPUT));

		// Play button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(19.05, 80.0)),
                                             module, Recall::PLAY_BUTTON_PARAM,
                                             Recall::PLAY_BUTTON_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.035, 80.0)), module, Recall::PLAY_GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.7, 65.0)),
		                                           module, Recall::NOW_POSITION_OUTPUT));
		// A timestamp is 10 wide.
		NowTimestamp* now_timestamp = createWidget<NowTimestamp>(mm2px(
        Vec(12.7 - (10.0 / 2.0), 69.0)));
    now_timestamp->module = module;
    addChild(now_timestamp);

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.035, 112.0)), module, Recall::LEFT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.05, 112.0)), module, Recall::RIGHT_OUTPUT));

		ConnectedLight* connect_light = createLightCentered<ConnectedLight>(
			mm2px(Vec(12.7, 3.2)), module, Recall::CONNECTED_LIGHT);
    connect_light->pos_module = module;
		addChild(connect_light);
	}
};

Model* modelRecall = createModel<Recall, RecallWidget>("Recall");
