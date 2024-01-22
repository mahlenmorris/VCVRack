#include "plugin.hpp"

#include "buffered.hpp"

struct Recall : Module {
	enum ParamId {
		LOOP_PARAM,
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
		CONNECTED_LIGHT,
		PLAY_BUTTON_LIGHT,
		LIGHTS_LEN
	};

  double playback_position;
  dsp::SchmittTrigger playTrigger;

	// To help implement Bounce, we need to know when we're bouncing.
	bool invertSpeed;

  // To display timestamps correctly.
	double seconds = 0.0;
	int length = 0;

	// To fade volume when near a recording head.
	double fade = 1.0f;

	Recall() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(LOOP_PARAM, 0, 2, 0, "What to do when hitting the endpoints",
								 {"Loop around", "Bounce", "Go back to Position"});
		// This has distinct values.
    getParamQuantity(LOOP_PARAM)->snapEnabled = true;
	  configParam(SPEED_PARAM, -10.f, 10.f, 1.f, "Playback speed/direction");
		configParam(POSITION_PARAM, 0.f, 10.f, 0.f, "0 - 10V position we start at");
		configSwitch(PLAY_BUTTON_PARAM, 0, 1, 0, "Press to start/stop this play head",
	               {"Silent", "Playing"});
		configInput(SPEED_INPUT, "Playback speed (added to knob value)");
		configInput(PLAY_GATE_INPUT, "Gate to start/stop playing");
		configOutput(NOW_POSITION_OUTPUT, "0 - 10V point in Memory this is now reading");
		configOutput(LEFT_OUTPUT, "");
		configOutput(RIGHT_OUTPUT, "");

		invertSpeed = false;
		playback_position = -1;
	}

	void process(const ProcessArgs& args) override {
		// TODO: maybe call this only every N samples, since the vast majority of
		// the time this won't change.
		// The number of modules it needs to go through does seem to increase the
		// CPU consummed by the module.
		Buffer* buffer = findClosestMemory(getLeftExpander().module);
		bool connected = (buffer != nullptr);

		// If connected and buffer isn't empty.
		if (connected && buffer->length > 0) {
			length = buffer->length;
			seconds = buffer->seconds;
			playTrigger.process(rescale(
					inputs[PLAY_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
			bool playing = (params[PLAY_BUTTON_PARAM].getValue() > 0.1f) ||
			               playTrigger.isHigh();
			if (playing) {
				float* left_array = buffer->left_array;
				float* right_array = buffer->right_array;
				int loop_type = (int) params[LOOP_PARAM].getValue();
				if (loop_type != 1) {
					invertSpeed = false;
				}
				if (playback_position == -1) { // Starting.
					playback_position = length * (params[POSITION_PARAM].getValue() / 10.0);
				}
				playback_position +=
					(inputs[SPEED_INPUT].getVoltage() + params[SPEED_PARAM].getValue()) *
					(invertSpeed ? -1 : 1);
				// Behavior at the endpoint depends on the LOOP setting.
				if ((playback_position < 0) || (playback_position >= length)) {
					switch (loop_type) {
						case 0: {  // Loop around.
							while (playback_position < 0) {
								playback_position += length;
							}
							while (playback_position >= length) {
								playback_position -= length;
							}
						}
						break;
						case 1: {  // Bounce.
							invertSpeed = !invertSpeed;
							playback_position +=
							 (inputs[SPEED_INPUT].getVoltage() + params[SPEED_PARAM].getValue()) *
							 (invertSpeed ? -1 : 1);
						}
						break;
						case 2: {
							playback_position = length * (params[POSITION_PARAM].getValue() / 10.0);
						}
            break;
					}
				}
				while (playback_position < 0) {
					playback_position += length;
				}
				while (playback_position >= length) {
					playback_position -= length;
				}
				outputs[NOW_POSITION_OUTPUT].setVoltage(playback_position * 10.0 / length);

				// Determine values to emit.
				int playback_start = trunc(playback_position);
				int playback_end = trunc(playback_start + 1);
				if (playback_end >= length) {
					playback_end -= length;  // Should be zero.
				}
				float start_fraction = playback_position - playback_start;
				if (buffer->NearHead(playback_start)) {
					fade = std::max(fade - 0.02, 0.0);
				} else {
					if (fade < 1.0) {
						fade = std::min(fade + 0.02, 1.0);
					}
				}
				outputs[LEFT_OUTPUT].setVoltage(fade *
					(left_array[playback_start] * (1.0 - start_fraction) +
				   left_array[playback_end] * (start_fraction)));
				outputs[RIGHT_OUTPUT].setVoltage(fade *
					(right_array[playback_start] * (1.0 - start_fraction) +
				   right_array[playback_end] * (start_fraction)));
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

		lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
	}
};

struct StartTimestamp : TimestampField {
	StartTimestamp() {
  }

  Recall* module;

  double getPosition() override {
    if (module && module->length > 0) {
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
    if (module && module->length > 0) {
			return module->playback_position * module->seconds / module->length;
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

		Trimpot* loop_knob = createParamCentered<Trimpot>(
        mm2px(Vec(8.024, 19.3)), module, Recall::LOOP_PARAM);
    loop_knob->minAngle = -0.28f * M_PI;
    loop_knob->maxAngle = 0.28f * M_PI;
    loop_knob->snap = true;
    addParam(loop_knob);

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 32.0)), module, Recall::SPEED_INPUT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.971, 32.0)), module, Recall::SPEED_PARAM));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(8.024, 54.0)), module, Recall::POSITION_PARAM));
		// A timestamp is 14 wide.
		StartTimestamp* start_timestamp = createWidget<StartTimestamp>(mm2px(
        Vec(8.024 - (14.0 / 2.0), 59.0)));
    start_timestamp->module = module;
    addChild(start_timestamp);

		// Play button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 80.0)),
                                             module, Recall::PLAY_BUTTON_PARAM,
                                             Recall::PLAY_BUTTON_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 80.0)), module, Recall::PLAY_GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.971, 54.0)), module, Recall::NOW_POSITION_OUTPUT));
		// A timestamp is 14 wide.
		NowTimestamp* now_timestamp = createWidget<NowTimestamp>(mm2px(
        Vec(20.971 - (14.0 / 2.0), 59.0)));
    now_timestamp->module = module;
    addChild(now_timestamp);

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.024, 112.0)), module, Recall::LEFT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.971, 112.0)), module, Recall::RIGHT_OUTPUT));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(3.394, 7.56)), module, Recall::CONNECTED_LIGHT));
	}
};

Model* modelRecall = createModel<Recall, RecallWidget>("Recall");
