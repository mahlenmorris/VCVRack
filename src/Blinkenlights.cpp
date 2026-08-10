// Blinkenlights is a visual module solely devoted to filling the patch with
// pleasing visual fluff. Inspired by the lighting props on film sets. The name
// is from cards posted in labs such as described here:
// https://en.wikipedia.org/wiki/Blinkenlights

#include "plugin.hpp"

static constexpr int MAX_LIGHTS_PER_COLUMN = 20;
static constexpr int LIGHT_COLUMNS = 4;

struct LightPattern {
  virtual void process(bool new_frame, std::vector<Light>* lights,
                       int index_start, const Module::ProcessArgs& args) = 0;
};

// As currently written, I don't much like this one.
/*
struct RandomPattern : LightPattern {
  static constexpr int MAX_CYCLES = 5;
  std::vector<int> lights_in_cycle[MAX_CYCLES];
  int previous_cycle = MAX_CYCLES - 1;
  int current_cycle = 0;

  RandomPattern() {
    // Pick which lights are lit in each of the five cycles.
    std::vector<int> light_index;
    for (int i = 0; i < MAX_LIGHTS_PER_COLUMN * LIGHT_COLUMNS; ++i) {
      light_index.push_back(i);
    }

    // Prepare a source of randomness that shuffle will like.
    // Seed with high-entropy source.
    std::random_device rd;

    // Initialize Mersenne Twister generator with seed.
    std::mt19937 random_source;
    random_source.seed(rd());
    std::shuffle(light_index.begin(), light_index.end(), random_source);

    // Now divide them among the cycles.
    int cycle = 0;
    for (int i = 0; i < MAX_LIGHTS_PER_COLUMN * LIGHT_COLUMNS; ++i) {
      lights_in_cycle[cycle].push_back(light_index.back());
      light_index.pop_back();
      cycle = (cycle + 1) % MAX_CYCLES;
    }
  }

  void process(bool new_frame, std::vector<Light>* lights, int index_start,
               const Module::ProcessArgs& process_args) override {
    if (new_frame) {
      previous_cycle = current_cycle;
      current_cycle = (current_cycle + 1) % MAX_CYCLES;

      // Turn on all the new lights.
      for (int index : lights_in_cycle[current_cycle]) {
        lights->at(index_start + index).setBrightness(1.f);
      }
      // Turn off all the on lights.
      // Have to do it over multiple process() calls to get to 0.0.
      for (int index : lights_in_cycle[previous_cycle]) {
        lights->at(index_start + index).setBrightness(0.f);
      }
    }
  }
};
*/

struct RandomBlockPattern : LightPattern {
  static constexpr int MAX_CYCLES = 5;
  std::vector<int> lights_in_cycle[MAX_CYCLES];
  int previous_cycle = MAX_CYCLES - 1;
  int current_cycle = 0;

  RandomBlockPattern() {
    // MAX_CYCLES blocks of lights.
    // Divide them among the cycles.
    std::vector<int> blocks;
    for (int i = 0; i < MAX_CYCLES; ++i) {
      blocks.push_back(i);
    }

    // Prepare a source of randomness that shuffle will like.
    // Seed with high-entropy source.
    std::random_device rd;

    // Initialize Mersenne Twister generator with seed.
    std::mt19937 random_source;
    random_source.seed(rd());
    std::shuffle(blocks.begin(), blocks.end(), random_source);

    for (int cycle = 0; cycle < MAX_CYCLES; ++cycle) {
      int block = blocks[cycle];
      for (int column = 0; column < LIGHT_COLUMNS; ++column) {
        for (int row = block * 4; row < (block + 1) * 4; ++row) {
          lights_in_cycle[cycle].push_back(column * MAX_LIGHTS_PER_COLUMN +
                                           row);

          WARN("cycle %d gets light %d", cycle,
               column * MAX_LIGHTS_PER_COLUMN + row);
        }
      }
    }
  }

  void process(bool new_frame, std::vector<Light>* lights, int index_start,
               const Module::ProcessArgs& process_args) override {
    if (new_frame) {
      previous_cycle = current_cycle;
      current_cycle = (current_cycle + 1) % MAX_CYCLES;

      // Turn on all the new lights.
      for (int index : lights_in_cycle[current_cycle]) {
        lights->at(index_start + index).setBrightness(1.f);
      }
      // Turn off all the on lights.
      // Have to do it over multiple process() calls to get to 0.0.
      for (int index : lights_in_cycle[previous_cycle]) {
        lights->at(index_start + index).setBrightness(0.f);
      }
    }
  }
};

struct Blinkenlights : Module {
  enum ParamId { PARAMS_LEN };
  enum InputId { INPUTS_LEN };
  enum OutputId { OUTPUTS_LEN };
  enum LightId {
    ENUMS(COLUMN0_LIGHT, MAX_LIGHTS_PER_COLUMN* LIGHT_COLUMNS),
    LIGHTS_LEN
  };

  // RandomPattern random_pattern;
  RandomBlockPattern random_block_pattern;

  int format = 0;

  Blinkenlights() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    // We randomly pick a style on startup. Could get overriden by json
    // contents.
    //    format = (int)floor(random::uniform() * 2);
  }

  ~Blinkenlights() {}

  // Save and retrieve menu choice(s) and data.
  // For now we have an empty version, as I've been told it's better to have
  // one in case you add to it later.
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {}

  void processBypass(const ProcessArgs& args) override {}

  void process(const ProcessArgs& args) override {
    // I'd like all of the Blinkenlights to (for now) be on the same schedule.
    // By default, for now, I'm going to try and change the display every half
    // second. Not sure this would work if the sample rate changes, to be
    // honest.
    switch (format) {
      case 0:
      default: {
        random_block_pattern.process(
            (args.frame % (int)floor(args.sampleRate / 2) == 0), &lights,
            COLUMN0_LIGHT, args);
        break;
      }
    }
  }
};

struct BlinkenlightsWidget : ModuleWidget {
  BlinkenlightsWidget(Blinkenlights* module) {
    setModule(module);
    setPanel(
        createPanel(asset::plugin(pluginInstance, "res/Blinkenlights.svg")));

    int column = 0;
    for (int i = 0; i < MAX_LIGHTS_PER_COLUMN * LIGHT_COLUMNS;
         i = i + MAX_LIGHTS_PER_COLUMN) {
      for (int j = 0; j < MAX_LIGHTS_PER_COLUMN; ++j) {
        addChild(createLightCentered<LargeLight<GreenLight>>(
            mm2px(Vec(6.0 + column * 6.0, 7.0 + (j * 6.0))), module,
            Blinkenlights::COLUMN0_LIGHT + i + j));
      }
      ++column;
    }
  }

  void appendContextMenu(Menu* menu) override {
    Blinkenlights* module = dynamic_cast<Blinkenlights*>(this->module);
    if (!module) return;
  }
};

Model* modelBlinkenlights =
    createModel<Blinkenlights, BlinkenlightsWidget>("Blinkenlights");
