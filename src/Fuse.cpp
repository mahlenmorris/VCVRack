#include "plugin.hpp"

struct Fuse : Module {
  enum ParamId {
    COUNT_PARAM,
    STYLE_PARAM,
		RESET_PARAM,
		TRIGGER_PARAM,
    SLEW_PARAM,
    UNTRIGGER_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RESET_INPUT,
    TRIGGER_INPUT,
    MAIN_INPUT,
    UNTRIGGER_INPUT,
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
    UNTRIGGER_LIGHT,
    LIGHTS_LEN
  };

  Fuse() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch(STYLE_PARAM, 0, 3, 0,
                 "Value of OUT as count -> LIMIT",
                 {"BLOW CLOSED (IN -> 0.0)",
                  "BLOW OPEN (0.0 -> IN)",
                  "NARROW (IN * (1 - count/LIMIT) -> 0.0)",
                  "WIDEN (IN * (count/LIMIT) -> IN)"
                 }
                );
    // Knob should snap to distinct values.
    getParamQuantity(STYLE_PARAM)->snapEnabled = true;

    configParam(COUNT_PARAM, 1, 1000, 10,
        "Number of TRIGGER events until fuse blows");
    // This is really an integer.
    getParamQuantity(COUNT_PARAM)->snapEnabled = true;

    configInput(RESET_INPUT, "Count is reset to zero when a trigger enters");
    configButton(RESET_PARAM, "Press to reset count of triggers to zero");

    configInput(TRIGGER_INPUT, "Adds one to the count each time a trigger enters");
    configButton(TRIGGER_PARAM, "Press to add one to the count");

    configInput(UNTRIGGER_INPUT, "Subtracts one from the count each time a trigger enters");
    configButton(UNTRIGGER_PARAM, "Press to subtract one from the count");

    configParam(SLEW_PARAM, 0.0f, 5.0f, 0.0f,
        "Rise/fall time for amplitude changes", " seconds");
    configOutput(BLOWN_OUTPUT, "Outputs a trigger when count hits LIMIT");

    configInput(MAIN_INPUT, "In");
    configOutput(MAIN_OUTPUT, "Out");

    // If user decides to "bypass" the module, we can just pass IN -> OUT.
    configBypass(MAIN_INPUT, MAIN_OUTPUT);

