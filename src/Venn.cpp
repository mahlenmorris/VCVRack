#include "plugin.hpp"

#include "parser-venn/driver.h"

constexpr int PART_LEN = 18;
constexpr const char* PARTS[PART_LEN] = {
  "Bass",
  "Drum",
  "Lead",
  "Melody",
  "Bloop",
  "Noise",
  "Piano",
  "Tympani",
  "Flute",
  "Grit",
  "Airiness",
  "Choir",
  "Theremin",
  "Gamelan",
  "Kalimba",
  "Bagpipe",
  "Erhu",
  "Kora"
};

constexpr int EFFECT_LEN = 26;
static const char* EFFECTS[EFFECT_LEN] = {
  "Reverb",
  "Delay",
  "Chorus",
  "Flange",
  "Filter",
  "Mix",
  "Distortion",
  "Punch",
  "Smoother",
  "Phaser",
  "Limiter",
  "Pan",
  "Pitch",
  "Gate",
  "Scale",
  "Key",
  "Grit",
  "Space",
  "Throb",
  "Echo",
  "Attack",
  "Polarity",
  "Pressure",
  "Timbre",
  "Tempo",
  "Dissonance"
};

struct Venn : Module {
  enum ParamId {
		EXP_LIN_LOG_PARAM,
		X_POSITION_ATTN_PARAM,
		Y_POSITION_ATTN_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    X_POSITION_INPUT,
    Y_POSITION_INPUT,
		X_POSITION_WIGGLE_INPUT,
		Y_POSITION_WIGGLE_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    DISTANCE_OUTPUT,
    WITHIN_GATE_OUTPUT,
		X_DISTANCE_OUTPUT,
		Y_DISTANCE_OUTPUT,
 		X_POSITION_OUTPUT,
		Y_POSITION_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  std::vector<Circle> circles;
  int current_circle;
  bool circles_loaded = false;
  size_t live_circle_count;
  int check_live_circles;
  // Current position we use.
  Vec point;
  // Last human selected point (to wiggle from).
  Vec human_point;
  // If > -1, the channel to "solo" on.
  int solo_channel;

  Venn() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(EXP_LIN_LOG_PARAM, -1.f, 1.f, 0.f, "");
		configParam(X_POSITION_ATTN_PARAM, -1.0f, 1.0f, 0.0f, "Attenuverter for second input to X position of Point");
		configParam(Y_POSITION_ATTN_PARAM, -1.0f, 1.0f, 0.0f, "Attenuverter for second input to Y position of Point");

    configInput(X_POSITION_INPUT, "X position of Point - attenuated value of other input will be added");
    configInput(Y_POSITION_INPUT, "Y position of Point - attenuated value of other input will be added");
		configInput(X_POSITION_WIGGLE_INPUT, "Multiplied by attenuverter to alter the X position of Point");
		configInput(Y_POSITION_WIGGLE_INPUT, "Multiplied by attenuverter to alter the Y position of Point");

    configOutput(DISTANCE_OUTPUT, "0V at edge, 10V at center, polyphonic");
    configOutput(WITHIN_GATE_OUTPUT, "0V outside circle, 10V within, polyphonic");
		configOutput(X_POSITION_OUTPUT, "The current X coordinate of the point (-5V -> 5V). Useful for recording point gestures and performances.");
		configOutput(Y_POSITION_OUTPUT, "The current Y coordinate of the point (-5V -> 5V). Useful for recording point gestures and performances.");
		configOutput(X_DISTANCE_OUTPUT, "");
		configOutput(Y_DISTANCE_OUTPUT, "");

    current_circle = -1;
    check_live_circles = 0;
    
    // To avoid errors with improper indexes into circles, I just fill it up.
    Circle circle;
    circle.present = false;
    // TODO: Make circles into a vector of size 16!
    for (int i = 0; i < 16; ++i) {
      circles.push_back(circle);
    }
    circles_loaded = true;
    point.x = 0;
    point.y = 0;
    solo_channel = -1;
  }
  
  // Turns a set of shapes into a text string that we can parse later to recreate the shapes.
  std::string to_string(std::vector<Circle>& the_circles) {
    std::string result;
    // Don't need to store deleted circles with a larger index than the largest intact one.
    // So I'll just take this opportunity to erase any at the end.
    int last_index = -1;
    for (int curr = the_circles.size() - 1; curr >= 0; curr--) {
      if (the_circles[curr].present) {
        last_index = curr;
        break;
      }
    }
    for (int curr = 0; curr <= last_index; curr++) {
      result.append(the_circles[curr].to_string());
    }
    return result;
  }

