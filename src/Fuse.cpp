#include "plugin.hpp"

struct Fuse : Module {
  enum ParamId {
    COUNT_PARAM,
    ACTION_PARAM,
		RESET_PARAM,
		TRIGGER_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RESET_INPUT,
    TRIGGER_INPUT,
    MAIN_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    BLOWN_OUTPUT,
    MAIN_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    RESET_LIGHT,
    TRIGGER_LIGHT,
    ENUMS(COUNT_METER_LIGHT, 10),
    LIGHTS_LEN
  };

  Fuse() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch(ACTION_PARAM, 0, 3, 0,
                 "What happens before/after count == LIMIT",
                 {"OUT == IN/OUT is zero",
                  "OUT is zero/OUT == IN",
                  "OUT = IN * (1 - count/LIMIT)",
                  "OUT = IN * (count/LIMIT)"
                 }
                );
    // Knob should snap to distinct values.
    getParamQuantity(ACTION_PARAM)->snapEnabled = true;

    configParam(COUNT_PARAM, 1, 1000, 10,
        "Number of TRIGGER events until fuse blows");
    // This is really an integer.
    getParamQuantity(COUNT_PARAM)->snapEnabled = true;

    configInput(RESET_INPUT, "Count is reset to zero when a trigger enters");
    configButton(RESET_PARAM, "Press to reset counted triggers to zero");

    configInput(TRIGGER_INPUT, "Adds one to the count each time a trigger enters");
    configButton(TRIGGER_PARAM, "Press to add one to the count");

    configOutput(BLOWN_OUTPUT, "Outputs a trigger when count hits LIMIT");

    configInput(MAIN_INPUT, "In");
    configOutput(MAIN_OUTPUT, "Out");

    // If user decides to "bypass" the module, we can just pass IN -> OUT.
    configBypass(MAIN_INPUT, MAIN_OUTPUT);

    count = 0;
  }

  void process(const ProcessArgs& args) override {
    step++;
    // Some lights are lit by triggers; these enable them to be
    // lit long enough to be seen by humans.
    if (trigger_light_countdown > 0) {
      trigger_light_countdown--;
    }
    if (reset_light_countdown > 0) {
      reset_light_countdown--;
    }

    // Test the RESET button and input.
    bool reset_was_low = !resetTrigger.isHigh();
    resetTrigger.process(rescale(
        inputs[RESET_INPUT].getVoltage(), 0.1f, 2.0f, 0.0f, 1.0f));
    if (reset_was_low && resetTrigger.isHigh()) {
      // Flash the reset light for a tenth of second.
      // Compute how many samples to show the light.
      reset_light_countdown = std::floor(args.sampleRate / 10.0f);
    }
    // Note that we don't bother to set reset_light_countdown when the user
    // presses the button; we just light up the button while it's
    // being pressed.
    bool reset = (params[RESET_PARAM].getValue() > 0.1f) ||
      (reset_was_low && resetTrigger.isHigh());
    if (reset) {
      count = 0;
    }

    // Test the TRIGGER button and input.
    // TODO: should TRIGGER in UI go back to COUNTER?
    bool trigger_was_low = !counterTrigger.isHigh();
    counterTrigger.process(rescale(inputs[TRIGGER_INPUT].getVoltage(),
                                   0.1f, 2.0f, 0.0f, 1.0f));
    bool trigger_from_input = trigger_was_low && counterTrigger.isHigh();

    // We only want one trigger from a button press.
    bool trigger_from_button = false;
    if (params[TRIGGER_PARAM].getValue() > 0.1f) {
      if (!trigger_button_pressed) {
        trigger_from_button = true;
        trigger_button_pressed = true;
      }
    } else {
      trigger_button_pressed = false;
    }

    int limit = params[COUNT_PARAM].getValue();

    // Determine whether or not the Fuse has blown, as certain behaviors
    // change with that fact. Note that Blown-ness can be changed by adding
    // a TRIGGER event or by lowering the LIMIT.
    bool blown = (count >= limit);
    if (!blown && (trigger_from_input || trigger_from_button)) {
	    count += 1;
      // Flash the TRIGGER light for a tenth of second.
      // Compute how many samples to show the light.
      // Note that, in contrast to RESET, we do set a timer on the TRIGGER
      // light; because we want to convey that TRIGGER does ONE TRIGGER per
      // press, but RESET is just as reset no matter how long you hold it.
      trigger_light_countdown = std::floor(args.sampleRate / 10.0f);
	    if (count >= limit) {
	      blown = true;
	    }
    }
    if (!was_blown && blown) {
      blownGenerator.trigger(1e-3f);
    }
    was_blown = blown;

    int action = params[ACTION_PARAM].getValue();
    int effective_count = count > limit ? limit : count;
    float out_voltage = 0.0f;

    if (blown) {
      if (action != 0 && action != 2) {
        // Actions 0, 2 don't require us to have IN value.
        out_voltage = inputs[MAIN_INPUT].getVoltage();
      }
    } else {
      if (action != 1) {
        float in_voltage = inputs[MAIN_INPUT].getVoltage();
        switch (action) {
          case 0:
            out_voltage = in_voltage;
            break;
          case 2:
            out_voltage = in_voltage * (1.0f -
              (1.0f * effective_count / limit));
            break;
          case 3:
            out_voltage = in_voltage * (1.0f * effective_count / limit);
            break;
        }
      }
    }
    outputs[MAIN_OUTPUT].setVoltage(out_voltage);

    // Set output for BLOWN.
    outputs[BLOWN_OUTPUT].setVoltage(
        blownGenerator.process(args.sampleTime) ? 10.0f : 0.0f);

    // Meter Lights
    // If it's been a (short) while, update the meter lights, but only
    // if the number of lights has changed.
    if (step == 512) {  // TODO: This should be a less arbitrary number.
      step = 0;
      int lit = floor((count * 1.0f) / params[COUNT_PARAM].getValue() * 10);
      if (prev_lit != lit) {
	      for (int v = 0; v < 10; v++) {
	        // Need to set _all_ lamps, since they may have been cleared.
	        lights[COUNT_METER_LIGHT + v].setBrightness(v + 1 <= lit ? 1.0f : 0.0f);
	      }
	      prev_lit = lit;
      }
    }

    // Button Lights.
    lights[RESET_LIGHT].setBrightness(
      reset || reset_light_countdown > 0 ? 1.0f: 0.0f);
    lights[TRIGGER_LIGHT].setBrightness(
      trigger_light_countdown > 0 ? 1.0f: 0.0f);
  }

  dsp::SchmittTrigger counterTrigger, resetTrigger;

  // On previous step, were we blown?
  bool was_blown = false;

  // Count of events observed since last reset.
  int count;

  // Previous number of lit lights in meter.
  int prev_lit = -1;
  unsigned short int step = 0;
  dsp::PulseGenerator blownGenerator;

  // Make sure we only trigger once when TRIGGER button is pressed.
  bool trigger_button_pressed = false;

  // Keeps lights lit long enough to see.
  int reset_light_countdown = 0;
  int trigger_light_countdown = 0;
};


