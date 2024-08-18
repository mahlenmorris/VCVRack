#include "plugin.hpp"

#include "parser-venn/driver.h"

struct Venn : Module {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    X_POSITION_INPUT,
    Y_POSITION_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    DISTANCE_OUTPUT,
    WITHIN_GATE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Venn() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(X_POSITION_INPUT, "");
    configInput(Y_POSITION_INPUT, "");
    configOutput(DISTANCE_OUTPUT, "");
    configOutput(WITHIN_GATE_OUTPUT, "");

    const char * initial = 
      "[Reverb] "
      "x = 6.0 "
      "y = 5.0 "
      "radius = 3.5 "
      "[Delay Time] "
      "x = 4 "
      "y = 5.5 "
      "radius = 3 ";
    
    // Just to have something to start with during development, here are two overlapping Circles.
    // TODO: put the compilation on a background thread, once the Tipsy port is live.
    VennDriver driver;
    driver.parse(initial);
    circles = driver.diagram.circles;   

    current_circle = 0;

    point.x = 5;
    point.y = 5;
  }

  // Save and retrieve menu choice(s).
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    /*
    json_t* default_inJ = json_object_get(rootJ, "default_in");
    if (default_inJ) {
      default_in_voltage = json_real_value(default_inJ);
    }
    */
  }

  void process(const ProcessArgs& args) override {
    // Update X and Y inputs.
    if (inputs[X_POSITION_INPUT].isConnected()) {
      point.x = inputs[X_POSITION_INPUT].getVoltage();
    }
    if (inputs[Y_POSITION_INPUT].isConnected()) {
      point.y = inputs[Y_POSITION_INPUT].getVoltage();
    }
    // Determine what values to output.
    // TODO: many optimizations, including doing nothing when neither point nor circles has changed.
    // TODO: Can SIMD library help me?

    // TODO: We will need deleted circles to make this work when we can delete circles.
    outputs[DISTANCE_OUTPUT].setChannels(circles.size());
    outputs[WITHIN_GATE_OUTPUT].setChannels(circles.size());

    for (size_t channel = 0; channel < circles.size(); channel++) {
      const Circle& circle = circles.at(channel);
      if (circle.present) {
        float x_distance = point.x - circle.x_center;
        float y_distance = point.y - circle.y_center;
        float distance = sqrt(x_distance * x_distance + y_distance * y_distance);

        if (distance > circle.radius) {
          outputs[DISTANCE_OUTPUT].setVoltage(0.0f, channel);
          outputs[WITHIN_GATE_OUTPUT].setVoltage(0.0f, channel);
        } else {
          // We'll do linear distance scaling for now.
          outputs[DISTANCE_OUTPUT].setVoltage((1 - distance / circle.radius) * 10, channel);
          outputs[WITHIN_GATE_OUTPUT].setVoltage(10.0f, channel);
        }
      } else {
        outputs[DISTANCE_OUTPUT].setVoltage(0.0f, channel);
        outputs[WITHIN_GATE_OUTPUT].setVoltage(0.0f, channel);
      }
    }
  }

  std::vector<Circle> circles;
  int current_circle;
  Vec point;
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
      module->point.x = e.pos.x / bounding_box.x * 10.0;
      module->point.y = (1 - (e.pos.y / bounding_box.y)) * 10.0;
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
        module->point.x = e.pos.x / bounding_box.x * 10.0;
        module->point.y = (1 - (e.pos.y / bounding_box.y)) * 10.0;
        e.consume(this);
    }
  }

  void onSelectKey(const SelectKeyEvent& e) override {
    // TODO: impose max/min size and position.
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
      if (module->current_circle >= 0) {  // i.e., there is a circle to edit.
        // Editing the Circle.
        // W - up
        if (e.keyName == "w" && (e.mods & RACK_MOD_CTRL) == 0) {
          module->circles.at(module->current_circle).y_center += 0.1;
          e.consume(this);
        }
        // S - down
        if (e.keyName == "s" && (e.mods & RACK_MOD_CTRL) == 0) {
          module->circles.at(module->current_circle).y_center -= 0.1;
          e.consume(this);
        }
        // A - left
        if (e.keyName == "a" && (e.mods & RACK_MOD_CTRL) == 0) {
          module->circles.at(module->current_circle).x_center -= 0.1;
          e.consume(this);
        }
        // D - right
        if (e.keyName == "d" && (e.mods & RACK_MOD_CTRL) == 0) {
          module->circles.at(module->current_circle).x_center += 0.1;
          e.consume(this);
        }
        // Q - smaller
        if (e.keyName == "q" && (e.mods & RACK_MOD_CTRL) == 0) {
          module->circles.at(module->current_circle).radius -= 0.1;
          e.consume(this);
        }
        // E - bigger
        if (e.keyName == "e" && (e.mods & RACK_MOD_CTRL) == 0) {
          module->circles.at(module->current_circle).radius += 0.1;
          e.consume(this);
        }
      }
      // Selecting which Circle.
      // Z - previous
      if (e.keyName == "z" && (e.mods & RACK_MOD_CTRL) == 0) {
        // TODO: what if there are none? What happens above?
        if (module->circles.size() > 0) {
          for (int curr = module->current_circle - 1; curr != module->current_circle; curr--) {
            if (curr < 0) {
              curr = module->circles.size() - 1;
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
        // TODO: what if there are none? What happens above?
        if (module->circles.size() > 0) {
          for (int curr = module->current_circle + 1; curr != module->current_circle; curr++) {
            if (curr >= (int) (module->circles.size())) {
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
        // TODO: Center on where mouse is (if available).
        circle.x_center = 5.0;  
        circle.y_center = 5.0;
        circle.radius = 1.0;
        circle.present = true;
        // What position should this be in?
        // Let's start with the first non-present slot.
        // Limit of 16 channels in a cable.
        for (int curr = 0; curr < 16; curr++) {
          if (curr == (int) module->circles.size()) {
            module->circles.push_back(circle);
            module->current_circle = curr;
            break;
          }
          if (!(module->circles.at(curr).present)) {
            module->circles.at(curr) = circle;
            module->current_circle = curr;
            break;
          }
        }
        e.consume(this);
      }

      // x - delete current one.
      if (e.keyName == "x" && (e.mods & RACK_MOD_CTRL) == 0) {
        if (module->current_circle >= 0) {  // i.e., there is a circle to delete.
          module->circles.at(module->current_circle).present = false;
          // Move focus to next circle, if any.
          bool found_next = false;
          if (module->circles.size() > 0) {
            for (int curr = module->current_circle + 1; curr != module->current_circle; curr++) {
              if (curr >= (int) (module->circles.size())) {
                curr = 0;
              }
              if (module->circles.at(curr).present) {
                module->current_circle = curr;
                found_next = true;
                break;
              }
            }
          }
          if (!found_next) {
            module->current_circle = -1;  // Indicares no currently selected circle.
          }
        }
        e.consume(this);
      }
    }
  }

  double nvg_x(float volt_x, double size) { 
    return volt_x * size / 10.0;
  }

  double nvg_y(float volt_y, double size) { 
    // Need to invert Y, since I want to put y == 0 on the bottom.
    return size - (volt_y * size / 10.0);
  }
  
  // By using drawLayer() instead of draw(), this becomes a glowing Depict
  // when the "room lights" are turned down. That seems correct to me.
  void drawLayer(const DrawArgs& args, int layer) override {
    if (module && layer == 1) {
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
      // TODO: add number, name, and vary the color (same scheme as Memory?).
      int index = -1;
      // TODO: Change to different font.
      std::shared_ptr<Font> font = APP->window->loadFont(
        asset::plugin(pluginInstance, "fonts/RobotoSlab-Regular.ttf"));

      // Only indicate that a circle is selected if Venn module has focus. 
      bool is_selected = (this == APP->event->selectedWidget);

      for (const Circle& circle : module->circles) {
        index++;
        if (circle.present) {
          nvgBeginPath(args.vg);
          nvgCircle(args.vg, nvg_x(circle.x_center, bounding_box.x), nvg_y(circle.y_center, bounding_box.x),
                  pixels_per_volt * circle.radius);
          nvgStrokeColor(args.vg, colors[index % COLOR_COUNT]);
          nvgStrokeWidth(args.vg, index == module->current_circle && is_selected ? 2.0 : 1.0);
          nvgStroke(args.vg);

          nvgFillColor(args.vg, colors[index % COLOR_COUNT]);
          nvgFontSize(args.vg, index == module->current_circle && is_selected ? 15 : 13);
          nvgFontFaceId(args.vg, font->handle);
          //nvgTextLetterSpacing(args.vg, -2);
          // Place in the center.
          // TODO: Precompute this string, so don't have to keep remaking it.
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
      nvgCircle(args.vg, nvg_x(module->point.x, bounding_box.x), nvg_y(module->point.y, bounding_box.x),
              pixels_per_volt * 0.15);
      nvgStrokeColor(args.vg, SCHEME_WHITE);
      nvgStrokeWidth(args.vg, 1.0);
      nvgStroke(args.vg);

      OpaqueWidget::draw(args);
      nvgResetScissor(args.vg);
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

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 48.0)), module, Venn::X_POSITION_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.989, 47.993)), module, Venn::Y_POSITION_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(174.36, 12.435)), module, Venn::DISTANCE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(174.625, 30.162)), module, Venn::WITHIN_GATE_OUTPUT));

    // The Circles.
    CircleDisplay* display = createWidget<CircleDisplay>(
      mm2px(Vec(26.19, 1.5)));
    display->box.size = mm2px(Vec(125.0, 125.0));
    display->module = module;
    addChild(display);

  }
};


Model* modelVenn = createModel<Venn, VennWidget>("Venn");