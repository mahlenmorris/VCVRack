#include <algorithm>
#include <vector>

#include "plugin.hpp"

struct Chances : Module {
  enum ParamId {
    CONTINUOUS_BUTTON_PARAM,
    STYLE_PARAM,

    ENUMS(VALUE_PARAM, 12),
    ENUMS(COUNT_PARAM, 12),
    PARAMS_LEN
  };
  enum InputId { TRIG_INPUT, INPUTS_LEN };
  enum OutputId { OUT_OUTPUT, OUTPUTS_LEN };
  enum LightId { CONTINUOUS_BUTTON_LIGHT, LIGHTS_LEN };

  // For detecting output triggers.
  dsp::SchmittTrigger inputTrigger;
  float prev_values[12];
  int prev_counts[12];
  // A vector of possibilities. While doing this prohibits non-integral
  // COUNT_PARAM values, it the data structure we want anyway for Shuffling,
  // and makes for a simple Sampling as well.
  std::vector<float> samples;

  Chances() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    // TODO: need to add a Trigger detector that means to reshuffle.
    configSwitch(STYLE_PARAM, 0, 1, 0, "Value Choosen by",
                 {"Sampling", "Shuffling"});
    // This has distinct values.
    getParamQuantity(STYLE_PARAM)->snapEnabled = true;

    for (int i = 0; i < 12; ++i) {
      configParam(VALUE_PARAM + i, -10.f, 10.f, 0.f, "Value to possibly emit");
      configParam(COUNT_PARAM + i, 0, 100, 0,
                  "Count of relative chance that this value will be chosen");
      getParamQuantity(COUNT_PARAM + i)->snapEnabled = true;
    }

    // A latched button.
    configSwitch(CONTINUOUS_BUTTON_PARAM, 0, 1, 0,
                 "Ignore trigger and continuously output values.",
                 {"Off", "On"});