  // Save and retrieve menu choice(s).
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    std::string diagram = to_string(circles);
    json_object_set_new(rootJ, "diagram", json_stringn(diagram.c_str(), diagram.size()));

    // We include "human_point" and "current_circle", so that it's exactly how user left it.
    // Like maybe the user has dialed in the exact right spot...
    json_t* saved_point = json_array();
    json_array_append_new(saved_point, json_real(human_point.x));
    json_array_append_new(saved_point, json_real(human_point.y));
    json_object_set(rootJ, "point", saved_point);
    json_decref(saved_point);
    json_object_set_new(rootJ, "current", json_integer(current_circle));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* pointJ = json_object_get(rootJ, "point");
    if (pointJ) {
      human_point.x = json_real_value(json_array_get(pointJ, 0));
      human_point.y = json_real_value(json_array_get(pointJ, 1));
    }
    json_t* currentJ = json_object_get(rootJ, "current");
    if (currentJ) {
      current_circle = json_integer_value(currentJ);
    }

    // We actually parse the string that defines the circles.
    json_t* diagramJ = json_object_get(rootJ, "diagram");
    if (diagramJ) {
      circles_loaded = false;
      std::string diagram = json_string_value(diagramJ);
      VennDriver driver;
      if (driver.parse(diagram) != 0) {
        WARN("Compile Failure:\n%s", diagram.c_str());
      }
      ClearAllCircles();
      for (int i = 0; i < std::min(16, (int) driver.diagram.circles.size()); ++i) {
        circles[i] = driver.diagram.circles.at(i);
      }
      json_t* currentJ = json_object_get(rootJ, "current");
      if (currentJ) {
        current_circle = json_integer_value(currentJ);
      } else {
        current_circle = driver.diagram.circles.size() > 0 ? 0 : -1;
      } 
    }
    circles_loaded = true;
  }

  void ClearAllCircles() {
    for (int i = 0; i < 16; ++i) {
      circles.at(i).present = false;
    }
  }

  void onReset(const ResetEvent& e) override {
    Module::onReset(e);

    // A lock around the sudden change, so process() and UI doesn't fail.
    circles_loaded = false;
    ClearAllCircles();
    current_circle = solo_channel = -1;
    human_point.x = 0.0;
    human_point.y = 0.0;
    circles_loaded = true;
  }

  float MyNormal() {
    return (random::uniform() + random::uniform()) / 2.0f; 
  }

  void onRandomize(const RandomizeEvent& e) override {
    Module::onRandomize(e);

    // When User hits Randomize, let's make some circles.
    int count = clamp((int) (MyNormal() * 10.0 + 3), 3, 13);
    circles_loaded = false;
    current_circle = solo_channel = -1;
    ClearAllCircles();
    for (int i = 0; i < count; ++i) {
        Circle circle;
        circle.x_center = random::uniform() * 9.6 - 4.8;  
        circle.y_center = random::uniform() * 9.9 - 4.8;
        circle.radius = MyNormal() * 5 + 1;
        circle.present = true;
        // Random names are delightful.
        // I'll allow them to range in length from 1 to three words, just to see how they look.
        // Thanks to @disquiet for suggesting I add evocative color names.
        std::string the_name(PARTS[(int) std::floor(random::uniform() * PART_LEN)]);
        if (random::uniform() > 0.333) {
          the_name.append(" ");
          the_name.append(EFFECTS[(int) (random::uniform() * sizeof(EFFECT_LEN))]);
          if (random::uniform() > 0.7) {
            the_name.append(" ");
            the_name.append(EFFECTS[(int) (random::uniform() * sizeof(EFFECT_LEN))]);
          }
        }
        circle.name = the_name;
        circles.at(i) = circle;
    }
    current_circle = 0;
    human_point.x = random::uniform() * 9.6 - 4.8;
    human_point.y = random::uniform() * 9.6 - 4.8;
    circles_loaded = true;
  }	

  // Fit values to range between -5..5.
  float WrapValue(float value) {
    while (value > 5.0) {
      value -= 10;
    }
    while (value < -5.0) {
      value += 10;
    }
    return value;
  }

  void process(const ProcessArgs& args) override {
    if (!circles_loaded) {
      return;
    }
    if (--check_live_circles <= 0) {
      // One sixtieth of a second.
      check_live_circles = (int) (args.sampleRate / 60);
      // It's not great that our channel count is basically the index of the highest
      // circle that has existed while this module is alive.
      // To avoid doing that, we occasionally check how many circles are present.
      // Deleting them is dangerous (multiple threads read the vector), so we don't do that.
      live_circle_count = 0;
      for (size_t channel = 0; channel < 16; channel++) {
        if (circles.at(channel).present) {
          live_circle_count = channel + 1;
        }
      }
    }
    // Determine where "point" is.
    // Update X and Y inputs.
    if (inputs[X_POSITION_INPUT].isConnected()) {
      point.x = inputs[X_POSITION_INPUT].getVoltage();
    } else {
      // We do these separately, so human can control one axis but not the other, if desired.
      point.x = human_point.x;
    }
    if (inputs[Y_POSITION_INPUT].isConnected()) {
      point.y = inputs[Y_POSITION_INPUT].getVoltage();
    } else {
      point.y = human_point.y;
    }

    // TODO: for this to work how I want it to, I need to track the last human selected point.
    if (params[X_POSITION_ATTN_PARAM].getValue() != 0.0f) {
      point.x += params[X_POSITION_ATTN_PARAM].getValue() *
        inputs[X_POSITION_WIGGLE_INPUT].getVoltage();
    }
    if (params[Y_POSITION_ATTN_PARAM].getValue() != 0.0f) {
      point.y += params[Y_POSITION_ATTN_PARAM].getValue() *
        inputs[Y_POSITION_WIGGLE_INPUT].getVoltage();
    }

    // TODO: make this a menu option? It's actually odd not to have walls.
    bool wrapping = false;
    if (wrapping) {
      // Wrap point values.
      point.x = WrapValue(point.x);
      point.y = WrapValue(point.y);
    } else {
      // Keep within walls.
      point.x = fmax(-5, fmin(5, point.x));
      point.y = fmax(-5, fmin(5, point.y));
    }
    // Wrap point values.
    // TODO: make this a menu option? It's actually odd not to have walls.
    point.x = WrapValue(point.x);
    point.y = WrapValue(point.y);

    // We have now determined the postion of "point".
    outputs[X_POSITION_OUTPUT].setVoltage(point.x);
    outputs[Y_POSITION_OUTPUT].setVoltage(point.y);

    // Determine what values to output.
    // TODO: many optimizations, including doing nothing when neither point nor circles has changed.
    // TODO: Can SIMD library help me?

    outputs[DISTANCE_OUTPUT].setChannels(live_circle_count);
    outputs[WITHIN_GATE_OUTPUT].setChannels(live_circle_count);
    outputs[X_DISTANCE_OUTPUT].setChannels(live_circle_count);
    outputs[Y_DISTANCE_OUTPUT].setChannels(live_circle_count);

    float scaling = params[EXP_LIN_LOG_PARAM].getValue();
    if (outputs[DISTANCE_OUTPUT].isConnected()) {
      // Save the compute time if not going to use 'scaling'.

      // If we are from -1 - 0, we want to scale it 0.1 - 1.
      if (scaling == 0.0) {
        scaling = 1.0;
      } else if (scaling < 0.0) {
        scaling = rack::math::rescale(scaling, -1, 0, .1, 1);
      } else {
        scaling = rack::math::rescale(scaling, 0, 1, 1, 10);
      }
    }
    for (size_t channel = 0; channel < live_circle_count; channel++) {
      const Circle& circle = circles.at(channel);
      // If solo-ing, make sure that only solo channel gets computed.
      if (circle.present && (solo_channel < 0 || (int) channel == solo_channel)) {
        // All of the outputs care if we are in the circle or not, so we always compute it,
        // regardless of there is a cable connected or not.
        float x_distance = point.x - circle.x_center;
        float y_distance = point.y - circle.y_center;

        // TODO: eliminate this sqrt() call? But the distance actually matters to the result.
        // Maybe there is a cheaper approximation? And we only need the true distance for
        // DISTANCE_OUTPUT.
        float distance = sqrt(x_distance * x_distance + y_distance * y_distance);
        if (distance > circle.radius) {
          outputs[DISTANCE_OUTPUT].setVoltage(0.0f, channel);
          outputs[WITHIN_GATE_OUTPUT].setVoltage(0.0f, channel);
          // An odd default value, since it is the same value as being in line with the center.
          // TODO: is there a better choice?
          outputs[X_DISTANCE_OUTPUT].setVoltage(0.0f, channel);
          outputs[Y_DISTANCE_OUTPUT].setVoltage(0.0f, channel);
        } else {
          outputs[WITHIN_GATE_OUTPUT].setVoltage(10.0f, channel);
          if (outputs[DISTANCE_OUTPUT].isConnected()) {
            float value = (1 - distance / circle.radius);
            // Since 1.0 is the default, skip the call to pow() if not needed.
            if (scaling != 1.0) {
              value = pow(value, scaling);
            }
            outputs[DISTANCE_OUTPUT].setVoltage(value * 10, channel);            
          }

          if (outputs[X_DISTANCE_OUTPUT].isConnected()) {
            outputs[X_DISTANCE_OUTPUT].setVoltage(x_distance / circle.radius * 5.0, channel);
          }
          if (outputs[Y_DISTANCE_OUTPUT].isConnected()) {
            outputs[Y_DISTANCE_OUTPUT].setVoltage(y_distance / circle.radius * 5.0, channel);
          }
        }
      } else {
        outputs[DISTANCE_OUTPUT].setVoltage(0.0f, channel);
        outputs[WITHIN_GATE_OUTPUT].setVoltage(0.0f, channel);
        outputs[X_DISTANCE_OUTPUT].setVoltage(0.0f, channel);
        outputs[Y_DISTANCE_OUTPUT].setVoltage(0.0f, channel);
      }
    }
    outputs[DISTANCE_OUTPUT].setChannels(live_circle_count);
    outputs[WITHIN_GATE_OUTPUT].setChannels(live_circle_count);
  }
};

