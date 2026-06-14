// Distribute is a random number generator with tunable distributions, based on Mixt.

#include "plugin.hpp"

#include "controlled_random_distribution.h"
#include "distribution_graphs.h"

struct Distribute : Module {

	enum ParamId {
		UPPER_LIMIT_PARAM,
		DISTRIBUTION_PARAM,
		LOWER_LIMIT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIG_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
    OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

  Distribute() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(UPPER_LIMIT_PARAM, -10.f, 10.f, 10.f, "Upper limit for randomization");
		configParam(DISTRIBUTION_PARAM, 0.f, 4.f, 1.f, "Affects how random outputs for the squares are chosen.");
		configParam(LOWER_LIMIT_PARAM, -10.f, 10.f, -10.f, "Lower limit for randomization");
		configInput(TRIG_INPUT, "Triggeres here will cause a new random number to be sent to the output.");
		configOutput(OUT_OUTPUT, "Emits random voltages according to the distribution and limits.");
  }
  
  ~Distribute() {
  }

  // Save and retrieve menu choice(s) and data.
  // For now we have an empty version, as I've been told it's better to have
  // one in case you add to it later.
  json_t* dataToJson() override {
    json_t* rootJ = json_object();

    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
  }

  void processBypass(const ProcessArgs& args) override {
  }

  // For detecting input triggers.
  dsp::SchmittTrigger inputTrigger;

  void process(const ProcessArgs& args) override {
    // If we get a trigger, output a new number.    // Determine if we have a DRIFT event from button or input.
    bool trig_was_low = !inputTrigger.isHigh();
    inputTrigger.process(rescale(
        inputs[TRIG_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
    bool trig_from_input = trig_was_low && inputTrigger.isHigh();

    if (trig_from_input) {
      float distribution = params[DISTRIBUTION_PARAM].getValue();
      float lower_limit = params[LOWER_LIMIT_PARAM].getValue();
      float upper_limit = params[UPPER_LIMIT_PARAM].getValue();
      if (lower_limit > upper_limit) {
        // Swap the limits if they're in the wrong order.
        std::swap(lower_limit, upper_limit);
      }
      RandomDistribution dist(lower_limit, upper_limit, distribution);

      outputs[OUT_OUTPUT].setVoltage(dist.next());
    }
  }
};

// TODO: I will need to make widgets to display the limits more visibly
// (something like the current square number widget).
// and the distribution graphic.
// May interpolate the graphic when between the two values? Figure it out when I'm doing it.


// Shows the distribution currently dialed in.
// Surround this by a FramebufferWidget. Or not? Measurement indicates this
// costs about 2-4 micros per frame, or 2-4 millis every 30 seconds.
// Not worth the extra complication of the FramebufferWidget, I think.
struct DistributionWidget : Widget {
  Distribute* module;

  DistributionWidget() {}

  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      float distribution;
      if (module) {
        distribution = module->params[Distribute::DISTRIBUTION_PARAM].getValue();
      } else {
        // A Gaussian distribution, surely the canonical probability
        // distribution in people's minds.
        distribution = 1.0;
      }
       
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();

      // Want the left-side line to not be slanted, so this makes the last
      // point be directly above bounding_box.x.
      double x_per_slice = bounding_box.x / (NUM_DISTRIBUTION_BINS - 1);
      // Find specific graph to use.
      float index_exact = distribution * 10.0f;
      int index_lower = static_cast<int>(std::floor(index_exact));
      int index_upper = std::min(NUM_DISTRIBUTION_GRAPHS - 1, index_lower + 1);
      float fraction = index_exact - index_lower;

      // Make half-white.
      // TODO: consider other possibilities...
      nvgFillColor(args.vg, nvgRGBA(250, 250, 250, 255));

      nvgSave(args.vg);
      nvgScissor(args.vg, RECT_ARGS(r));  // Not sure this is right?

      // Draw the Distribution.
      // In one shape we:
      // * Start at the bottom left corner.
      // * Move to each point in the pre_calculated function.
      // * Move to the bottom right corner.
      // * Join back to the bottom left corner.
      nvgBeginPath(args.vg);
      nvgMoveTo(args.vg, 0, bounding_box.y);
      for (int i = 0; i < NUM_DISTRIBUTION_BINS; i++) {
        float h_lower = DISTRIBUTION_GRAPHS[index_lower][i];
        float h_upper = DISTRIBUTION_GRAPHS[index_upper][i];
        float height = h_lower + fraction * (h_upper - h_lower);

        float x = i * x_per_slice;
        float y = bounding_box.y * (1 - height);
        nvgLineTo(args.vg, x, y);
      }
      nvgLineTo(args.vg, bounding_box.x, bounding_box.y);
      nvgClosePath(args.vg);
      nvgFill(args.vg);
      // Restore previous state.
      nvgResetScissor(args.vg);
      nvgRestore(args.vg);
    }
	}
};



// Just the tiny window showing the number more obviously,
// as we have room and it's good to make it very obvious.
struct DistributeNumberDisplayWidget : TransparentWidget {
  // 'module' must be set by creator.
  Distribute* module;
  Distribute::ParamId my_param_id;
  float default_for_browser;

