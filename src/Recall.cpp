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
		INPUTS_LEN
	};
	enum OutputId {
		LEFT_OUT_OUTPUT,
		RIGHT_OUT_OUTPUT,
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

	Recall() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(LOOP_PARAM, 0, 2, 0, "What to do when hitting the endpoints",
								 {"Loop around", "Bounce", "Go back to Position"});
		// This has distinct values.
    getParamQuantity(LOOP_PARAM)->snapEnabled = true;
	  configParam(SPEED_PARAM, -10.f, 10.f, 1.f, "Playback speed/direction");
		configParam(POSITION_PARAM, 0.f, 10.f, 0.f, "Starting position");
		configSwitch(PLAY_BUTTON_PARAM, 0, 1, 0, "Press to start/stop this play head",
	               {"Silent", "Playing"});
		configInput(PLAY_GATE_INPUT, "Gate to start/stop playing");
		configOutput(LEFT_OUT_OUTPUT, "");
		configOutput(RIGHT_OUT_OUTPUT, "");

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
			playTrigger.process(rescale(
					inputs[PLAY_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
			bool playing = (params[PLAY_BUTTON_PARAM].getValue() > 0.1f) ||
			               playTrigger.isHigh();
			if (playing) {
				int length = buffer->length;
				float* array = buffer->array;
				int loop_type = (int) params[LOOP_PARAM].getValue();
				if (loop_type != 1) {
					invertSpeed = false;
				}
				if (playback_position == -1) { // Starting.
					playback_position = length * (params[POSITION_PARAM].getValue() / 10.0);
				}
				playback_position += params[SPEED_PARAM].getValue() * (invertSpeed ? -1 : 1);
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
							playback_position += params[SPEED_PARAM].getValue() * (invertSpeed ? -1 : 1);
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
				int playback_start = trunc(playback_position);
				float start_fraction = playback_position - playback_start;
				float value = array[playback_start] * (1.0 - start_fraction) +
				  array[playback_start + 1] * (start_fraction);
				outputs[LEFT_OUT_OUTPUT].setVoltage(value);
				lights[PLAY_BUTTON_LIGHT].setBrightness(1.0f);
			} else {
				outputs[LEFT_OUT_OUTPUT].setVoltage(0.0f);
				lights[PLAY_BUTTON_LIGHT].setBrightness(0.0f);
			}

		} else {
			// Can only be playing if connected.
			lights[PLAY_BUTTON_LIGHT].setBrightness(0.0f);
		}

		lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
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
        mm2px(Vec(8.024, 32.0)), module, Recall::LOOP_PARAM);
    loop_knob->minAngle = -0.28f * M_PI;
    loop_knob->maxAngle = 0.28f * M_PI;
    loop_knob->snap = true;
    addParam(loop_knob);
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.971, 32.0)), module, Recall::SPEED_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 48.0)), module, Recall::POSITION_PARAM));

		// Play button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 80.0)),
                                             module, Recall::PLAY_BUTTON_PARAM,
                                             Recall::PLAY_BUTTON_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 80.0)), module, Recall::PLAY_GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.024, 112.0)), module, Recall::LEFT_OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.971, 112.0)), module, Recall::RIGHT_OUT_OUTPUT));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(3.394, 7.56)), module, Recall::CONNECTED_LIGHT));
	}
};

Model* modelRecall = createModel<Recall, RecallWidget>("Recall");