// Adds support for undo/redo for changes to a single Circle.
struct VennCircleUndoRedoAction : history::ModuleAction {
  enum CircleEditType {
    CHANGE,
    ADDITION,
    DELETION
  };
  CircleEditType edit;
  Circle old_circle;
  Circle new_circle;
  int old_position;  // Index into circles this was at. CHANGE doesn't alter the index.
  int new_position;  // Index after a non-CHANGE action.

  VennCircleUndoRedoAction(
      int64_t id, const Circle& oldCircle, const Circle& newCircle,
      int old_index) : old_circle{oldCircle}, new_circle{newCircle}, old_position{old_index} {
    moduleId = id;
    name = "circle edit";  // TODO: split this into circle enlarge/circle shrink/circle move?
    edit = CHANGE;
  }

  VennCircleUndoRedoAction(int64_t id, const Circle& circle, int old_index,
                           int new_index, bool addition) : 
                           old_position{old_index}, new_position{new_index} {
    moduleId = id;
    if (addition) {
      name = "circle addition";
      edit = ADDITION;
      new_circle = circle;
    } else {
      name = "circle deletion";
      edit = DELETION;
      old_circle = circle;
    }
  }

  void undo() override {
    Venn *module = dynamic_cast<Venn*>(APP->engine->getModule(moduleId));
    if (module) {
      module->circles_loaded = false;
      switch (edit) {
        case DELETION: {
          module->circles[old_position] = old_circle;
          module->current_circle = old_position;
        }
        break;
        case CHANGE: {
          module->circles[old_position] = old_circle;
        }
        break;
        case ADDITION: {
          module->circles[new_position].present = false;
          module->current_circle = old_position;
        }
        break;
      }
      module->circles_loaded = true;
    }
  }

