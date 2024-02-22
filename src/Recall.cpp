#include <cmath>

#include "plugin.hpp"
#include "buffered.hpp"

struct Recall : PositionedModule {
	enum ParamId {
		BOUNCE_PARAM,
		SPEED_PARAM,
		POSITION_PARAM,
		PLAY_BUTTON_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PLAY_GATE_INPUT,
		SPEED_INPUT,
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

	// Where we are in the movement, before taking POSITION parameters into account.
	// Always 0.0 <= playback_position < length when Looping.
	// Always 0.0 <= playback_position < 2 * length when Bouncing.
  double playback_position;

  // Where we are in memory (for the timestamp indicator).
  double display_position;

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
		configParam(POSITION_PARAM, 0.f, 10.f, 0.f, "0 - 10V position we start at");
		configSwitch(PLAY_BUTTON_PARAM, 0, 1, 0, "Press to start/stop this play head",
	               {"Silent", "Playing"});
		configInput(SPEED_INPUT, "Playback speed (added to knob value)");
		configInput(PLAY_GATE_INPUT, "Gate to start/stop playing");
		configOutput(NOW_POSITION_OUTPUT, "0 - 10V point in Memory this is now reading");
		configOutput(LEFT_OUTPUT, "");
		configOutput(RIGHT_OUTPUT, "");

    line_record.position = 0.0;
		line_record.type = RECALL;
		playback_position = -1;
	}

	void process(const ProcessArgs& args) override {
		// TODO: maybe call this only every N samples, since the vast majority of
		// the time this won't change.
		// The number of modules it needs to go through does seem to increase the
		// CPU consummed by the module.
		Buffer* buffer = findClosestMemory(getLeftExpander().module);
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

			// Are we in motion or not?
			playTrigger.process(rescale(
					inputs[PLAY_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
			bool playing = (params[PLAY_BUTTON_PARAM].getValue() > 0.1f) ||
			               playTrigger.isHigh();
			if (playing) {
				if (playback_position == -1) { // Starting.
					playback_position = 0;
				}
				playback_position +=
					inputs[SPEED_INPUT].getVoltage() + params[SPEED_PARAM].getValue();
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
			}

			// Even if we're not playing, we want to show movement caused by POSITION movement,
			// so user can see where playback will pick up. 
			// Now add the influence of POSITION parameter(s).
			double offset = (loop_type == 1 ? 2.0 : 1.0) * length *
			  (params[POSITION_PARAM].getValue() / 10.0);
			display_position = playback_position + offset;

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

struct StartTimestamp : TimestampField {
	StartTimestamp() {
  }

  Recall* module;

  double getPosition() override {
    if (module && module->seconds > 0) {
			return module->params[Recall::POSITION_PARAM].getValue() * module->seconds / 10.0;
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

struct RecallWidget : ModuleWidget {
	RecallWidget(Recall* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Recall.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(8.024, 19.3)),
                                             module, Recall::BOUNCE_PARAM,
                                             Recall::BOUNCE_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 32.0)), module, Recall::SPEED_INPUT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.971, 32.0)), module, Recall::SPEED_PARAM));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(8.024, 54.0)), module, Recall::POSITION_PARAM));
		// A timestamp is 10 wide.
		StartTimestamp* start_timestamp = createWidget<StartTimestamp>(mm2px(
        Vec(8.024 - (10.0 / 2.0), 59.0)));
    start_timestamp->module = module;
    addChild(start_timestamp);

		// Play button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 80.0)),
                                             module, Recall::PLAY_BUTTON_PARAM,
                                             Recall::PLAY_BUTTON_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 80.0)), module, Recall::PLAY_GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.971, 54.0)), module, Recall::NOW_POSITION_OUTPUT));
		// A timestamp is 10 wide.
		NowTimestamp* now_timestamp = createWidget<NowTimestamp>(mm2px(
        Vec(20.971 - (10.0 / 2.0), 59.0)));
    now_timestamp->module = module;
    addChild(now_timestamp);

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.024, 112.0)), module, Recall::LEFT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.971, 112.0)), module, Recall::RIGHT_OUTPUT));

		ConnectedLight* connect_light = createLightCentered<ConnectedLight>(
			mm2px(Vec(14.240, 3.0)), module, Recall::CONNECTED_LIGHT);
    connect_light->module = module;
		addChild(connect_light);
	}
};

Model* modelRecall = createModel<Recall, RecallWidget>("Recall");