struct FuseWidget : ModuleWidget {
  FuseWidget(Fuse* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Fuse.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(
	      Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(
	      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(
	      Vec(box.size.x - 2 * RACK_GRID_WIDTH,
	          RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Action as count increase.
    RoundBlackSnapKnob* action_knob = createParamCentered<RoundBlackSnapKnob>(
        mm2px(Vec(8.024, 48.0)), module, Fuse::ACTION_PARAM);
    action_knob->minAngle = -0.28f * M_PI;
    action_knob->maxAngle = 0.28f * M_PI;
    addParam(action_knob);

    // Reset
    addInput(createInputCentered<PJ301MPort>(
	      mm2px(Vec(8.024, 64.0)), module, Fuse::RESET_INPUT));
        // Making this a Button and not a Latch means that it pops back up
        // when you let go.
        addParam(createLightParamCentered<VCVLightButton<
                 MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 64.0)),
                                                 module, Fuse::RESET_PARAM,
                                                 Fuse::RESET_LIGHT));

    // Trigger
    addInput(createInputCentered<PJ301MPort>(
	      mm2px(Vec(8.024, 80.0)), module, Fuse::TRIGGER_INPUT));
        // Making this a Button and not a Latch means that it pops back up
        // when you let go.
        addParam(createLightParamCentered<VCVLightButton<
                 MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 80.0)),
                                                 module, Fuse::TRIGGER_PARAM,
                                                 Fuse::TRIGGER_LIGHT));

    addParam(createParamCentered<RoundBlackKnob>(
        mm2px(Vec(8.024, 96.0)), module, Fuse::COUNT_PARAM));

    addOutput(createOutputCentered<PJ301MPort>(
	      mm2px(Vec(20.971, 96.0)), module, Fuse::BLOWN_OUTPUT));

    addInput(createInputCentered<PJ301MPort>(
	      mm2px(Vec(8.024, 112.0)), module, Fuse::MAIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(
	      mm2px(Vec(20.971, 112.0)), module, Fuse::MAIN_OUTPUT));

    // Adds meter lights.
    float light_x_mm = 20.0f;
    float light_y_mm = 50.0f;
    float light_y_spacing_mm = 4.0f;

    for (int i = 0; i < 6; i++) {
      addChild(createLight<MediumLight<GreenLight>>(
          mm2px(Vec(light_x_mm, light_y_mm - light_y_spacing_mm * i)),
	             module, Fuse::COUNT_METER_LIGHT + i));
    }
    for (int i = 6; i < 9; i++) {
      addChild(createLight<MediumLight<YellowLight>>(
          mm2px(Vec(light_x_mm, light_y_mm - light_y_spacing_mm * i)),
	             module, Fuse::COUNT_METER_LIGHT + i));
    }
    for (int i = 9; i < 10; i++) {
      addChild(createLight<MediumLight<RedLight>>(
          mm2px(Vec(light_x_mm, light_y_mm - light_y_spacing_mm * i)),
	             module, Fuse::COUNT_METER_LIGHT + i));
    }
  }
};

Model* modelFuse = createModel<Fuse, FuseWidget>("Fuse");