  void redo() override {
    Venn *module = dynamic_cast<Venn*>(APP->engine->getModule(moduleId));
    if (module) {
      module->circles_loaded = false;
      switch (edit) {
        case DELETION: {
          module->circles[old_position].present = false;
          module->current_circle = new_position;
        }
        break;
        case CHANGE: {
          module->circles[old_position] = new_circle;
        }
        break;
        case ADDITION: {
          module->circles[new_position] = new_circle;
          module->current_circle = new_position;

        }
      }
      module->circles_loaded = true;
    }
  }
};

struct CircleDisplay : OpaqueWidget {
  Venn* module;

  CircleDisplay() {}

  static constexpr int COLOR_COUNT = 7;
  NVGcolor colors[COLOR_COUNT] = {
    SCHEME_RED,
    SCHEME_BLUE,
    SCHEME_ORANGE,
    SCHEME_PURPLE,
    SCHEME_GREEN,
    SCHEME_CYAN,
    SCHEME_WHITE
  };


  // Move point to current location if left clicked.
  void onButton(const ButtonEvent& e) override {
    OpaqueWidget::onButton(e);

    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
      // Must change position in widget to voltage.
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();
      module->human_point.x = module->point.x = e.pos.x / bounding_box.x * 10.0 - 5;
      module->human_point.y = module->point.y = (1 - (e.pos.y / bounding_box.y)) * 10.0 - 5;
      e.consume(this);
    }