  DistributeNumberDisplayWidget() : module{nullptr} {}

  void drawLayer(const DrawArgs& args, int layer) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));
    if (layer == 1) {
      // No background color!
      float value = 0.0;
      std::shared_ptr<Font> font = APP->window->loadFont(
          asset::plugin(pluginInstance, "fonts/RobotoSlab-Regular.ttf"));
      if (module) {
        value = module->params[my_param_id].getValue();
      } else {
        value = default_for_browser;
      }
      if (font) {
        char text[10];
        snprintf(text, 10, "%1.3f", value);
        Rect r = box.zeroPos();
        Vec bounding_box = r.getBottomRight();
        nvgFillColor(args.vg, color::WHITE);
        nvgFontSize(args.vg, 20);
        nvgTextAlign(args.vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -1);
        nvgText(args.vg, bounding_box.x / 2, 0, text, NULL);
      }
    }
    TransparentWidget::drawLayer(args, layer);
    nvgResetScissor(args.vg);
  }
};

struct DistributeRandomButton : VCVLightButton<MediumSimpleLight<WhiteLight>> {
  DistributeRandomButton() {}
};

struct DistributeWidget : ModuleWidget {

  DistributeWidget(Distribute* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Distribute.svg"),
                         asset::plugin(pluginInstance, "res/Distribute-dark.svg")));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(23.967, 20.704)), module, Distribute::UPPER_LIMIT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(23.967, 32.643)), module, Distribute::LOWER_LIMIT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(23.967, 43.332)), module, Distribute::DISTRIBUTION_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.191, 120.053)), module, Distribute::TRIG_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(22.256, 119.887)), module, Distribute::OUT_OUTPUT));

    // 1st limit picker.
    DistributeNumberDisplayWidget* number_1 = createWidget<DistributeNumberDisplayWidget>(mm2px(Vec(3.0, 17.0)));
    number_1->box.size = mm2px(Vec(15.0, 6.0));
    number_1->module = module;
    number_1->my_param_id = Distribute::UPPER_LIMIT_PARAM;
    number_1->default_for_browser = -10.0;
    addChild(number_1);

    // 2nd limit picker.
    DistributeNumberDisplayWidget* number_2 = createWidget<DistributeNumberDisplayWidget>(mm2px(Vec(3.0, 29.0)));
    number_2->box.size = mm2px(Vec(15.0, 6.0));
    number_2->module = module;
    number_2->my_param_id = Distribute::LOWER_LIMIT_PARAM;
    number_2->default_for_browser = 10.0;
    addChild(number_2);
    
    // PDF display.
    DistributionWidget* dist_graph = createWidget<DistributionWidget>(mm2px(Vec(2.878, 39.237)));
    dist_graph->module = module;
    dist_graph->box.size = mm2px(Vec(15.0, 8.0));
    addChild(dist_graph);
  }
};

Model* modelDistribute = createModel<Distribute, DistributeWidget>("Distribute");