    count = 0;
  }

  // Save and retrieve menu choice(s).
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "default_in", json_real(default_in_voltage));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* default_inJ = json_object_get(rootJ, "default_in");
    if (default_inJ) {
      default_in_voltage = json_real_value(default_inJ);
    }
  }

  // Returns completion from 0.0 -> 1.0.
  float getCompletion() {
    int limit = params[COUNT_PARAM].getValue();
    int effective_count = count > limit ? limit : count;
    if (limit > 0) {
      return 1.0f * effective_count / limit;
    } else {
      return 0.0f;
    }
  }

  int getStyle() {
    return params[STYLE_PARAM].getValue();
  }

  float get_new_envelope(float current_value, float desired_value,
                         float sample_rate) {
    if (current_value  < 0.0f) {
      // Uninitialized!
      return desired_value;
    }
    if (current_value == desired_value) {
      return desired_value;
    }
    float slew = params[SLEW_PARAM].getValue();
    if (slew == 0.0f) {
      return desired_value;
    }
    // Compute how much we're allowed to change per sample over the 0-1
    // range this operates in.
    float max_move =  1.0f / (slew * sample_rate);
    if (desired_value > current_value) {
      return std::min(current_value + max_move, desired_value);
    } else {
      return std::max(current_value - max_move, desired_value);
    }
  }

  void process(const ProcessArgs& args) override {
    // Some lights are lit by triggers; these enable them to be
    // lit long enough to be seen by humans.
    if (trigger_light_countdown > 0) {
      trigger_light_countdown--;
    }
    if (untrigger_light_countdown > 0) {
      untrigger_light_countdown--;
    }
    if (reset_light_countdown > 0) {
      reset_light_countdown--;
    }

    // Test the RESET button and input.
    bool reset_was_low = !reset_trigger.isHigh();
    reset_trigger.process(rescale(
        inputs[RESET_INPUT].getVoltage(), 0.1f, 2.0f, 0.0f, 1.0f));
    if (reset_was_low && reset_trigger.isHigh()) {
      // Flash the reset light for a tenth of second.
      // Compute how many samples to show the light.
      reset_light_countdown = std::floor(args.sampleRate / 10.0f);
    }
    // Note that we don't bother to set reset_light_countdown when the user
    // presses the button; we just light up the button while it's
    // being pressed.
    bool reset = (params[RESET_PARAM].getValue() > 0.1f) ||
      (reset_was_low && reset_trigger.isHigh());
    if (reset) {
      count = 0;
    }

    // Test the TRIGGER button and input.
    bool trigger_was_low = !counter_trigger.isHigh();
    counter_trigger.process(rescale(inputs[TRIGGER_INPUT].getVoltage(),
                                   0.1f, 2.0f, 0.0f, 1.0f));
    bool trigger_from_input = trigger_was_low && counter_trigger.isHigh();

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

    // Now test the UNTRIGGER button and input.
    bool untrigger_was_low = !countdown_trigger.isHigh();
    countdown_trigger.process(rescale(inputs[UNTRIGGER_INPUT].getVoltage(),
                                   0.1f, 2.0f, 0.0f, 1.0f));
    bool untrigger_from_input = untrigger_was_low && countdown_trigger.isHigh();

    // We only want one trigger to register from a button press.
    bool untrigger_from_button = false;
    if (params[UNTRIGGER_PARAM].getValue() > 0.1f) {
      if (!untrigger_button_pressed) {
        untrigger_from_button = true;
        untrigger_button_pressed = true;
      }
    } else {
      untrigger_button_pressed = false;
    }

    int limit = params[COUNT_PARAM].getValue();

    // Determine whether or not the Fuse has blown, as certain behaviors
    // change with that fact. Note that Blown-ness can be changed by adding
    // a TRIGGER event or by lowering the LIMIT.
    bool blown = (count >= limit);
    // Process the untrigger first.
    // Note that we, for semantic reasons, don't lower the count I've we've
    // blown the fuse.
    // TODO: make this possible to allow in the menu?
    if (!blown && (untrigger_from_input || untrigger_from_button)) {
	    count = std::max(0, count - 1);
      // Flash the UNTRIGGER light for a tenth of second.
      // Compute how many samples to show the light.
      // Note that, in contrast to RESET, we do set a timer on the UNTRIGGER
      // light; because we want to convey that UNTRIGGER does ONE UNTRIGGER per
      // press, but RESET is just as reset no matter how long you hold it.
      untrigger_light_countdown = std::floor(args.sampleRate / 10.0f);
    }
    // Now process an incoming trigger.
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
      blown_generator.trigger(1e-3f);
    }
    was_blown = blown;

    float effective_count = 1.0f * (count > limit ? limit : count);

    // Here we look at the current setting s and set what the current
    // amount of amplitude modulation should be if we DON'T take slew control
    // into account.
    float desired_envelope_value = 0.0f;
    int style = getStyle();
    if (blown) {
      if (style == 1 || style == 3) {
        // styles 1 and 3 have OUT == IN when blown.
        desired_envelope_value = 1.0f;
      }
    } else {
      if (style != 1) {
        switch (style) {
          case 0:
            desired_envelope_value = 1.0f;
            break;
          case 2:
            desired_envelope_value = 1.0f - (effective_count / limit);
            break;
          case 3:
            desired_envelope_value = effective_count / limit;
            break;
        }
      }
    }
    current_envelope_value =
      get_new_envelope(current_envelope_value, desired_envelope_value,
                       args.sampleRate);
    if (current_envelope_value > 0.0f) {
      // Only read IN if we need to.
      float in_voltage = default_in_voltage;
      if (inputs[MAIN_INPUT].isConnected()) {
        in_voltage = inputs[MAIN_INPUT].getVoltage();
      }
      outputs[MAIN_OUTPUT].setVoltage(in_voltage * current_envelope_value);
    } else {
      outputs[MAIN_OUTPUT].setVoltage(0.0f);
    }

    // Set output for BLOWN.
    outputs[BLOWN_OUTPUT].setVoltage(
        blown_generator.process(args.sampleTime) ? 10.0f : 0.0f);

    // Button Lights.
    lights[RESET_LIGHT].setBrightness(
      reset || reset_light_countdown > 0 ? 1.0f: 0.0f);
    lights[TRIGGER_LIGHT].setBrightness(
      trigger_light_countdown > 0 ? 1.0f: 0.0f);
    lights[UNTRIGGER_LIGHT].setBrightness(
      untrigger_light_countdown > 0 ? 1.0f: 0.0f);
  }

  // Set by context menu.
  // Value of IN when IN is not connected.
  float default_in_voltage = 0.0f;

  // Detects input triggers.
  dsp::SchmittTrigger counter_trigger, countdown_trigger, reset_trigger;

  // Tracks the current position of the "amplitude envelope" that STYLE
  // helps determine. Needed to make SLEW control meaningful.
  // Should always be in [0.0, 1.0].
  float current_envelope_value = -1.0f;

  // Count of events observed since last reset.
  int count;

  // On previous step, were we blown?
  bool was_blown = false;
  // Makes the output trigger at BLOWN when needed.
  dsp::PulseGenerator blown_generator;

  // Make sure we only trigger once when TRIGGER button is pressed.
  bool trigger_button_pressed = false;
  bool untrigger_button_pressed = false;

  // Keeps lights on buttons lit long enough to see.
  int reset_light_countdown = 0;
  int trigger_light_countdown = 0;
  int untrigger_light_countdown = 0;
};