    /*   
    // Hmm, might care about right button presses later?   
    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
      createContextMenu();
      e.consume(this);
    }
    */
  }

  // Move point to current location when dragged.
  void onDragHover(const DragHoverEvent& e) override {
    OpaqueWidget::onDragHover(e);

    if (e.origin == this) {
        // Must change position in widget to voltage.
        Rect r = box.zeroPos();
        Vec bounding_box = r.getBottomRight();
        module->human_point.x = module->point.x = e.pos.x / bounding_box.x * 10.0 - 5;
        module->human_point.y = module->point.y = (1 - (e.pos.y / bounding_box.y)) * 10.0 - 5;
        e.consume(this);
    }
  }

  void RememberChange(const Circle& old_circle) {
    APP->history->push(
      new VennCircleUndoRedoAction(module->id, old_circle,
                                   module->circles.at(module->current_circle),
                                   module->current_circle));
  }

  void onSelectKey(const SelectKeyEvent& e) override {
    if (!module->circles_loaded) {
      // Don't edit circles if we're in the middle of loading them!
      return;
    } 
    // Undo/redo can make current_circle invalid. Rationalize it.
    // TODO: impose max/min size and position.
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
      if (module->current_circle >= 0) {  // i.e., there is a circle to edit.
        // Editing the Circle.
        // W - up
        if (e.keyName == "w" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles.at(module->current_circle));
          module->circles.at(module->current_circle).y_center += 0.1;
          RememberChange(old);
          e.consume(this);
        }
        // S - down
        if (e.keyName == "s" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles.at(module->current_circle));
          module->circles.at(module->current_circle).y_center -= 0.1;
          RememberChange(old);
          e.consume(this);
        }
        // A - left
        if (e.keyName == "a" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles.at(module->current_circle));
          module->circles.at(module->current_circle).x_center -= 0.1;
          RememberChange(old);
          e.consume(this);
        }
        // D - right
        if (e.keyName == "d" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles.at(module->current_circle));
          module->circles.at(module->current_circle).x_center += 0.1;
          RememberChange(old);
          e.consume(this);
        }
        // Q - smaller
        if (e.keyName == "q" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles.at(module->current_circle));
          module->circles.at(module->current_circle).radius -= 0.1;
          if (module->circles.at(module->current_circle).radius < 0.1) {
            module->circles.at(module->current_circle).radius = 0.1;
          }
          RememberChange(old);
          e.consume(this);
        }
        // E - bigger
        if (e.keyName == "e" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles.at(module->current_circle));
          module->circles.at(module->current_circle).radius += 0.1;
          RememberChange(old);
          e.consume(this);
        }
      }
      // Selecting which Circle.
      // Z - previous
      if (e.keyName == "z" && (e.mods & RACK_MOD_CTRL) == 0) {
        if (module->live_circle_count > 0) {
          for (int curr = module->current_circle - 1; curr != module->current_circle; curr--) {
            if (curr < 0) {
              curr = 16 - 1;
            }
            if (module->circles.at(curr).present) {
              module->current_circle = curr;
              break;
            }
          }
        }
        e.consume(this);
      }
      // C - next
      if (e.keyName == "c" && (e.mods & RACK_MOD_CTRL) == 0) {
        if (module->live_circle_count > 0) {
          for (int curr = module->current_circle + 1; curr != module->current_circle; curr++) {
            if (curr >= 16) {
              curr = 0;
            }
            if (module->circles.at(curr).present) {
              module->current_circle = curr;
              break;
            }
          }
        }
        e.consume(this);
      }

      // Creation and deletion.
      // F - add new one
      if (e.keyName == "f" && (e.mods & RACK_MOD_CTRL) == 0) {
        Circle circle;
        int old_index = module->current_circle;
        // TODO: Center on where mouse is (if available).
        circle.x_center = random::uniform() * 2 - 1;  
        circle.y_center = random::uniform() * 2 - 1;
        circle.radius = 1.0 + random::uniform();
        circle.present = true;
        // What position should this be in?
        // Let's start with the first non-present slot.
        // Limit of 16 channels in a cable, so won't add after that.
        bool added = false;
        for (int curr = 0; curr < 16; curr++) {
          if (!(module->circles.at(curr).present)) {
            module->circles.at(curr) = circle;
            module->current_circle = curr;
            added = true;
            break;
          }
        }
        if (added) {
          APP->history->push(
            new VennCircleUndoRedoAction(module->id, module->circles.at(module->current_circle),
                                         old_index, module->current_circle, true));
        }
        e.consume(this);
      }

      // x - delete current one.
      if (e.keyName == "x" && (e.mods & RACK_MOD_CTRL) == 0) {
        if (module->current_circle >= 0) {  // i.e., there is a circle to delete.
          // Copy circle *before* "present" is set to false!
          Circle old_circle(module->circles.at(module->current_circle));
          int old_index = module->current_circle;
          module->circles.at(module->current_circle).present = false;
          // Move focus to next circle, if any.
          bool found_next = false;
          for (int curr = module->current_circle + 1; curr != module->current_circle; curr++) {
            if (curr >= 16) {
              curr = 0;
            }
            if (module->circles.at(curr).present) {
              module->current_circle = curr;
              found_next = true;
              break;
            }
          }
          if (!found_next) {
            module->current_circle = -1;  // Indicares no currently selected circle.
          }
          APP->history->push(
            new VennCircleUndoRedoAction(module->id, old_circle,
                                         old_index, module->current_circle, false));
        }
        e.consume(this);
      }

      // Solo this channel or return to full poly.
      if (e.keyName == "r" && (e.mods & RACK_MOD_CTRL) == 0) {
        if (module->current_circle >= 0) {  // i.e., there is a circle to solo.
          // Set or unset the solo_channel.
          module->solo_channel = (module->solo_channel >= 0) ? -1 : module->current_circle;
        }
        e.consume(this);
      }      
    }
  }

  double nvg_x(float volt_x, double size) { 
    return (volt_x + 5) * size / 10.0;
  }

  double nvg_y(float volt_y, double size) { 
    // Need to invert Y, since I want to put y == -5 on the bottom.
    return size - ((volt_y + 5) * size / 10.0);
  }

  // Caller is responsible for deleting this.
  std::vector<Circle>* default_circles() {
    float x[6] = {0.062705, -2.653372, 2.517570, 3.260226, -2.121736, 0.974539};
    float y[6] = {-0.411569, 2.684878, 4.339533, -0.208806, -2.508809, -4.108777};
    float radius[6] = {1.742869, 4.004038, 2.987639, 2.559288, 2.169408, 3.424444};

    std::vector<Circle>* circs = new std::vector<Circle>();
    for (int i = 0; i < 6; ++i) {
      Circle circle;
      circle.x_center = x[i];
      circle.y_center = y[i];
      circle.radius = radius[i];
      circle.present = true;
      circs->push_back(circle);
    }
    return circs;
  }

  // By using drawLayer() instead of draw(), this becomes a glowing Depict
  // when the "room lights" are turned down. That seems correct to me.
  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      std::vector<Circle>* circles;
      int current_circle, solo_circle; 
      Vec point;
      if (module) {
        // If we have a module, but the circles are being updated, best not to draw anything.
        // TODO: reconsider this decision, since we no longer have the issue of out-of-range indexies.
        if (!module->circles_loaded) {
          return;
        } else {
          circles = &(module->circles);
          current_circle = module->current_circle;
          solo_circle = module->solo_channel;
          point = module->point;
        }
      } else {
        // Simple demo values to show in the browser and library page.
        circles = default_circles();
        current_circle = 2;
        solo_circle = -1;
        point.x = 0.0;
        point.y = 0.2345;
      }
      nvgScissor(args.vg, RECT_ARGS(args.clipBox));
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();
      // Assuming that we are always on a square-pixeled surface, with X and Y the same distances.
      double pixels_per_volt = bounding_box.x / 10.0;

      // Background first.
      nvgBeginPath(args.vg);
      nvgRect(args.vg, 0.0, 0.0, bounding_box.x, bounding_box.y);
      nvgFillColor(args.vg, SCHEME_DARK_GRAY);
      nvgFill(args.vg);

      // The circles.
      int index = -1;
      // TODO: Change to different font?
      std::shared_ptr<Font> font = APP->window->loadFont(
        asset::plugin(pluginInstance, "fonts/RobotoSlab-Regular.ttf"));

      // Only indicate that a circle is selected if Venn module has focus. 
      bool is_selected = (this == APP->event->selectedWidget);

      for (const Circle& circle : *circles) {
        index++;
        if (circle.present) {
          // Draw the circle itself.
          nvgBeginPath(args.vg);
          nvgCircle(args.vg, nvg_x(circle.x_center, bounding_box.x), nvg_y(circle.y_center, bounding_box.x),
                  pixels_per_volt * circle.radius);
          NVGcolor circle_color = colors[index % COLOR_COUNT];
          if (solo_circle >= 0 && solo_circle != index) {
            // Dim the muted circles.
            circle_color = nvgTransRGBAf(circle_color, 0.3);
          }
          nvgStrokeColor(args.vg, circle_color);
          nvgStrokeWidth(args.vg, index == current_circle && is_selected ? 2.0 : 1.0);
          nvgStroke(args.vg);
          
          // Now draw the text in the center.
          nvgFillColor(args.vg, colors[index % COLOR_COUNT]);
          nvgFontSize(args.vg, index == current_circle && is_selected ? 15 : 13);
          nvgFontFaceId(args.vg, font->handle);
          //nvgTextLetterSpacing(args.vg, -2);
          // Place in the center. TODO: make it apparent where the center of the circle is.
          // TODO: Precompute this string, so don't have to keep remaking it.
          // Or just let FrameBuffer cache the entire result.
          std::string center_text = std::to_string(index + 1);
          if (!circle.name.empty()) {
            center_text.append(" - ");
            center_text.append(circle.name);
          }
          nvgText(args.vg, nvg_x(circle.x_center, bounding_box.x),
                          nvg_y(circle.y_center, bounding_box.x),
                          center_text.c_str(), NULL);
        }
      }

      // Draw the pointer (need a name for it).
      nvgBeginPath(args.vg);
      nvgCircle(args.vg, nvg_x(point.x, bounding_box.x), nvg_y(point.y, bounding_box.x),
              pixels_per_volt * 0.15);
      nvgStrokeColor(args.vg, SCHEME_WHITE);
      nvgStrokeWidth(args.vg, 1.0);
      nvgStroke(args.vg);

      OpaqueWidget::draw(args);
      nvgResetScissor(args.vg);

      if (!module) {
        delete circles;
      }
    }
	}
};


