// Blinkenlights is a visual module solely devoted to filling the patch with
// pleasing visual fluff. Inspired by the lighting props on film sets. The name
// is from cards posted in labs such as described here:
// https://en.wikipedia.org/wiki/Blinkenlights

#include <unordered_set>

#include "plugin.hpp"

static constexpr int MAX_LIGHTS_PER_COLUMN = 20;
static constexpr int LIGHT_COLUMNS = 4;

struct LightPattern {
  virtual void process(bool new_frame, std::vector<Light>* lights,
                       int index_start) = 0;
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

  void process(bool new_frame, std::vector<Light>* lights,
               int index_start) override {
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
        }
      }
    }
  }

  void process(bool new_frame, std::vector<Light>* lights,
               int index_start) override {
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
        // TODO: consider turning off lights more slowly with
        // setBrightnessSmooth().
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
  // If set, need to select a cbale to read from.
  bool pick_cable = true;

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

  void onRandomize(const RandomizeEvent& e) override {
    Module::onRandomize(e);
    pick_cable = true;
  }

  void processBypass(const ProcessArgs& args) override {}

  void process(const ProcessArgs& args) override {
    // All cable checking and light updates are done in
    // BlinkenlightsWidget::step() so we don't burn audio thread CPU.
  }
};

struct BlinkenlightsWidget : ModuleWidget {
  // Sometimes we pick a bad cable. This is the time in seconds to wait for a 0
  // -> 10V transition before picking a new cable.
  static constexpr double CABLE_TIMEOUT_SECONDS = 5.0;
  // If we simply cannot find a cable, then switch this often.
  static constexpr double FALLBACK_FRAME_SECONDS = 0.5;

  // Cable whose input we are watching, or -1 if not found a good cable yet.
  int64_t watched_cable_id = -1;
  // If a cable is unchanging, add it here so we ignore it in the future.
  std::unordered_set<int64_t> ignored_cable_ids;
  double last_transition_time = 0.0;
  // For tracking 0.0->10.0 transitions.
  bool was_zero = false;

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

  void findCableToWatch() {
    watched_cable_id = -1;
    last_transition_time = system::getTime();
    was_zero = false;
    if (!APP || !APP->scene || !APP->scene->rack) {
      return;
    }
    std::vector<CableWidget*> candidates;
    // Find all cables whose input ivalue is currently 0.0 or 10.0.
    // This is a really crude filter for finding "beats" within the patch
    // to respond to.
    for (CableWidget* cw : APP->scene->rack->getCompleteCables()) {
      if (!cw || !cw->cable || ignored_cable_ids.count(cw->cable->id) > 0) {
        continue;
      }
      Cable* cable = cw->cable;
      if (cable->inputModule && cable->inputId >= 0) {
        float v = cable->inputModule->getInput(cable->inputId).getVoltage();
        if (std::fabs(v) < 1e-4f || std::fabs(v - 10.f) < 1e-4f) {
          candidates.push_back(cw);
        }
      }
    }

    // Pick a cable to monitor.
    if (!candidates.empty()) {
      int index = (int)std::floor(random::uniform() * candidates.size());
      if (index >= (int)candidates.size()) {
        index = (int)candidates.size() - 1;
      }
      Cable* cable = candidates[index]->cable;
      watched_cable_id = cable->id;
      float v = cable->inputModule->getInput(cable->inputId).getVoltage();
      was_zero = (std::fabs(v) < 1e-4f);
    }
  }

  void step() override {
    Blinkenlights* module = dynamic_cast<Blinkenlights*>(this->module);
    if (module && !module->isBypassed()) {
      if (module->pick_cable) {
        module->pick_cable = false;
        findCableToWatch();
      }

      bool new_frame = false;
      double now = system::getTime();
      CableWidget* cw =
          (watched_cable_id >= 0 && APP && APP->scene && APP->scene->rack)
              ? APP->scene->rack->getCable(watched_cable_id)
              : nullptr;
      if (cw && cw->isComplete() && cw->cable && cw->cable->inputModule &&
          cw->cable->inputId >= 0) {
        float v =
            cw->cable->inputModule->getInput(cw->cable->inputId).getVoltage();
        if (was_zero && std::fabs(v - 10.f) < 1e-4f) {
          // Did transition from 0.0 -> 10.0, move to new frame of the
          // animation.
          new_frame = true;
          was_zero = false;
          last_transition_time = now;
        } else {
          if (std::fabs(v) < 1e-4f) {
            was_zero = true;
          } else if (std::fabs(v - 10.f) >= 1e-4f) {
            was_zero = false;
          }
          if (now - last_transition_time > CABLE_TIMEOUT_SECONDS) {
            // Too long without a transition; ignore this cable for this run and
            // find another one.
            ignored_cable_ids.insert(watched_cable_id);
            findCableToWatch();
          }
        }
      } else {
        watched_cable_id = -1;
        // Fallback schedule when no matching cable is found or cable is
        // removed.
        if (now - last_transition_time >= FALLBACK_FRAME_SECONDS) {
          new_frame = true;
          last_transition_time = now;
        }
      }

      switch (module->format) {
        case 0:
        default: {
          module->random_block_pattern.process(new_frame, &module->lights,
                                               Blinkenlights::COLUMN0_LIGHT);
          break;
        }
      }
    }

    ModuleWidget::step();
  }

  void appendContextMenu(Menu* menu) override {
    Blinkenlights* module = dynamic_cast<Blinkenlights*>(this->module);
    if (!module) return;
  }
};

Model* modelBlinkenlights =
    createModel<Blinkenlights, BlinkenlightsWidget>("Blinkenlights");
