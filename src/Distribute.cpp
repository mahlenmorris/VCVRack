// Distribute is a random number generator with tunable distributions, based on Mixt.

#include "plugin.hpp"

#include "controlled_random_distribution.h"
#include "distribution_graphs.h"

struct Distribute : Module {

	enum ParamId {
		UPPER_LIMIT_PARAM,
		DISTRIBUTION_PARAM,
		LOWER_LIMIT_PARAM,
		BIAS_PARAM,
    SECTION_PARAM,
		CONTINUOUS_PARAM,
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
    CONTINUOUS_LIGHT,
		LIGHTS_LEN
	};

  Distribute() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(UPPER_LIMIT_PARAM, -10.f, 10.f, 10.f, "Upper limit for randomization");
		configParam(DISTRIBUTION_PARAM, 0.f, 4.f, 1.f, "Affects how random outputs for the squares are chosen.");
		configParam(LOWER_LIMIT_PARAM, -10.f, 10.f, -10.f, "Lower limit for randomization");
		configParam(BIAS_PARAM, -1.0f, 1.0f, 0.0f, "(Both only) Biases the random outputs towards the lower or upper limit.");
    // A 3-position toggle switch (values: 0, 1, 2)
    configSwitch(SECTION_PARAM, 0.0f, 2.0f, 1.0f, "Section", {"Left", "Both", "Right"});
    // A latched button.
    configSwitch(CONTINUOUS_PARAM, 0, 1, 0, "Ignore trigger and continuously outputs random numbers.",
                 {"Off", "On"});

 		configInput(TRIG_INPUT, "Triggers here will cause a new random number to be sent to the output.");
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
    bool continuous = params[CONTINUOUS_PARAM].getValue() > 0.5f;

    if (trig_from_input || continuous) {
      float distribution = params[DISTRIBUTION_PARAM].getValue();
      float lower_limit = params[LOWER_LIMIT_PARAM].getValue();
      float upper_limit = params[UPPER_LIMIT_PARAM].getValue();
      if (lower_limit > upper_limit) {
        // Swap the limits if they're in the wrong order.
        std::swap(lower_limit, upper_limit);
      }
      float bias = params[BIAS_PARAM].getValue();
      RandomDistribution::PDFSection pdf_section = GetSection();
      RandomDistribution dist(lower_limit, upper_limit, distribution,
          pdf_section == RandomDistribution::BOTH ? bias: 0.0f, pdf_section);

      outputs[OUT_OUTPUT].setVoltage(dist.next());
    }

    // Lights.
    lights[CONTINUOUS_LIGHT].setBrightness(continuous ? 1.f : 0.f);
  }

RandomDistribution::PDFSection GetSection() { 
  int section = params[SECTION_PARAM].getValue();
  switch (section) {
    case 0:
      return RandomDistribution::LEFT;
    case 1:
      return RandomDistribution::BOTH;
    case 2:
      return RandomDistribution::RIGHT;
    default:
      // This should never happen, but just in case, we'll default to BOTH.
      return RandomDistribution::BOTH;
  }
}

};

// Shows the distribution currently dialed in.
// Surround this by a FramebufferWidget. Or not? Measurement indicates this
// costs about 2-4 micros per frame, or 2-4 millis every 30 seconds.
// Not worth the extra complication of the FramebufferWidget, I think.
// TODO: remeasure this draw time, after adding bias and section.
struct DistributionWidget : Widget {
  Distribute* module;

  DistributionWidget() {}

  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      float distribution;
      float bias;
      RandomDistribution::PDFSection pdf_section;
      if (module) {
        distribution = module->params[Distribute::DISTRIBUTION_PARAM].getValue();
        pdf_section = module->GetSection();
        // bias is only useful when we're at BOTH.
        bias = pdf_section == RandomDistribution::BOTH ? module->params[Distribute::BIAS_PARAM].getValue() : 0.0f;
      } else {
        // A Gaussian distribution, surely the canonical probability
        // distribution in people's minds.
        distribution = 1.0;
        bias = 0.0f;
        pdf_section = RandomDistribution::BOTH;
      }
       
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();

      // Displaying sections other than BOTH changes the range of bins we look at.
      int first_bin = 0;
      int last_bin = NUM_DISTRIBUTION_BINS - 1;
      if (pdf_section == RandomDistribution::LEFT) {
        last_bin = floor(NUM_DISTRIBUTION_BINS / 2);  // i.e., 30. We use 31 bins.
      } else if (pdf_section == RandomDistribution::RIGHT) {
        first_bin = floor(NUM_DISTRIBUTION_BINS / 2); // i.e., 30. We use 31 bins.
      }

      // Want the left-side line to not be slanted, so this makes the last
      // point be directly above bounding_box.x.
      double x_per_slice = bounding_box.x / (last_bin - first_bin);
      // Find specific graph to use.
      float index_exact = distribution * 10.0f;
      int index_lower = static_cast<int>(std::floor(index_exact));
      int index_upper = std::min(NUM_DISTRIBUTION_GRAPHS - 1, index_lower + 1);
      float fraction = index_exact - index_lower;

      // Find specific bias graph to use.
      float bias_index_exact = (bias + 1.0f) * 10.0f;
      int bias_index_lower = static_cast<int>(std::floor(bias_index_exact));
      int bias_index_upper = std::min(NUM_BIAS_GRAPHS - 1, bias_index_lower + 1);
      float bias_fraction = bias_index_exact - bias_index_lower;

      // Make pretty bright white.
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
      for (int i = first_bin; i <= last_bin; i++) {
        float h_00 = DISTRIBUTION_GRAPHS[index_lower][bias_index_lower][i];
        float h_10 = DISTRIBUTION_GRAPHS[index_upper][bias_index_lower][i];
        float h_01 = DISTRIBUTION_GRAPHS[index_lower][bias_index_upper][i];
        float h_11 = DISTRIBUTION_GRAPHS[index_upper][bias_index_upper][i];

        // Bilinear interpolation
        float h_0 = h_00 + fraction * (h_10 - h_00);
        float h_1 = h_01 + fraction * (h_11 - h_01);
        float height = h_0 + bias_fraction * (h_1 - h_0);

        float x = (i - first_bin) * x_per_slice;
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
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.513, 60.678)), module, Distribute::DISTRIBUTION_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(24.967, 60.678)), module, Distribute::BIAS_PARAM));
    addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(15.24, 56.678)), module, Distribute::SECTION_PARAM));
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(7.191, 104.0)),
                                             module, Distribute::CONTINUOUS_PARAM,
                                             Distribute::CONTINUOUS_LIGHT));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.191, 118.0)), module, Distribute::TRIG_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(22.256, 111.0)), module, Distribute::OUT_OUTPUT));

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
    DistributionWidget* dist_graph = createWidget<DistributionWidget>(mm2px(Vec(3.0, 39.3)));
    dist_graph->module = module;
    dist_graph->box.size = mm2px(Vec(24.0, 13.0));
    addChild(dist_graph);
  }
};

Model* modelDistribute = createModel<Distribute, DistributeWidget>("Distribute");