    configInput(TRIG_INPUT, "Triggers here will cause a new random number to "
                            "be sent to the output.");
    configOutput(OUT_OUTPUT,
                 "Emits values according to the relative chances set above.");
    // Init with impossible values, to guarantee a refresh at the start.
    for (int pos = 0; pos < 12; ++pos) {
      prev_values[pos] = -12.0;
      prev_counts[pos] = -1;
    }
  }

  // If asked to, save the curve data in json for reading when loaded.
  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {}

  void process(const ProcessArgs &args) override {
    // Determine if we need to recompute the probability field.
    // If knobs have changed, then yes!
    bool need_update = false;
    for (int pos = 0; pos < 12; ++pos) {
      if ((prev_values[pos] != params[VALUE_PARAM + pos].getValue()) ||
          (prev_counts[pos] != params[COUNT_PARAM + pos].getValue())) {
        need_update = true;
        break;
      }
    }
    if (need_update) {
      // Doing this the simple way, may optimize later.
      samples.clear();
      for (int pos = 0; pos < 12; ++pos) {
        float value = params[VALUE_PARAM + pos].getValue();
        int count = params[COUNT_PARAM + pos].getValue();
        prev_values[pos] = value;
        prev_counts[pos] = count;
        if (count > 0) {
          for (int i = 0; i < count; ++i) {
            samples.push_back(value);
          }
        }
      }
      // TODO: if shuffling, arrange for that? Or not until shuffle is
      // triggered? What if this is first entry?? Think more on what user
      // expects.
      // TODO: this should dirty the Framebuffer?
    }
    // Time to output new value?
    bool trig_was_low = !inputTrigger.isHigh();
    inputTrigger.process(
        rescale(inputs[TRIG_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
    bool trig_from_input = trig_was_low && inputTrigger.isHigh();

    bool continuous = params[CONTINUOUS_BUTTON_PARAM].getValue() > 0.5f;
    if (trig_from_input || continuous) {
      // TODO: see if shuffling.
      if (samples.size() > 0) {
        size_t position =
            (size_t)floor(rack::random::uniform() * samples.size());
        assert(position < samples.size());
        outputs[OUT_OUTPUT].setVoltage(samples.at(position));
      } else {
        outputs[OUT_OUTPUT].setVoltage(0.0f);
      }
    }

    // Lights.
    lights[CONTINUOUS_BUTTON_LIGHT].setBrightness(continuous ? 1.f : 0.f);
  }
};

struct ChancesDisplay : Widget {
  Chances *module;
  std::string fontPath;

  ChancesDisplay() {
    fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
  }

  // We'll want a Framebuffer for this at some point?
  // Which will require this to be draw().
  void drawLayer(const DrawArgs &args, int layer) override {
    if (layer == 1) {

      if (module) {
        // Get values from actual module.
      } else {
        // Default values to show in module browser and library.
      }

      Rect r = box.zeroPos(); // .shrink(Vec(4, 5));  // TODO: ???
      Vec bounding_box = r.getBottomRight();
      /*
      Vec p0 =
          transform((module && module->initialized) ? module->display_points[0]
                                                    : demo_display_points[0],
                    bounding_box);

      // Draw middle line.
      nvgBeginPath(args.vg);
      nvgMoveTo(args.vg, 0.0, bounding_box.y / 2.0);
      nvgLineTo(args.vg, bounding_box.x, bounding_box.y / 2.0);
      nvgLineCap(args.vg, NVG_ROUND);
      nvgMiterLimit(args.vg, 2.f);
      nvgStrokeWidth(args.vg, 1.5f);
      nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
      nvgStroke(args.vg);

      // Draw voltage numbers, to make the IN and OUT range more obvious.
      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (font) {
        nvgFontSize(args.vg, 13);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -2);

        std::string text = unipolar ? "5" : "0";
        // Place on the line just off the left edge.
        nvgText(args.vg, 1, bounding_box.y / 2.0 + 4, text.c_str(), NULL);

        text = unipolar ? "0" : "-5";
        // Place a little above the bottom just off the left edge.
        nvgText(args.vg, 1, bounding_box.y - 5, text.c_str(), NULL);

        text = unipolar ? "10" : "5";
        // Place a little above the bottom just off the right edge.
        nvgText(args.vg, bounding_box.x - 12, bounding_box.y - 5, text.c_str(),
                NULL);
        // Place a little below the top just off the left edge.
        nvgText(args.vg, 1, 12, text.c_str(), NULL);
      }

      // The graph of the output function.
      nvgBeginPath(args.vg);
      nvgMoveTo(args.vg, p0.x, p0.y);
      for (int i = 1; i < DISPLAY_POINT_COUNT; i++) {
        Vec next = transform((module && module->initialized)
                                 ? module->display_points[i]
                                 : demo_display_points[i],
                             bounding_box);
        nvgLineTo(args.vg, next.x, next.y);
      }
      nvgLineCap(args.vg, NVG_ROUND);
      nvgMiterLimit(args.vg, 2.f);
      nvgStrokeWidth(args.vg, 1.5f);
      nvgStrokeColor(args.vg, outputColor);
      nvgStroke(args.vg);

      // And a short vertical line indicating the position of IN.
      if (input_connected) {
        float x = domain_value * bounding_box.x * 0.1f;
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, x, bounding_box.y);
        nvgLineTo(args.vg, x, bounding_box.y * 0.8);
        nvgLineCap(args.vg, NVG_ROUND);
        nvgMiterLimit(args.vg, 2.f);
        nvgStrokeWidth(args.vg, 1.5f);
        nvgStrokeColor(args.vg, inputColor);
        nvgStroke(args.vg);
      }
    }
    */
    }
  }
};

struct ChancesWidget : ModuleWidget {
  static constexpr float X_DIFF_MM = 11.0;

  ChancesWidget(Chances *module) {
    setModule(module);
    setPanel(
        // TODO: add dark version.
        createPanel(asset::plugin(pluginInstance, "res/Chances.svg")));

    ChancesDisplay *display =
        createWidget<ChancesDisplay>(mm2px(Vec(1.0, 10.0)));
    display->box.size = mm2px(Vec(72.0, 30.0));
    display->module = module;
    addChild(display);

    for (int pos = 0; pos < 5; ++pos) {
      addParam(createParamCentered<RoundBlackKnob>(
          mm2px(Vec(22.0 + pos * X_DIFF_MM, 46.0)), module,
          Chances::COUNT_PARAM + pos));
      addParam(createParamCentered<RoundBlackKnob>(
          mm2px(Vec(22.0 + pos * X_DIFF_MM, 56.0)), module,
          Chances::VALUE_PARAM + pos));
    }
    addParam(
        createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(
            mm2px(Vec(21.822, 116.0)), module, Chances::CONTINUOUS_BUTTON_PARAM,
            Chances::CONTINUOUS_BUTTON_LIGHT));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(35.487, 116.0)),
                                                 module, Chances::STYLE_PARAM));

    addInput(createInputCentered<ThemedPJ301MPort>(
        mm2px(Vec(8.032, 116.0)), module, Chances::TRIG_INPUT));

    addOutput(createOutputCentered<ThemedPJ301MPort>(
        mm2px(Vec(68.819, 116.0)), module, Chances::OUT_OUTPUT));
  }

  void appendContextMenu(Menu *menu) override {
    Chances *module = dynamic_cast<Chances *>(this->module);
  }
};

Model *modelChances = createModel<Chances, ChancesWidget>("Chances");