struct FuseDisplay : Widget {
  Fuse* module;
  std::string fontPath;
  int ticks_to_keep_text_visible = 0;

  FuseDisplay() {
    fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
  }

  void onHover(const HoverEvent & e) override {
    ticks_to_keep_text_visible = 5;  // Arbitrary, but seems to be enough.
  }

  // Given two points that define a line, find y for the given x.
  float find_y(Vec point_0, Vec point_1, float x) {
    // Need to be careful if x1 == x0.
    float x_diff = std::max(point_1.x - point_0.x, 0.00001f);
    float a = (point_1.y - point_0.y) / x_diff;
    float b = point_0.y - a * point_0.x;
    return a * x + b;
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    if ((layer == 1) && module) {
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();
      float completion = module->getCompletion();
      int style = module->getStyle();

      // Draw the completed part in green/yellow/red.
      // We track this color, in case there will be text on top of it.
      NVGcolor main_color = SCHEME_GREEN;
      if (completion > 0.0f) {
        nvgBeginPath(args.vg);
        // In the case of style 0 or 1, that's a rectangle.
        // For style 2, it's a triangle tapering off to the right.
        // To not touch the edges, we squinch in 0.5 units on all sides.
        switch (style) {
          case 0:
            nvgRect(args.vg, 0.5, 0.5,
                    bounding_box.x * completion - 1.0f, bounding_box.y - 1.0f);
            if (completion < 0.7f) {
              main_color = SCHEME_GREEN;
            } else if (completion < 1.0f) {
              main_color = SCHEME_YELLOW;
            } else {
              main_color = SCHEME_RED;
            }
            break;
          case 1:
            nvgRect(args.vg, 0.5, 0.5,
                    bounding_box.x * completion - 1.0f, bounding_box.y - 1.0f);
            if (completion < 0.7f) {
              main_color = SCHEME_RED;
            } else if (completion < 1.0f) {
              main_color = SCHEME_YELLOW;
            } else {
              main_color = SCHEME_GREEN;
            }
            break;
          case 2:
            nvgMoveTo(args.vg, 0.5, 0.5);
            nvgLineTo(args.vg, bounding_box.x - 0.5f, bounding_box.y / 2.0f);
            nvgLineTo(args.vg, 0.5, bounding_box.y - 0.5);
            nvgClosePath(args.vg);
            main_color = color::plus(
              color::mult(SCHEME_RED, completion),
              color::mult(SCHEME_GREEN, 1 - completion));
            break;
          case 3:
            nvgMoveTo(args.vg, 0.5, bounding_box.y / 2.0f);
            nvgLineTo(args.vg, bounding_box.x - 0.5f, 0.5);
            nvgLineTo(args.vg, bounding_box.x - 0.5f, bounding_box.y - 0.5);
            nvgClosePath(args.vg);
            main_color = color::plus(
              color::mult(SCHEME_GREEN, completion),
              color::mult(SCHEME_RED, 1 - completion));
            break;
        }
        nvgFillColor(args.vg, main_color);
        nvgFill(args.vg);
      }
      if (completion < 1.0f) {
        // Fill in the rest with gray.
        float cut_x = bounding_box.x * completion - 0.5f;
        nvgBeginPath(args.vg);
        switch (style) {
          case 0:
          case 1:
            // In the case of style 0 or 1, that's a rectangle.
            nvgRect(args.vg, cut_x, 0.5,
                             bounding_box.x * (1.0 - completion),
                             bounding_box.y - 1.0f);
            break;
          case 2:
            {
              Vec upper_left(0.5, 0.5),
                  right(bounding_box.x - 0.5f, bounding_box.y / 2.0f),
                  lower_left(0.5, bounding_box.y - 0.5);
              nvgMoveTo(args.vg, right.x, right.y);  // Start at the tip.
              nvgLineTo(args.vg, cut_x, find_y(upper_left, right, cut_x));
              nvgLineTo(args.vg, cut_x, find_y(lower_left, right, cut_x));
              nvgClosePath(args.vg);
              break;
            }
          case 3:
            {
              Vec left(0.5, bounding_box.y / 2.0f),
                  upper_right(bounding_box.x - 0.5f, 0.5),
                  lower_right(bounding_box.x - 0.5f, bounding_box.y - 0.5);
              nvgMoveTo(args.vg, upper_right.x, upper_right.y);
              nvgLineTo(args.vg, lower_right.x, lower_right.y);
              nvgLineTo(args.vg, cut_x, find_y(left, lower_right, cut_x));
              nvgLineTo(args.vg, cut_x, find_y(left, upper_right, cut_x));
              nvgClosePath(args.vg);
              break;
            }
        }
        nvgFillColor(args.vg, color::mult(color::WHITE, 0.5f));
        nvgFill(args.vg);
      }

      if (ticks_to_keep_text_visible > 0) {
        ticks_to_keep_text_visible--;
      }

      if (ticks_to_keep_text_visible > 0) {
        std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
        if (font) {
          // WHITE is really hard to read on YELLOW.
          nvgFillColor(args.vg,
              color::isEqual(main_color, SCHEME_YELLOW) ? color::BLACK
                                                        : color::WHITE);
          nvgFontSize(args.vg, 13);
          nvgFontFaceId(args.vg, font->handle);
          nvgTextLetterSpacing(args.vg, -2);

          std::string text = std::to_string(module->count);
          // Place on the line just off the left edge.
          nvgText(args.vg, 1, bounding_box.y / 2.0 + 4, text.c_str(), NULL);

          text = std::to_string(static_cast<int>(
            floor(completion * 100))) + "%";
          // Place on the line just off the left edge.
          float backspace = text.length() * 4.5f + 2;
          nvgText(args.vg, bounding_box.x - backspace, bounding_box.y / 2.0 + 4,
                  text.c_str(), NULL);
        }
      }
    }
    Widget::drawLayer(args, layer);
  }
};

