#include "plugin.hpp"

#include "buffered.hpp"

struct Remember : PositionedModule {
	enum ParamId {
		BOUNCE_PARAM,
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
		BOUNCE_LIGHT,
		LIGHTS_LEN
	};

	// Where we are in the movement, before taking POSITION parameters into account.
	// Always 0.0 <= playback_position < length when Looping.
	// Always 0.0 <= playback_position < 2 * length when Bouncing.
	int recording_position;

	// Where we are in memory (for the timestamp indicator).
  int display_position;

  dsp::SchmittTrigger recordTrigger;

	// To help implement Bounce, we need to know when we're bouncing.
	bool invertSpeed;

	// To display timestamps correctly.
	double seconds = 0.0;
	int length = 0;

	Remember() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(BOUNCE_PARAM, 0, 1, 0, "Endpoint Behavior",
								 {"Loop around", "Bounce"});
		configParam(POSITION_PARAM, 0.f, 10.f, 0.f, "Starting position");
		configSwitch(RECORD_BUTTON_PARAM, 0, 1, 0, "Press to start/stop this record head",
	               {"Inactive", "Recording"});
		configInput(RECORD_GATE_INPUT, "Gate to start/stop recording");
		configOutput(LEFT_OUTPUT, "");
		configOutput(RIGHT_OUTPUT, "");
		configOutput(NOW_POSITION_OUTPUT, "");
		configInput(LEFT_INPUT, "");
		configInput(RIGHT_INPUT, "");

		line_record.position = 0.0;
		line_record.type = REMEMBER;
		recording_position = -1;
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
			length = buffer->length;
			seconds = buffer->seconds;

			// Are we in motion or not?
			recordTrigger.process(rescale(
					inputs[RECORD_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
			bool recording = (params[RECORD_BUTTON_PARAM].getValue() > 0.1f) ||
			               recordTrigger.isHigh();
			if (recording) {
				if (recording_position == -1) { // Starting.
					recording_position = 0;
				}

				recording_position += 1;
				switch (loop_type) {
					case 0: {  // Loop around.
						if (recording_position < 0) {
							// Can't happen now, but might add a reverse speed.
							recording_position += length;
						} else if (recording_position >= length) {
							recording_position -= length;
						}
					}
					break;
					case 1: {  // Bounce.
						if (recording_position < 0) {
							// Can't happen now, but might add a reverse speed.
							recording_position += 2 * length;
						} else if (recording_position >= 2 * length) {
							recording_position -= 2 * length;
						}
					}
					break;
				}
				// Now add the influence of POSITION parameters.
			  // Offset indicated by POSITION parameter(s).
				int position_offset = trunc(length * (params[POSITION_PARAM].getValue() / 10.0));

				display_position = recording_position + position_offset;

				if (display_position >= length) {
					switch (loop_type) {
						case 0: {  // Loop around.
							display_position -= length;
						}
						break;
						case 1: {  // Bounce.
							// There might be simpler math for this, it just escapes me now.
							if (display_position < 2 * length) {
								display_position = length * 2 - display_position - 1;
							} else {
								display_position -= length * 2;
							}
						}
						break;
					}
				}

				FloatPair gotten;
				buffer->Get(&gotten, display_position);

				// TODO(clicks): Should do fade if detect we're near another record head.
				outputs[LEFT_OUTPUT].setVoltage(gotten.left);
				outputs[RIGHT_OUTPUT].setVoltage(gotten.right);
				outputs[NOW_POSITION_OUTPUT].setVoltage(display_position * 10.0 / length);

 			  // So Display knows where we are.
				line_record.position = (double) display_position;

				// This module is optimized for recording one sample to one integral position
				// in array. Later modules can figure out how to do fancier stuff.
				buffer->Set(display_position,
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
    lights[BOUNCE_LIGHT].setBrightness(loop_type == 1);
		lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
	}
};

struct NowRememberTimestamp : TimestampField {
	NowRememberTimestamp() {
  }

  Remember* module;

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

struct RememberWidget : ModuleWidget {
	RememberWidget(Remember* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Remember.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(8.024, 14.0)),
                                             module, Remember::BOUNCE_PARAM,
                                             Remember::BOUNCE_LIGHT));

		addParam(createParamCentered<RoundBlackKnob>(
			mm2px(Vec(15.24, 48.0)), module, Remember::POSITION_PARAM));

		// Record button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 63.067)),
                                             module, Remember::RECORD_BUTTON_PARAM,
                                             Remember::RECORD_BUTTON_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 63.067)), module,
		                                         Remember::RECORD_GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 82.285)),
		                                           module, Remember::NOW_POSITION_OUTPUT));
		// A timestamp is 10 wide.
		NowRememberTimestamp* now_timestamp = createWidget<NowRememberTimestamp>(mm2px(
        Vec(15.24 - (10.0 / 2.0), 87.286)));
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

		ConnectedLight* connect_light = createLightCentered<ConnectedLight>(
			mm2px(Vec(14.240, 3.0)), module, Remember::CONNECTED_LIGHT);
    connect_light->module = module;
		addChild(connect_light);
	}
};


Model* modelRemember = createModel<Remember, RememberWidget>("Remember");