struct VennWidget : ModuleWidget {
  VennWidget(Venn* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Venn.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(166.582, 11.906)), module, Venn::EXP_LIN_LOG_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(9.446, 25.0)), module, Venn::X_POSITION_ATTN_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(21.034, 25.0)), module, Venn::Y_POSITION_ATTN_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.446, 16.0)), module, Venn::X_POSITION_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.034, 16.0)), module, Venn::Y_POSITION_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.446, 34.0)), module, Venn::X_POSITION_WIGGLE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.034, 34.0)), module, Venn::Y_POSITION_WIGGLE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.446, 45.0)), module, Venn::X_POSITION_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.034, 45.0)), module, Venn::Y_POSITION_OUTPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(178.17, 11.906)), module, Venn::DISTANCE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(178.17, 30.162)), module, Venn::WITHIN_GATE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(166.582, 47.286)), module, Venn::X_DISTANCE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(178.17, 47.286)), module, Venn::Y_DISTANCE_OUTPUT));

    // The Circles.
    CircleDisplay* display = createWidget<CircleDisplay>(
      mm2px(Vec(31.0, 1.7)));
    display->box.size = mm2px(Vec(125.0, 125.0));
    display->module = module;
    addChild(display);

  }

  void appendContextMenu(Menu* menu) override {
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Inspired by Leafcutter John's 'Forester' instrument."));
  }
};


Model* modelVenn = createModel<Venn, VennWidget>("Venn");