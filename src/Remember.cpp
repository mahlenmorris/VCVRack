#include "plugin.hpp"

#include "buffered.hpp"

struct Remember : Module {
	enum ParamId {
		LOOP_PARAM,
		POSITION_PARAM,
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
		NOW_POSITION_OUTPUT,
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		CONNECTED_LIGHT,
		RECORD_BUTTON_LIGHT,
		LIGHTS_LEN
	};

	int recording_position;
  dsp::SchmittTrigger recordTrigger;

	// To help implement Bounce, we need to know when we're bouncing.
	bool invertSpeed;

	// To display timestamps correctly.
	double seconds = 0.0;
	int length = 0;

	Remember() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(LOOP_PARAM, 0, 2, 0, "What to do when hitting the endpoints",
								 {"Loop around", "Bounce", "Go back to Position"});
		// This has distinct values.
    getParamQuantity(LOOP_PARAM)->snapEnabled = true;
		configParam(POSITION_PARAM, 0.f, 10.f, 0.f, "Starting position");
		configSwitch(RECORD_BUTTON_PARAM, 0, 1, 0, "Press to start/stop this record head",
	               {"Inactive", "Recording"});
		configInput(RECORD_GATE_INPUT, "Gate to start/stop recording");
		configOutput(LEFT_OUTPUT, "");
		configOutput(RIGHT_OUTPUT, "");
		configOutput(NOW_POSITION_OUTPUT, "");
		configInput(LEFT_INPUT, "");
		configInput(RIGHT_INPUT, "");

		invertSpeed = false;
		recording_position = -1;
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
			recordTrigger.process(rescale(
					inputs[RECORD_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
			bool recording = (params[RECORD_BUTTON_PARAM].getValue() > 0.1f) ||
			               recordTrigger.isHigh();
			if (recording) {
				int length = buffer->length;
				float* left_array = buffer->left_array;
				float* right_array = buffer->right_array;
				int loop_type = (int) params[LOOP_PARAM].getValue();
				if (loop_type != 1) {
					invertSpeed = false;
				}
				if (recording_position == -1) { // Starting.
					recording_position = trunc(length * (params[POSITION_PARAM].getValue() / 10.0));
				}

				recording_position += (invertSpeed ? -1 : 1);
				// Behavior at the endpoint depends on the LOOP setting.
				if ((recording_position < 0) || (recording_position >= length)) {
					switch (loop_type) {
						case 0: {  // Loop around.
							while (recording_position < 0) {
								recording_position += length;
							}
							while (recording_position >= length) {
								recording_position -= length;
							}
						}
						break;
						case 1: {  // Bounce.
							invertSpeed = !invertSpeed;
							recording_position += (invertSpeed ? -1 : 1);
						}
						break;
						case 2: {
							recording_position = length * (params[POSITION_PARAM].getValue() / 10.0);
						}
            break;
					}
				}
				while (recording_position < 0) {
					recording_position += length;
				}
				while (recording_position >= length) {
					recording_position -= length;
				}
				outputs[LEFT_OUTPUT].setVoltage(left_array[recording_position]);
				outputs[RIGHT_OUTPUT].setVoltage(right_array[recording_position]);
				outputs[NOW_POSITION_OUTPUT].setVoltage(recording_position * 10.0 / length);

				// This module is optimized for recording one sample to one integral position
				// in array. Later modules can figure out how to do fancier stuff.
				buffer->Set(recording_position,
					 inputs[LEFT_INPUT].getVoltage(), inputs[RIGHT_INPUT].getVoltage(),
				   getId());
				lights[RECORD_BUTTON_LIGHT].setBrightness(1.0f);
			} else {
				lights[RECORD_BUTTON_LIGHT].setBrightness(0.0f);
			}
		} else {
			// Can only be recording if connected.
			lights[RECORD_BUTTON_LIGHT].setBrightness(0.0f);
		}

		lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
	}
};

struct NowRememberTimestamp : TimestampField {
	NowRememberTimestamp() {
  }

  Remember* module;

  double getPosition() override {
    if (module && module->length > 0) {
			return module->recording_position * module->seconds / module->length;
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

struct RememberWidget : ModuleWidget {
	RememberWidget(Remember* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Remember.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		Trimpot* loop_knob = createParamCentered<Trimpot>(
        mm2px(Vec(8.024, 32.0)), module, Remember::LOOP_PARAM);
    loop_knob->minAngle = -0.28f * M_PI;
    loop_knob->maxAngle = 0.28f * M_PI;
    loop_knob->snap = true;
    addParam(loop_knob);
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 48.0)), module, Remember::POSITION_PARAM));

		// Record button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 63.067)),
                                             module, Remember::RECORD_BUTTON_PARAM,
                                             Remember::RECORD_BUTTON_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 63.067)), module,
		                                         Remember::RECORD_GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 82.285)),
		                                           module, Remember::NOW_POSITION_OUTPUT));
		// A timestamp is 14 wide.
		NowRememberTimestamp* now_timestamp = createWidget<NowRememberTimestamp>(mm2px(
        Vec(15.24 - (14.0 / 2.0), 87.286)));
    now_timestamp->module = module;
    addChild(now_timestamp);

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.814, 99.412)),
		                                           module, Remember::LEFT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.761, 99.412)),
		                                           module, Remember::RIGHT_OUTPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 112.0)), module,
		                                         Remember::LEFT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.971, 112.0)), module,
		                                         Remember::RIGHT_INPUT));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(3.394, 7.56)), module, Remember::CONNECTED_LIGHT));
	}
};


Model* modelRemember = createModel<Remember, RememberWidget>("Remember");
