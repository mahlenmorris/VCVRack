#include "plugin.hpp"

#include "buffered.hpp"

struct Brainwash : Module {
  enum ParamId {
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
    OUTPUTS_LEN
  };
  enum LightId {
    CONNECTED_LIGHT,
    RECORD_BUTTON_LIGHT,
    LIGHTS_LEN
  };

  // We look for the nearest Memory every NN samples. This saves CPU time.
  int find_memory_countdown = 0;
  std::shared_ptr<Buffer> buffer;

  // Where we are in the movement, before taking POSITION parameters into account.
  // Always 0.0 <= playback_position < length when Looping.
  // Always 0.0 <= playback_position < 2 * length when Bouncing.
  int recording_position;

  // For detecting the Gate that controls recording.
  dsp::SchmittTrigger recordTrigger;

  Brainwash() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch(RECORD_BUTTON_PARAM, 0, 1, 0, "Press to record/release to stop this recording",
                 {"Inactive", "Recording"});
    configInput(RECORD_GATE_INPUT, "Gate to start/stop recording");
    configInput(LEFT_INPUT, "Left");
    configInput(RIGHT_INPUT, "Right");

    recording_position = -1;
  }

  void process(const ProcessArgs& args) override {
    // Only call this only every N samples, since the vast majority of
    // the time this won't change.
    // The number of modules it needs to go through does seem to increase the
    // CPU consummed by the module.
    if (--find_memory_countdown <= 0) {
      // One sixtieth of a second.
      find_memory_countdown = (int) (args.sampleRate / 60);

      buffer = findClosestMemory(getLeftExpander().module);
    }

    bool connected = (buffer != nullptr) && buffer->IsValid();

    // If connected and buffer isn't empty.
    if (connected) {
      // Are we in motion or not?
      recordTrigger.process(rescale(
          inputs[RECORD_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
      // 'recording' just reflects the state of the button and the gate input.
      bool recording = (params[RECORD_BUTTON_PARAM].getValue() > 0.1f) ||
                     recordTrigger.isHigh();


      if (recording) {  // Still recording.
        lights[RECORD_BUTTON_LIGHT].setBrightness(1.0f);
      } else {
        lights[RECORD_BUTTON_LIGHT].setBrightness(0.0f);
      }
    }
    lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
  }
};

struct BrainwashWidget : ModuleWidget {
  VCVLightSlider<WhiteLight>* adjust_slider;

  BrainwashWidget(Brainwash* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Brainwash.svg"),
                         asset::plugin(pluginInstance, "res/Brainwash-dark.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Record button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(19.05, 87.408)),
                                             module, Brainwash::RECORD_BUTTON_PARAM,
                                             Brainwash::RECORD_BUTTON_LIGHT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.35, 87.408)), module,
                                             Brainwash::RECORD_GATE_INPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.638, 110.525)), module,
                                             Brainwash::LEFT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.638, 120.844)), module,
                                             Brainwash::RIGHT_INPUT));

    /*
    // TODO: ConnectedLight assumes that we have a position, which we do not!
    ConnectedLight* connect_light = createLightCentered<ConnectedLight>(
      mm2px(Vec(12.7, 3.2)), module, Brainwash::CONNECTED_LIGHT);
    connect_light->pos_module = module;
    addChild(connect_light);
    */ 
  }

};


Model* modelBrainwash = createModel<Brainwash, BrainwashWidget>("Brainwash");