struct FuseWidget : ModuleWidget {
  FuseWidget(Fuse* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Fuse.svg"),
                         asset::plugin(pluginInstance, "res/Fuse-dark.svg")));

    addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ThemedScrew>(
	      Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ThemedScrew>(
	      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ThemedScrew>(
	      Vec(box.size.x - 2 * RACK_GRID_WIDTH,
	          RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Screen at the top.
    FuseDisplay* display = createWidget<FuseDisplay>(
      mm2px(Vec(1.240, 17.5)));
    display->box.size = mm2px(Vec(28.0, 4.0));
    display->module = module;
    addChild(display);

    // Style.
    RoundBlackSnapKnob* style_knob = createParamCentered<RoundBlackSnapKnob>(
        mm2px(Vec(8.024, 32.0)), module, Fuse::STYLE_PARAM);
    style_knob->minAngle = -0.28f * M_PI;
    style_knob->maxAngle = 0.28f * M_PI;
    addParam(style_knob);

    // Count.
    addParam(createParamCentered<RoundBlackKnob>(
        mm2px(Vec(20.971, 32.0)), module, Fuse::COUNT_PARAM));

    // Trigger
    addInput(createInputCentered<ThemedPJ301MPort>(
	      mm2px(Vec(8.024, 48.0)), module, Fuse::TRIGGER_INPUT));
    // Making this a Button and not a Latch means that it pops back up
    // when you let go.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 48.0)),
                                             module, Fuse::TRIGGER_PARAM,
                                             Fuse::TRIGGER_LIGHT));

    // Untrigger
    addInput(createInputCentered<ThemedPJ301MPort>(
	      mm2px(Vec(8.024, 64.0)), module, Fuse::UNTRIGGER_INPUT));
    // Making this a Button and not a Latch means that it pops back up
    // when you let go.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 64.0)),
                                             module, Fuse::UNTRIGGER_PARAM,
                                             Fuse::UNTRIGGER_LIGHT));

    // Reset
    addInput(createInputCentered<ThemedPJ301MPort>(
	      mm2px(Vec(8.024, 80.0)), module, Fuse::RESET_INPUT));
    // Making this a Button and not a Latch means that it pops back up
    // when you let go.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 80.0)),
                                             module, Fuse::RESET_PARAM,
                                             Fuse::RESET_LIGHT));

    // Slew. Not always needed, so making it smaller.
    addParam(createParamCentered<RoundSmallBlackKnob>(
        mm2px(Vec(8.024, 96.0)), module, Fuse::SLEW_PARAM));

    addOutput(createOutputCentered<ThemedPJ301MPort>(
	      mm2px(Vec(20.971, 96.0)), module, Fuse::BLOWN_OUTPUT));

    addInput(createInputCentered<ThemedPJ301MPort>(
	      mm2px(Vec(8.024, 112.0)), module, Fuse::MAIN_INPUT));

    addOutput(createOutputCentered<ThemedPJ301MPort>(
	      mm2px(Vec(20.971, 112.0)), module, Fuse::MAIN_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    Fuse* module = dynamic_cast<Fuse*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Unplugged value of IN"));
    float defaults[] = {-10.0f, -5.0f, -1.0f, 1.0f, 5.0f, 10.0f};
    for (const float default_in : defaults) {
      menu->addChild(createCheckMenuItem(string::f("%gV", default_in), "",
          [=]() {return default_in == module->default_in_voltage;},
          [=]() {module->default_in_voltage = default_in;}
      ));
    }
  }
};

Model* modelFuse = createModel<Fuse, FuseWidget>("Fuse");
