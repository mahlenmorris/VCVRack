#include "plugin.hpp"

#include "parser-venn/driver.h"
#include "st_textfield.hpp"

#include <chrono>   
#include <thread>

constexpr int VENN_COLOR_COUNT = 7;
NVGcolor venn_colors[VENN_COLOR_COUNT] = {
  SCHEME_RED,
  SCHEME_BLUE,
  SCHEME_ORANGE,
  SCHEME_PURPLE,
  SCHEME_GREEN,
  SCHEME_CYAN,
  SCHEME_WHITE
};

constexpr int VENN_COLOR_NAMES = 8;  // Number of names per color.
constexpr const char* VENN_COLOR_ARRAY[VENN_COLOR_COUNT][VENN_COLOR_NAMES] = {
  { "Scarlet", "Crimson", "Ruby", "Cherry", "Rose", "Vermilion", "Maroon", "Firetruck" }, 
  { "Azure", "Navy", "Indigo", "Ocean", "Sky", "Blueberry", "Sapphire", "Bluebird" },
  { "Tangerine", "Cinnamon", "Pumpkin", "Carrot", "Tiger", "Juice", "Fire", "Warm" },
  { "Lavender", "Violet", "Lilac", "Amethyst", "Eggplant", "Grape", "Plum", "Royal" },
  { "Jade", "Olive", "Lime", "Grass", "Tree", "Leaf", "Frog", "Lime" },
  { "Aqua", "Calm", "Peace", "Serenity", "Clarity", "Turquoise", "Teal", "Seafoam" },
  { "Ivory", "Cream", "Clean", "Simple", "Snow", "Cloud", "Milk", "Pearl" }
};

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
  "Air",
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

constexpr int VARIABLES_LEN = 6;
static const char* VARIABLES[VARIABLES_LEN] = {
  "pointx",
  "pointy",
  "distance",
  "within",
  "x",
  "y"
};

constexpr int SINGLE_ARG_LEN = 6;
static const char* SINGLE_FUNCS[SINGLE_ARG_LEN] = {
  "abs",
  "ceiling",
  "floor",
  "log2",
  "sign",
  "sin"
};

constexpr int BOOL_OP_LEN = 6;
static const char* BOOL_OPS[BOOL_OP_LEN] = {
  "==",
  ">=",
  "<=",
  "<",
  ">",
  "!="
};

std::string notes("abcdefg");
std::string simple_operators("+-*/");

struct Venn : Module {
  enum ParamId {
		EXP_LIN_LOG_PARAM,
		X_POSITION_ATTN_PARAM,
		Y_POSITION_ATTN_PARAM,
		INV_WITHIN_PARAM,
		INV_X_PARAM,
		INV_Y_PARAM,
		OFST_X_PARAM,
		OFST_Y_PARAM,
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
    MATH1_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
  	INV_WITHIN_LIGHT,
		INV_X_LIGHT,
		INV_Y_LIGHT,
		OFST_X_LIGHT,
		OFST_Y_LIGHT,
    LIGHTS_LEN
  };

  /*
  // Class devoted to handling the lengthy (compared to single sample)
  // process of compiling code for expressions.
  struct CompilationThread {
    VennDriver* driver;
    bool shutdown;
    bool initiate_compile;
    std::string text;  // Text to compile.
    bool running;  // TRUE if still compiling, false if completed.
    bool useful; // TRUE if last completed compile created something for module to use.
    // Product of a successful compilation.
    VennExpression result_exp;

    explicit CompilationThread(VennDriver* drv) : driver{drv} {
      running = false;
      useful = false;
      shutdown = false;
      initiate_compile = false;
    }

    void Halt() {
      shutdown = true;
      initiate_compile = false;
    }

    void StartNewCompile(const std::string &new_text, int circle) {
      running = true;  // Tells caller not to use previous result.
      text = new_text;
      initiate_compile = true;
    }

    // If compilation succeeds, sets flag saying so and prepares vectors
    // of main_blocks and expression_blocks for module to use later.
    void Compile() {
      while (!shutdown) {
        if (initiate_compile) {
          running = true;
          useful = false;
          // 'text' might get changed during compilation which would be bad.
          // Let's copy it and send that.
          std::string stable_text(text);
          bool compiles = !driver->parse(stable_text);
          if (compiles) {
            result_exp = driver->exp;  // Should COPY it.
            useful = true;
          }
          initiate_compile = false;
          running = false;  // Signals caller to pick up result.
        }
        // It seems like I need a tiny sleep here to allow join() to work
        // on this thread?
        // TODO: maybe make this longer, like 10ms?
        if (!shutdown) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
      }
    }
  };
  */

  // The array of circles.
  Circle circles[16];
  // Index to the circle being edited, or -1 if no circles are available.
  int current_circle;
  // Flag to indicate that circles are being wholesale updated, maybe
  // don't look at them right now.
  bool circles_loaded = false;
  // Number of live channels, updated 60 times/second.
  // Optimization for setting channel count and not iterating through
  // high-numbered unused circles.
  size_t live_circle_count;
  // Counts down process() calls until it's time to check if there are
  // unused circles we can stop computing for.
  // That process updates live_circle_count.
  int check_live_circles;
  // Current position we use.
  Vec point;
  // Last human selected point (to wiggle from).
  // Set by the UI.
  Vec human_point;
  // If true, the current channel is solo'd.
  // Set and unset by the UI widget.
  bool solo;
  // True if UI believes the circles (or point) are being edited.
  // Useful for other UI elements to know.
  bool editing;
  // True iff editor's keystrokes are accepted (i.e., the keyboard map)
  bool keystrokes_accepted;
  // Menu item that decides whether to show the readable keyboard hints
  // or just the icon indicating that keyboard input is accepted.
  // TODO: This should probably apply to all instances of Venn at once, but reading XTStyle,
  // it's more work than I want to do for V1.
  bool show_keyboard;
  // Flag to VennWidget::step() to update any text fields.
  // Indicates that something other than the UI has just changed circles.
  bool update_text_widgets;
  // Menu item to only compute MATH1 items when WITHIN a circle.
  // Obviates the need to surround everything with "within ? value : 0".
  bool only_compute_math1_within;

  // For evaluating expressions.
  std::shared_ptr<VennVariables> variables;
  VennExpression math1_expressions[16];
  // VennDriver driver;
  // CompilationThread* compiler;
  // std::thread* compile_thread;
  // bool compile_in_progress = false;

  Venn() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(EXP_LIN_LOG_PARAM, -1.f, 1.f, 0.f, "Controls the speed that DISTANCE increases. At -1, it grows early, 0 is linear, 1 grows slowly.");
		configParam(X_POSITION_ATTN_PARAM, -1.0f, 1.0f, 0.0f, "Attenuverter for 2nd input to X position of Point");
		configParam(Y_POSITION_ATTN_PARAM, -1.0f, 1.0f, 0.0f, "Attenuverter for 2nd input to Y position of Point");

    configInput(X_POSITION_INPUT, "X position of Point - attenuated value of other input will be added");
    configInput(Y_POSITION_INPUT, "Y position of Point - attenuated value of other input will be added");
		configInput(X_POSITION_WIGGLE_INPUT, "Multiplied by attenuverter to alter the X position of Point");
		configInput(Y_POSITION_WIGGLE_INPUT, "Multiplied by attenuverter to alter the Y position of Point");

    configOutput(DISTANCE_OUTPUT, "0V at edge, 10V at center, polyphonic");
    configOutput(WITHIN_GATE_OUTPUT, "0V outside circle, 10V within, polyphonic");
		configOutput(X_POSITION_OUTPUT, "The current X coordinate of the point (-5V -> 5V). Useful for recording point gestures and performances.");
		configOutput(Y_POSITION_OUTPUT, "The current Y coordinate of the point (-5V -> 5V). Useful for recording point gestures and performances.");
		configOutput(X_DISTANCE_OUTPUT, "Within a circle, varies linearly from left to right, polyphonic");
		configOutput(Y_DISTANCE_OUTPUT, "Within a circle, varies linearly from bottom to top, polyphonic");
		configOutput(MATH1_OUTPUT, "Outputs values based on per-circle formulas, polyphonic");

    configSwitch(INV_X_PARAM, 0, 1, 0, "Invert",
                 {"Increases as point moves to the right", "Increases as point moves to the left"});
    configSwitch(INV_Y_PARAM, 0, 1, 0, "Invert",
                 {"Increases as point moves up", "Increases as point moves down"});
    configSwitch(INV_WITHIN_PARAM, 0, 1, 0, "Invert",
                 {"Increasing (outside = 0V, inside = 10V)", "Decreasing (outside = 10V, inside = 0V)"});
    configSwitch(OFST_X_PARAM, 0, 1, 0, "Offset",
                 {"Bipolar (-5V - +5V)", "Unipolar (0V - 10V)"});
    configSwitch(OFST_Y_PARAM, 0, 1, 0, "Offset",
                 {"Bipolar (-5V - +5V)", "Unipolar (0V - 10V)"});

    current_circle = -1;
    check_live_circles = 0;

    for (int i = 0; i < 16; ++i) {
      circles[i].present = false;  // Start empty; every circle is not present.
    }    

    circles_loaded = true;
    point.x = 0;
    point.y = 0;
    solo = false;
    editing = false;
    keystrokes_accepted = false;
    show_keyboard = true;  // For new instances, we are true.
    // TODO: could menu choice be a default for all instances?
    // Once user has learned it, it applies to all.
    only_compute_math1_within = true;

    // For compiling typed-in expressions.
    variables = std::make_shared<VennVariables>();
  /*
    compile_in_progress = false;
    compiler = new CompilationThread(&driver);
    compile_thread = new std::thread(&CompilationThread::Compile, compiler);
  */
  }
  
  ~Venn() {
    /*
    // A LOT of this would be better handled with shared_ptr.
    if (compiler) {
      compiler->Halt();
    }
    if (compile_thread) {
      compile_thread->join();
      delete compile_thread;
    }
    if (compiler) {
      delete compiler;
    }
    */
  }

  // Turns a set of shapes into a text string that we can parse later to recreate the shapes.
  std::string to_string(Circle the_circles[16]) {
    std::string result;
    // Don't need to store deleted circles with a larger index than the largest intact one.
    // So I'll just take this opportunity to erase any at the end.
    int last_index = -1;
    for (int curr = 16 - 1; curr >= 0; curr--) {
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

    json_object_set_new(rootJ, "show_keyboard", json_integer(show_keyboard ? 1 : 0));
    json_object_set_new(rootJ, "only_compute_math1_within",
        json_integer(only_compute_math1_within ? 1 :0));
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
      VennDriver driver(variables);
      if (driver.parse(diagram) != 0) {
        WARN("Compile Failure:\n%s", diagram.c_str());
      }
      ClearAllCircles();
      live_circle_count = std::min(16, (int) driver.diagram.circles.size()) + 1;
      for (int i = 0; i < (int) live_circle_count - 1; ++i) {
        circles[i] = driver.diagram.circles.at(i);
      }
      json_t* currentJ = json_object_get(rootJ, "current");
      if (currentJ) {
        current_circle = json_integer_value(currentJ);
      } else {
        current_circle = driver.diagram.circles.size() > 0 ? 0 : -1;
      }

      // So we've copied all the circles in place, but we haven't created the Expressions
      // yet. Now we can do that.
      for (int i = 0; i < (int) live_circle_count - 1; ++i) {
        // WARN("math1 = '%s'", circles[i].math1.c_str());
        if (!circles[i].math1.empty()) {
          if (driver.parse(circles[i].math1) == 0) {  // Compiles!
            math1_expressions[i] = driver.exp;
            //  WARN("Compile worked?");
            //  WARN("%s", math1_expressions[i].to_string().c_str());
          } /*
           else {
            if (driver.errors.size() > 0) {
              WARN("Compile Failed:\n%s", driver.errors[0].message.c_str());
            }
          } */
        }
      }
    }
    circles_loaded = true;
    update_text_widgets = true;

    json_t* keyboardJ = json_object_get(rootJ,  "show_keyboard");
    if (keyboardJ) {
      show_keyboard = json_integer_value(keyboardJ) > 0;
    }
    json_t* only_compute_math1_withinJ = json_object_get(rootJ,  "only_compute_math1_within");
    if (only_compute_math1_withinJ) {
      only_compute_math1_within = json_integer_value(only_compute_math1_withinJ) > 0;
    }
  }

  void ClearAllCircles() {
    for (int i = 0; i < 16; ++i) {
      circles[i].present = false;
    }
  }

  void onReset(const ResetEvent& e) override {
    Module::onReset(e);

    // A lock around the sudden change, so process() and UI doesn't fail.
    circles_loaded = false;
    ClearAllCircles();
    current_circle = -1;
    solo = false;
    human_point.x = 0.0;
    human_point.y = 0.0;
    circles_loaded = true;
    update_text_widgets = true;
  }

  float MyNormal() {
    return (random::uniform() + random::uniform()) / 2.0f; 
  }

  enum RandomMathGen {
    CONSTANT,
    NOTE,
    VARIABLE,
    BIN_OP,  // Should never be between two CONSTANTs
    BOOL_OP,
    BOOL,
    TERNERY,  // bool ? exp : exp
    SINGLE_ARG_FUNC,  // No need to demo two arg functions
    LIMIT,    // Good to demo.
    SCALE,    // Good to demo
    EXPRESSION
  };

  // Given a sorted list of points on (0, 1) and a matching list of RandomMathGen values,
  // pick a random number in (0, 1) and return the corresponding RandomMathGen.
  // The last element of dist[] must be 1.0, which marks the end of the array.
  // e.g. :
  // RandomPick({0.3, 0.5, 0.85, 1.0}, {CONSTANT, NOTE, VARIABLE, SINGLE_ARG_FUNC})
  // gives a 30% chance of CONSTANT, a 20% of NOTE, a 35% chance of VARIABLE,
  // and a 15% chance of SINGLE_ARG_FUNC.
  RandomMathGen RandomPick(float dist[], RandomMathGen choices[]) {
    float choice = random::uniform();
    int pick = 0;
    for (; dist[pick] < 0.99; pick++) {
      if (choice <= dist[pick]) {
        return choices[pick];
      }
    }
    return choices[pick];
  }

  // By no means does this produce the full range of possible expressions!
  // It's simply here to suggest people look at what this can do.
  std::string RandomMath(RandomMathGen choice) {
    switch (choice) {
      case CONSTANT: {
        char result[10];
        std::snprintf(result, 10, "%.2f", random::uniform() * 10 - 5.0);
        return result;
      }
      case NOTE: {
        std::string note(notes.substr((int) (random::uniform() * 7), 1));
        note.append(std::to_string((int) (random::uniform() * 4 + 2)));
        return note;
      }
      case VARIABLE: return VARIABLES[(int) (random::uniform() * VARIABLES_LEN)];
      case BIN_OP: {
        std::string result(RandomMath(VARIABLE));
        result.append(" ");
        result.append(simple_operators.substr((int) (random::uniform() * 4), 1));
        result.append(" ");
        result.append(RandomMath(CONSTANT));
        return result;      
      }
      case BOOL_OP: {
        std::string result(RandomMath(VARIABLE));
        result.append(" ");
        result.append(BOOL_OPS[(int) (random::uniform() * BOOL_OP_LEN)]);
        result.append(" ");
        result.append(RandomMath(CONSTANT));
        return result;      
      }
      case BOOL: {
        if (random::uniform() < 0.5) {
          return "within";
        } else {
          return RandomMath(BOOL_OP);
        }
      }
      case TERNERY: {
        std::string result(RandomMath(BOOL));
        result.append(" ? ");
        static float probs[] = {0.1, 0.2, 0.5, 0.9, 1.0};
        static RandomMathGen action[] = {NOTE, VARIABLE, BIN_OP, SINGLE_ARG_FUNC, CONSTANT};
        result.append(RandomMath(RandomPick(probs, action)));
        result.append(" : ");
        result.append(RandomMath(RandomPick(probs, action)));
        return result;
      }
      case SINGLE_ARG_FUNC: {
        std::string result(SINGLE_FUNCS[(int) (random::uniform() * SINGLE_ARG_LEN)]);
        result.append("(");
        result.append(RandomMath(BIN_OP));
        result.append(")");
        return result;
      }
      case LIMIT: {
        std::string result("limit(");
        static float probs[] = {0.2, 0.5, 1.0};
        static RandomMathGen action[] = {VARIABLE, BIN_OP, SINGLE_ARG_FUNC};
        result.append(RandomMath(RandomPick(probs, action)));
        result.append(", ");
        result.append(RandomMath(CONSTANT));
        result.append(", ");
        result.append(RandomMath(CONSTANT));
        result.append(")");
        return result;
      }
      case SCALE: {
        std::string result("scale(");
        static float probs[] = {0.2, 0.5, 1.0};
        static RandomMathGen action[] = {VARIABLE, BIN_OP, SINGLE_ARG_FUNC};
        result.append(RandomMath(RandomPick(probs, action)));
        result.append(", ");
        result.append(RandomMath(CONSTANT));
        result.append(", ");
        result.append(RandomMath(CONSTANT));
        result.append(", ");
        result.append(RandomMath(CONSTANT));
        result.append(", ");
        result.append(RandomMath(CONSTANT));
        result.append(")");
        return result;
      }
      case EXPRESSION: {
        static float probs[] = {0.1, 0.2, 0.3, 0.4, 0.7, 0.95, 1.0};
        static RandomMathGen result[] = {NOTE, TERNERY, LIMIT, SCALE, BIN_OP, SINGLE_ARG_FUNC, CONSTANT};
        return RandomMath(RandomPick(probs, result));
      }
      default:
        return "";
    }
  }

  void onRandomize(const RandomizeEvent& e) override {
    Module::onRandomize(e);

    // When User hits Randomize, let's make some circles.
    int count = clamp((int) (MyNormal() * 10.0 + 3), 3, 13);
    circles_loaded = false;
    current_circle = -1;
    solo = false;
    ClearAllCircles();

    std::shared_ptr<VennDriver> driver = std::make_shared<VennDriver>(variables);

    for (int i = 0; i < count; ++i) {
        Circle circle;
        circle.x_center = random::uniform() * 9.6 - 4.8;  
        circle.y_center = random::uniform() * 9.9 - 4.8;
        circle.radius = MyNormal() * 3 + .1;
        circle.present = true;
        // Random names are delightful.
        // I'll allow them to range in length from 1 to 3 words, just to see how they look.
        // Thanks to @disquiet for suggesting I add evocative color names.
        std::string the_name;
        if (random::uniform() > 0.5) {
          the_name.append(VENN_COLOR_ARRAY[i % VENN_COLOR_COUNT][(int) std::floor(random::uniform() * VENN_COLOR_NAMES)]);
          the_name.append(" ");
        }
        the_name.append(PARTS[(int) std::floor(random::uniform() * PART_LEN)]);
        if (random::uniform() > 0.5) {
          the_name.append(" ");
          the_name.append(EFFECTS[(int) (random::uniform() * sizeof(EFFECT_LEN))]);
        }
        circle.name = the_name;

        // Random MATH1 (demos what they can be, I hope).
        std::string random_math(RandomMath(EXPRESSION));
        if (driver->parse(random_math) == 0) {
          circle.math1 = random_math;
          math1_expressions[i] = driver->exp;
        }
        circles[i] = circle;
    }
    current_circle = 0;
    human_point.x = random::uniform() * 9.6 - 4.8;
    human_point.y = random::uniform() * 9.6 - 4.8;
    circles_loaded = true;
    update_text_widgets = true;
  }	

  void processBypass(const ProcessArgs& args) override {
    if (!circles_loaded) {
      return;
    }
    // We do this so you can continue editing the Surface.
    update_circle_count(args);

    outputs[DISTANCE_OUTPUT].setChannels(live_circle_count);
    outputs[WITHIN_GATE_OUTPUT].setChannels(live_circle_count);
    outputs[X_DISTANCE_OUTPUT].setChannels(live_circle_count);
    outputs[Y_DISTANCE_OUTPUT].setChannels(live_circle_count);

    for (size_t i = 0; i < live_circle_count; ++i) {
        outputs[DISTANCE_OUTPUT].setVoltage(0.0f, i);
        outputs[WITHIN_GATE_OUTPUT].setVoltage(0.0f, i);
        outputs[X_DISTANCE_OUTPUT].setVoltage(0.0f, i);
        outputs[Y_DISTANCE_OUTPUT].setVoltage(0.0f, i);
    }
  }

  void update_circle_count(const ProcessArgs& args) {
    if (--check_live_circles <= 0) {
      // One sixtieth of a second.
      check_live_circles = (int) (args.sampleRate / 60);
      // We occasionally check how many circles are marked present.
      // This allows us to set the channel count correctly, and reduce the overhead of
      // circles that don't exist.
      live_circle_count = 0;
      for (size_t channel = 0; channel < 16; channel++) {
        if (circles[channel].present) {
          live_circle_count = channel + 1;
        }
      }
    }
  }

  void process(const ProcessArgs& args) override {
    if (!circles_loaded) {
      return;
    }
    update_circle_count(args);
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

    if (params[X_POSITION_ATTN_PARAM].getValue() != 0.0f) {
      point.x += params[X_POSITION_ATTN_PARAM].getValue() *
        inputs[X_POSITION_WIGGLE_INPUT].getVoltage();
    }
    if (params[Y_POSITION_ATTN_PARAM].getValue() != 0.0f) {
      point.y += params[Y_POSITION_ATTN_PARAM].getValue() *
        inputs[Y_POSITION_WIGGLE_INPUT].getVoltage();
    }

    // Keep within walls.
    point.x = fmax(-5, fmin(5, point.x));
    point.y = fmax(-5, fmin(5, point.y));

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
    outputs[MATH1_OUTPUT].setChannels(live_circle_count);

    float exp_lin_log = params[EXP_LIN_LOG_PARAM].getValue();
    float scaling = 1.0;
    // Need to compute distance correctly if DISTANCE or MATH1 is connected.
    if ((outputs[DISTANCE_OUTPUT].isConnected()) ||
        (outputs[MATH1_OUTPUT].isConnected())) {
      // If we are from -1 - 0, we want to scale it to 0.1 - 1.
      if (exp_lin_log != 0.0) {
        scaling = rack::math::rescale(-1 * fabs(exp_lin_log), -1, 0, .1, 1);
        if (exp_lin_log > 0.0) {
          scaling = 1.0 / scaling;
        }
      }
    }
    bool invert_gate = params[INV_WITHIN_PARAM].getValue();
    bool offset_x = params[OFST_X_PARAM].getValue();
    bool invert_x = params[INV_X_PARAM].getValue();
    bool offset_y = params[OFST_Y_PARAM].getValue();
    bool invert_y = params[INV_Y_PARAM].getValue();
    if (outputs[MATH1_OUTPUT].isConnected()) {
      // These can only matter if we are computing MATH1 outputs.
      *variables->GetVarFromEnum(VennVariables::VAR_POINTX) = point.x;
      *variables->GetVarFromEnum(VennVariables::VAR_POINTY) = point.y;
      *variables->GetVarFromEnum(VennVariables::VAR_LEFTX) =  -5 * (invert_x ? -1 : 1) + (offset_x ? 5.0 : 0.0);
      *variables->GetVarFromEnum(VennVariables::VAR_RIGHTX) = 5 * (invert_x ? -1 : 1) + (offset_x ? 5.0 : 0.0);
      *variables->GetVarFromEnum(VennVariables::VAR_TOPY) = 5 * (invert_y ? -1 : 1) + (offset_y ? 5.0 : 0.0);
      *variables->GetVarFromEnum(VennVariables::VAR_BOTTOMY) = -5 * (invert_y ? -1 : 1) + (offset_y ? 5.0 : 0.0);
    }
    // Iterate through the circles.
    for (size_t channel = 0; channel < live_circle_count; channel++) {
      const Circle& circle = circles[channel];
      float x = 0.0f, y = 0.0f, within = 0.0f, distance = 0.0f, math1 = 0.0f;
      bool within_state = false;

      // If solo-ing, make sure that only solo channel gets computed.
      if (circle.present && (!solo || (int) channel == current_circle)) {
        // All of the outputs care if we are in the circle or not, so we always compute it,
        // regardless of there is a cable connected or not.
        float x_distance = point.x - circle.x_center;
        float y_distance = point.y - circle.y_center;

        // Despite my intution, sqrt() is probably cheap enough now to not try and replace it
        // with an approximation.
        float distance_actual = sqrt(x_distance * x_distance + y_distance * y_distance);
        if (distance_actual > circle.radius) {
          within = invert_gate ? 10.0f : 0.0f;
        } else {
          within_state = true;
          // Compute the values of within, distance, x, and y.
          within = invert_gate ? 0.0f : 10.0f;

          float value = (1 - distance_actual / circle.radius);
          // Since 1.0 is the default, skip the call to pow() if not needed.
          if (scaling != 1.0) {
            value = pow(value, scaling);
          }
          distance = value * 10;            

          x = x_distance / circle.radius * 5.0 * (invert_x ? -1.0 : 1.0) + (offset_x ? 5.0 : 0.0);
          y = y_distance / circle.radius * 5.0 * (invert_y ? -1.0 : 1.0) + (offset_y ? 5.0 : 0.0);
        }
      }
      outputs[DISTANCE_OUTPUT].setVoltage(distance, channel);
      outputs[WITHIN_GATE_OUTPUT].setVoltage(within, channel);
      outputs[X_DISTANCE_OUTPUT].setVoltage(x, channel);
      outputs[Y_DISTANCE_OUTPUT].setVoltage(y, channel);
      if (outputs[MATH1_OUTPUT].isConnected()) {
        // If solo-ing, make sure that only solo channel gets computed.
        if (circle.present && (!solo || (int) channel == current_circle)) {
          if (within_state || !only_compute_math1_within) {
            *variables->GetVarFromEnum(VennVariables::VAR_DISTANCE) = distance;
            *variables->GetVarFromEnum(VennVariables::VAR_WITHIN) = within;
            *variables->GetVarFromEnum(VennVariables::VAR_X) = x;
            *variables->GetVarFromEnum(VennVariables::VAR_Y) = y;
            math1 = math1_expressions[channel].Compute();
          }
        }
        outputs[MATH1_OUTPUT].setVoltage(math1, channel);
      }
    }

    // Lights.
    lights[INV_WITHIN_LIGHT].setBrightness(params[INV_WITHIN_PARAM].getValue());
    lights[INV_X_LIGHT].setBrightness(params[INV_X_PARAM].getValue());
    lights[INV_Y_LIGHT].setBrightness(params[INV_Y_PARAM].getValue());
    lights[OFST_X_LIGHT].setBrightness(params[OFST_X_PARAM].getValue());
    lights[OFST_Y_LIGHT].setBrightness(params[OFST_Y_PARAM].getValue());
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
      // Need to tell state-holding widgets to update. But since the module doesn't
      // hold pointers to the widgets, we can only set a flag.
      module->update_text_widgets = true;

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
        break;
      }
      // Need to tell state-holding widgets to update. But since the module doesn't
      // hold pointers to the widgets, we can only set a flag.
      module->update_text_widgets = true;

      module->circles_loaded = true;
    }
  }
};

// Forward Declaration, so the text fields can point to them.
struct VennNameTextField;
struct VennMath1TextField;

struct WidgetUpdater {
  Venn* module;
  VennNameTextField* name_widget = nullptr;
  VennMath1TextField* math1_widget = nullptr;

  void UpdateWidgets();
};

/*
Principles for the UI:
* There are these states for the editor:
  Not Engaged
  Keystrokes will not affect the editor, and no circle is currently selected (?).
  This is the case when other modules have focus.

  Circle editing
  Keystrokes will affect the position/number of circles. This is implemented by each of
  the widgets that can allow this to send keystrokes (onSelectKey() calls) to one
  single function, possibly one in Venn module?

  Editing current circle
  Keystrokes are being used by the focus widget, and are not sent to the circle editor.

* It should be visually clear to the user which state you are in.
  Currently this is signified by the reveal of the keyboard hint,
  but there should be at least one other way, since i think the keyboard hint
  should be optional (as it adds visual noise to the editor). And it's really not
  sufficient with three states.
* The main Widget will need to cycle through all of the subwidgets to see if they
  have focus. My previous idea of having each widget check in step() won't work,
  because no Widget changes to Not Engaged.

*/

struct VennNameTextField : STTextField {
  Venn* module;
  std::string venn_text;
  WidgetUpdater* widget_updater;

  VennNameTextField() {
    module = nullptr;
    this->text = &venn_text;
    fontPath = asset::plugin(pluginInstance, "fonts/RobotoSlab-Regular.ttf");
    fontSize = 12.0f;
    color = SCHEME_WHITE;
    bgColor = SCHEME_BLACK;
    textOffset = math::Vec(0, -2);  // Put closer to corner than default.
    extended.Initialize(3, 1);  // Much shorter window.
  }

  void setModule(Venn* the_module) {
    module = the_module;
  }

  // bgColor seems to have no effect if I don't do this. Drawing a background
  // and then letting STTextField draw the rest fixes that.
  // TODO: Make STTextField actually use bgColor. Or draw background color on templates.
  void draw(const DrawArgs& args) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));

    // background only
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(args.vg, bgColor);
    nvgFill(args.vg);

    if (module && module->editing && module->current_circle >= 0) {
      STTextField::draw(args);  // Draw text.
    }
    nvgResetScissor(args.vg);
  }

    // There are a few keys that we want to accept here, rather than in the STTextField.
    void onSelectKey(const SelectKeyEvent& e) override {
      if (!module->circles_loaded) {
        // Don't edit circles if we're in the middle of loading them!
        return;
      }
      bool my_key = false;  // Set if key was intended for this layer of key press interpretation.
      if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) { 
        // TAB -> next
        if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == 0) {
          if (module->live_circle_count > 0) {
            for (int curr = module->current_circle + 1; curr != module->current_circle; curr++) {
              if (curr >= 16) {
                curr = 0;
              }
              if (module->circles[curr].present) {
                module->current_circle = curr;
                widget_updater->UpdateWidgets();
                break;
              }
            }
          }
          my_key = true;
          e.consume(this);
        }
        // SHIFT-TAB -> prev
        if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
          if (module->live_circle_count > 0) {
            for (int curr = module->current_circle - 1; curr != module->current_circle; curr--) {
              if (curr < 0) {
                curr = 16 - 1;
              }
              if (module->circles[curr].present) {
                module->current_circle = curr;
                widget_updater->UpdateWidgets();
                break;
              }
            }
          }
          my_key = true;
          e.consume(this);
        }
      }
      if (!my_key) {
        STTextField::onSelectKey(e);
      }
    }  

  void CircleUpdated(const std::string& name) {
    text->assign(name);
  }

  std::string getText() {
    return *text;
  }

  void setText(const std::string& new_text) {
    text->assign(new_text);
    // TODO: probably need to call updatedText in STTextField?
  }
};

struct VennErrorWidget;
struct VennErrorTooltip : ui::Tooltip {
  VennErrorWidget* errorWidget;
  std::string error_text;

  VennErrorTooltip(const std::string &text) : error_text{text} {}

  void step() override;
};

struct VennErrorWidget : widget::OpaqueWidget {
  std::shared_ptr<VennDriver> driver;
  VennErrorTooltip* tooltip;

  VennErrorWidget() {
    tooltip = NULL;
  }

  void setDriver(std::shared_ptr<VennDriver> the_driver) {
    driver = the_driver;
  }

  void onEnter(const EnterEvent & e) override {
    create_tooltip();
  }

  void onLeave(const LeaveEvent & e) override {
    destroy_tooltip();
  }

  void create_tooltip() {
    if (!settings::tooltips)
      return;
    if (tooltip)  // Already exists.
      return;
    if (!driver)
      return;
    std::string tip_text;
    if (driver->errors.size() == 0) {
      tip_text = "Program compiles!";
    } else {
      Error err = driver->errors[0];
      // Remove "syntax error, " from message.
      std::string msg = err.message;
      if (msg.rfind("syntax error, ", 0) == 0) {
        msg = msg.substr(14);
      }
      tip_text = msg;
    }
    VennErrorTooltip* new_tooltip = new VennErrorTooltip(tip_text);
    new_tooltip->errorWidget = this;
    APP->scene->addChild(new_tooltip);
    tooltip = new_tooltip;
  }

  void destroy_tooltip() {
    if (!tooltip)
      return;
    APP->scene->removeChild(tooltip);
    delete tooltip;
    tooltip = NULL;
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();
      bool good = (driver) ? driver->errors.size() == 0 : true;
      // Fill the rectangle with either blue or orange.
      // For color blind users, these are better choices than green/red.
      NVGcolor main_color =
          (good ? SCHEME_GREEN : color::RED);
      nvgBeginPath(args.vg);
      nvgRect(args.vg, 0.5, 0.5,
              bounding_box.x - 1.0f, bounding_box.y - 1.0f);
      nvgFillColor(args.vg, main_color);
      nvgFill(args.vg);
      std::string fontPath;
      fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (font) {
        nvgFillColor(args.vg,
           (good ? color::BLACK : color::WHITE));
        nvgFontSize(args.vg, 13);
        nvgTextAlign(args.vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -1);

        std::string text = (good ? "Good" : "Fix");
        // Place on the line just off the left edge.
        nvgText(args.vg, bounding_box.x / 2, 0, text.c_str(), NULL);
      }
    }
    Widget::drawLayer(args, layer);
  }
};

void VennErrorTooltip::step() {
  text = error_text;
  Tooltip::step();
  // Position at bottom-right of parameter
  box.pos = errorWidget->getAbsoluteOffset(errorWidget->box.size).round();
  // Fit inside parent (copied from Tooltip.cpp)
  assert(parent);
  box = box.nudge(parent->box.zeroPos());
}

struct VennMath1TextField : STTextField {
  Venn* module;
  std::string math1_text;
  std::shared_ptr<VennDriver> driver;
  WidgetUpdater* widget_updater;

  VennMath1TextField() {
    module = nullptr;
    this->text = &math1_text;
    fontPath = asset::plugin(pluginInstance, "fonts/RobotoSlab-Regular.ttf");
    fontSize = 12.0f;
    color = SCHEME_WHITE;
    bgColor = SCHEME_BLACK;

    textOffset = math::Vec(0, -2);  // Put closer to corner than default.

    extended.Initialize(3, 1);  // Much shorter window.
  }

  void setModule(Venn* the_module, VennErrorWidget* error_widget) {
    module = the_module;
    if (module) {
      driver = std::make_shared<VennDriver>(module->variables);
      error_widget->setDriver(driver);
    }
  }

  // There are a few keys that we want to accept here, rather than in the STTextField.
  void onSelectKey(const SelectKeyEvent& e) override {
    if (!module->circles_loaded) {
      // Don't edit circles if we're in the middle of loading them!
      return;
    }
    bool my_key = false;  // Set if key was intended for this layer of key press interpretation.
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) { 
      // TAB -> next
      if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == 0) {
        if (module->live_circle_count > 0) {
          for (int curr = module->current_circle + 1; curr != module->current_circle; curr++) {
            if (curr >= 16) {
              curr = 0;
            }
            if (module->circles[curr].present) {
              module->current_circle = curr;
              widget_updater->UpdateWidgets();
              break;
            }
          }
        }
        my_key = true;
        e.consume(this);
      }
      // SHIFT-TAB-> prev
      if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
        if (module->live_circle_count > 0) {
          for (int curr = module->current_circle - 1; curr != module->current_circle; curr--) {
            if (curr < 0) {
              curr = 16 - 1;
            }
            if (module->circles[curr].present) {
              module->current_circle = curr;
              widget_updater->UpdateWidgets();
              break;
            }
          }
        }
        my_key = true;
        e.consume(this);
      }
    }
    if (!my_key) {
      STTextField::onSelectKey(e);
    }
  }

  // bgColor seems to have no effect if I don't do this. Drawing a background
  // and then letting STTextField draw the rest fixes that.
  // TODO: Make STTextField actually use bgColor. Or draw background color on templates.
  void draw(const DrawArgs& args) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));

    // background only
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(args.vg, bgColor);
    nvgFill(args.vg);

    if (module && module->editing && module->current_circle >= 0) {
      STTextField::draw(args);  // Draw text.
    }
    nvgResetScissor(args.vg);
  }

  void CircleUpdated(const std::string& name) {
    math1_text.assign(name);
  }

  std::string getText() {
    return *text;
  }

  void setText(const std::string& new_text) {
    bool equal = text->compare(new_text) == 0;
    text->assign(new_text);
    if (!equal) {
      // if the text has changed, then I should call onChange() to recompile.
      ChangeEvent dummy;
      onChange(dummy);
    }

    // TODO: probably need to call updatedText in STTextField?
  }
  
  // User has updated the text.
  // NOTA BENE: This is a risky idea, but I'm going to try doing the compile
  // inside the UI thread here.
  // TODO: as a test to see how bad an idea this is, I could pause for a bit here instead.
  // Just do some pointless effort for a while.
  void onChange(const ChangeEvent& e) override {
    if (module) {
      // Sometimes the text isn't actually different? If I don't check
      // this, I might get spurious history events.
      // TODO: do I need to actually check that the string has really changed anymore?


      //auto start = std::chrono::high_resolution_clock::now();

      if (math1_text.empty()) {
        if (!(driver->errors.empty())) {
          // So that if user removes all text, the error widget reads "Good".
          driver->errors.clear();
        }
        VennExpression inactive;
        module->math1_expressions[module->current_circle] = inactive;
      } else {
        if (driver->parse(math1_text) == 0) {
          // Success.
          module->math1_expressions[module->current_circle] = driver->exp;
        } else {



          // TODO: DO NOT SUBMIT!
          // WARN("Failed to compile '%s'", math1_text.c_str());


        }
      }
      // auto elapsed = std::chrono::high_resolution_clock::now() - start;
      // WARN("Compiling\n%s\ntook %lld micros", math1_text.c_str(),
      //     std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
    }
  }

};

void WidgetUpdater::UpdateWidgets() {
  if (!name_widget) {
    WARN("name_widget not set.");
    return;
  }
  if (!math1_widget) {
    WARN("math1_widget not set.");
    return;
  }
  if (module && module->current_circle >= 0) {
    name_widget->CircleUpdated(module->circles[module->current_circle].name);
    math1_widget->CircleUpdated(module->circles[module->current_circle].math1);
  } else {
    name_widget->CircleUpdated("");
    math1_widget->CircleUpdated("");
  }
}    

struct CircleDisplay : OpaqueWidget {
  Venn* module;
  Vec last_hover_pos;
  WidgetUpdater* widget_updater;

  CircleDisplay() {}

  // Move point to current location if left clicked.
  void onButton(const ButtonEvent& e) override {
    OpaqueWidget::onButton(e);

    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
      // Must change position in widget to voltage.
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();
      module->human_point.x = e.pos.x / bounding_box.x * 10.0 - 5;
      module->human_point.y = (1 - (e.pos.y / bounding_box.y)) * 10.0 - 5;
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
        module->human_point.x = e.pos.x / bounding_box.x * 10.0 - 5;
        module->human_point.y = (1 - (e.pos.y / bounding_box.y)) * 10.0 - 5;
        e.consume(this);
    }
  }

  void RememberChange(const Circle& old_circle) {
    APP->history->push(
      new VennCircleUndoRedoAction(module->id, old_circle,
                                   module->circles[module->current_circle],
                                   module->current_circle));
  }

  void onHover(const HoverEvent&	e) override {
    if (module && module->keystrokes_accepted) {
      // In case the user presses "f" to create a new circle, we note the current position.
      last_hover_pos = e.pos;
      // I think we don't want to consume this event.
      OpaqueWidget::onHover(e);
    }
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
          Circle old(module->circles[module->current_circle]);
          module->circles[module->current_circle].y_center += 0.1;
          RememberChange(old);
          e.consume(this);
        }
        // S - down
        if (e.keyName == "s" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles[module->current_circle]);
          module->circles[module->current_circle].y_center -= 0.1;
          RememberChange(old);
          e.consume(this);
        }
        // A - left
        if (e.keyName == "a" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles[module->current_circle]);
          module->circles[module->current_circle].x_center -= 0.1;
          RememberChange(old);
          e.consume(this);
        }
        // D - right
        if (e.keyName == "d" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles[module->current_circle]);
          module->circles[module->current_circle].x_center += 0.1;
          RememberChange(old);
          e.consume(this);
        }
        // Q - smaller
        if (e.keyName == "q" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles[module->current_circle]);
          module->circles[module->current_circle].radius -= 0.1;
          if (module->circles[module->current_circle].radius < 0.1) {
            module->circles[module->current_circle].radius = 0.1;
          }
          RememberChange(old);
          e.consume(this);
        }
        // E - bigger
        if (e.keyName == "e" && (e.mods & RACK_MOD_CTRL) == 0) {
          Circle old(module->circles[module->current_circle]);
          module->circles[module->current_circle].radius += 0.1;
          RememberChange(old);
          e.consume(this);
        }
      }
      // Selecting which Circle.
      // Z or SHIFT-TAB - previous
      if ((e.keyName == "z" && (e.mods & RACK_MOD_CTRL) == 0) || 
          (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT)) {
        if (module->live_circle_count > 0) {
          for (int curr = module->current_circle - 1; curr != module->current_circle; curr--) {
            if (curr < 0) {
              curr = 16 - 1;
            }
            if (module->circles[curr].present) {
              module->current_circle = curr;
              widget_updater->UpdateWidgets();
              break;
            }
          }
        }
        e.consume(this);
      }
      // C or TAB - next
      if ((e.keyName == "c" && (e.mods & RACK_MOD_CTRL) == 0) ||
          (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == 0)) {
        if (module->live_circle_count > 0) {
          for (int curr = module->current_circle + 1; curr != module->current_circle; curr++) {
            if (curr >= 16) {
              curr = 0;
            }
            if (module->circles[curr].present) {
              module->current_circle = curr;
              widget_updater->UpdateWidgets();
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
        // Center on where mouse is (if available).
        Rect r = box.zeroPos();
        Vec bounding_box = r.getBottomRight();
        float x = last_hover_pos.x / bounding_box.x * 10.0 - 5;
        float y = (1 - (last_hover_pos.y / bounding_box.y)) * 10.0 - 5;
        if (fabs(x) < 5.0 && fabs(y) < 5.0) {
          circle.x_center = x;  
          circle.y_center = y;
        } else {
          circle.x_center = random::uniform() * 2 - 1;  
          circle.y_center = random::uniform() * 2 - 1;
        }

        circle.radius = 1.0 + random::uniform();
        circle.present = true;
        // What position should this be in?
        // Let's start with the first non-present slot.
        // Limit of 16 channels in a cable, so won't add after that.
        bool added = false;
        for (int curr = 0; curr < 16; curr++) {
          if (!(module->circles[curr].present)) {
            module->circles[curr] = circle;
            module->current_circle = curr;
            widget_updater->UpdateWidgets();
            added = true;
            break;
          }
        }
        if (added) {
          APP->history->push(
            new VennCircleUndoRedoAction(module->id, module->circles[module->current_circle],
                                         old_index, module->current_circle, true));
        }
        e.consume(this);
      }

      // x - delete current one.
      if (e.keyName == "x" && (e.mods & RACK_MOD_CTRL) == 0) {
        if (module->current_circle >= 0) {  // i.e., there is a circle to delete.
          // Copy circle *before* "present" is set to false!
          Circle old_circle(module->circles[module->current_circle]);
          int old_index = module->current_circle;
          module->circles[module->current_circle].present = false;
          // Move focus to next circle, if any.
          bool found_next = false;
          for (int curr = module->current_circle + 1; curr != module->current_circle; curr++) {
            if (curr >= 16) {
              curr = 0;
            }
            if (module->circles[curr].present) {
              module->current_circle = curr;
              widget_updater->UpdateWidgets();
              found_next = true;
              break;
            }
          }
          if (!found_next) {
            module->current_circle = -1;  // Indicate there is no currently selected circle.
            widget_updater->UpdateWidgets();
          }
          APP->history->push(
            new VennCircleUndoRedoAction(module->id, old_circle,
                                         old_index, module->current_circle, false));
        }
        e.consume(this);
      }

      // Turn solo on or off.
      if (e.keyName == "r" && (e.mods & RACK_MOD_CTRL) == 0) {
        if (module->current_circle >= 0) {  // i.e., there is a circle to solo.
          // Set or unset the solo_channel.
          module->solo = !(module->solo);
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

  void default_circles(Circle circs[16]) {
    float x[6] = {0.062705, -2.653372, 2.517570, 3.260226, -2.121736, 0.974539};
    float y[6] = {-0.411569, 2.684878, 4.339533, -0.208806, -2.508809, -4.108777};
    float radius[6] = {1.742869, 4.004038, 2.987639, 2.559288, 2.169408, 3.424444};
    const char * names[6] = {"Delay", "Melody Speed", "Melody Crunch", "Reverb Space",
                             "Noise Pad", "Float\nSparkle"};
    for (int i = 0; i < 6; ++i) {
      Circle circle;
      circle.x_center = x[i];
      circle.y_center = y[i];
      circle.radius = radius[i];
      circle.present = true;
      circle.name = names[i];
      circs[i] = circle;

    }
  }

  // By using drawLayer() instead of draw(), this becomes a glowing display
  // when the "room lights" are turned down. That seems correct to me.
  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      Circle dummy[16];
      Circle *circles;
      int circle_count;
      int current_circle;
      bool solo; 
      Vec point;
      bool currently_editing;
      bool currently_keyboard;
      bool show_full_keyboard;
      if (module) {
        // If we have a module, but the circles are being updated, best not to draw anything.
        // TODO: reconsider this decision, since we no longer have the issue of out-of-range indexies.
        if (!module->circles_loaded) {
          return;
        } else {
          circles = module->circles;
          circle_count = module->live_circle_count;
          current_circle = module->current_circle;
          solo = module->solo;
          point = module->point;
          currently_editing = module->editing;
          currently_keyboard = module->keystrokes_accepted;
          show_full_keyboard = module->show_keyboard;
        }
      } else {
        // Simple demo values to show in the browser and library page.
        default_circles(dummy);
        circles = dummy;
        circle_count = 6;
        current_circle = 2;
        solo = false;
        point.x = 0.0;
        point.y = 0.2345;
        currently_editing = false;
        currently_keyboard = false;
        show_full_keyboard = false;
      }
      nvgScissor(args.vg, RECT_ARGS(args.clipBox));
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();
      // Assuming that we are always on a square-pixeled surface, with X and Y the same distances.
      double pixels_per_volt = bounding_box.x / 10.0;

      // Background first.
      // The background (with keyboard hints) is in the SVG template. We just hide it when
      // keyboard input isn't accepted.
      if (!currently_keyboard || !show_full_keyboard) {
        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0.0, 0.0, bounding_box.x, bounding_box.y);
        nvgFillColor(args.vg, SCHEME_DARK_GRAY);
        nvgFill(args.vg);
      }

      // The circles.
      std::shared_ptr<Font> font = APP->window->loadFont(
        asset::plugin(pluginInstance, "fonts/RobotoSlab-Regular.ttf"));

      for (int index = 0; index < circle_count; index++) {
        const Circle& circle = circles[index];
        if (circle.present) {
          // Draw the circle itself.
          nvgBeginPath(args.vg);
          nvgCircle(args.vg, nvg_x(circle.x_center, bounding_box.x), nvg_y(circle.y_center, bounding_box.x),
                  pixels_per_volt * circle.radius);
          NVGcolor circle_color = venn_colors[index % VENN_COLOR_COUNT];
          if (solo && current_circle != index) {
            // Dim the muted circles.
            circle_color = nvgTransRGBAf(circle_color, 0.3);
          }
          nvgStrokeColor(args.vg, circle_color);
          nvgStrokeWidth(args.vg, index == current_circle && currently_editing ? 2.0 : 1.0);
          nvgStroke(args.vg);
          
          // Now draw the text in the center.
          nvgFillColor(args.vg, venn_colors[index % VENN_COLOR_COUNT]);
          nvgFontSize(args.vg, index == current_circle && currently_editing ? 15 : 13);
          nvgFontFaceId(args.vg, font->handle);
          // Place in the center.
          std::string center_number = std::to_string(index + 1);
          nvgTextAlign(args.vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
          nvgText(args.vg, nvg_x(circle.x_center, bounding_box.x),
                          nvg_y(circle.y_center, bounding_box.x),
                          center_number.c_str(), NULL);
          // If we can find some text to put in it, we can add a name.
          std::string name(circle.name);
          if (name.empty()) {
            name = circle.math1;
          } 
          if (!name.empty()) {
            nvgTextAlign(args.vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
            // Break name into lines by newlines.
            std::vector<std::string> lines;
            // Break this into words. Since I don't know the font,
            // too much effort to predict length, and don't want to use a
            // TextField.
            // TODO: cache this computation by moving this elsewhere? This
            // section only takes about 5 usec to run.
            auto start = 0U;
            auto end = name.find('\n');
            while (end != std::string::npos) {
              lines.push_back(name.substr(start, end - start));
              start = end + 1;
              end = name.find('\n', start);
            }
            std::string last = name.substr(start);
            lines.push_back(last);
            for (int i = 0; i < (int) lines.size(); i++) {
              nvgText(args.vg, nvg_x(circle.x_center, bounding_box.x),
                      nvg_y(circle.y_center, bounding_box.x) + 5.0 + i * 10.0, lines[i].c_str(), NULL);
            }
          }
        }
      }

      // Draw the Point.
      nvgBeginPath(args.vg);
      nvgCircle(args.vg, nvg_x(point.x, bounding_box.x), nvg_y(point.y, bounding_box.x),
              pixels_per_volt * 0.15);
      nvgStrokeColor(args.vg, SCHEME_WHITE);
      nvgStrokeWidth(args.vg, 1.0);
      nvgStroke(args.vg);

      OpaqueWidget::draw(args);
      nvgResetScissor(args.vg);
    }
	}
};

// Just the tiny window showing which circle is currently selected, if any.
struct VennNumberDisplayWidget : TransparentWidget {
  Venn* module;

  VennNumberDisplayWidget() {
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));
    if (layer == 1) {
      // No background color!

      if (module) {
        std::shared_ptr<Font> font = APP->window->loadFont(
            asset::plugin(pluginInstance, "fonts/RobotoSlab-Regular.ttf"));

        if (font) {
          if (module->editing && module->current_circle >= 0) {
            std::string text(std::to_string(module->current_circle + 1));
            Rect r = box.zeroPos();
            Vec bounding_box = r.getBottomRight();
            nvgFillColor(args.vg, settings::preferDarkPanels ? color::WHITE :
                                                               color::BLACK);
            nvgFontSize(args.vg, 28);
            nvgTextAlign(args.vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
            nvgFontFaceId(args.vg, font->handle);
            nvgTextLetterSpacing(args.vg, -1);
            nvgText(args.vg, bounding_box.x / 2, 0, text.c_str(), NULL);
          } else {
            nvgText(args.vg, 0, 0, "", NULL);  // Clear it.
          }
        }
      }
    }
    TransparentWidget::drawLayer(args, layer);
    nvgResetScissor(args.vg);
  }
};

struct VennKeyboardIcon : SvgWidget {
  Venn* module;

  VennKeyboardIcon() {
    box.size = mm2px(Vec(8.0, 6.0));
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Venn-key-icon.svg")));
  }

  void draw(const DrawArgs& args) override {
    // Only show the icon under certain conditions.
    if (module && module->keystrokes_accepted && !(module->show_keyboard)) {
      SvgWidget::draw(args);
    }
  }
};

struct VennWidget : ModuleWidget {
  CircleDisplay* display;
  VennNameTextField* name_field;
  VennMath1TextField* math1_field;
  WidgetUpdater widget_updater;

  VennWidget(Venn* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Venn.svg"),
                         asset::plugin(pluginInstance, "res/Venn-dark.svg")));

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
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(178.17, 92.963)), module, Venn::MATH1_OUTPUT));

    addParam(createLightParamCentered<VCVLightLatch<
          MediumSimpleLight<WhiteLight>>>(mm2px(Vec(166.582, 28.78)), module, Venn::INV_WITHIN_PARAM, Venn::INV_WITHIN_LIGHT));
    addParam(createLightParamCentered<VCVLightLatch<
          MediumSimpleLight<WhiteLight>>>(mm2px(Vec(166.582, 64.673)), module, Venn::INV_X_PARAM, Venn::INV_X_LIGHT));
    addParam(createLightParamCentered<VCVLightLatch<
          MediumSimpleLight<WhiteLight>>>(mm2px(Vec(178.17, 64.673)), module, Venn::INV_Y_PARAM, Venn::INV_Y_LIGHT));
    addParam(createLightParamCentered<VCVLightLatch<
          MediumSimpleLight<WhiteLight>>>(mm2px(Vec(166.582, 77.655)), module, Venn::OFST_X_PARAM, Venn::OFST_X_LIGHT));
    addParam(createLightParamCentered<VCVLightLatch<
          MediumSimpleLight<WhiteLight>>>(mm2px(Vec(178.17, 77.655)), module, Venn::OFST_Y_PARAM, Venn::OFST_Y_LIGHT));

    widget_updater.module = module;
    
    // Information about the currently selected circle.
    // Lining up vertically with the black box around the X/Y outputs.
    VennNumberDisplayWidget* number = createWidget<VennNumberDisplayWidget>(mm2px(Vec(10.0, 58.6)));
    // Decided by seeing how big "16" is in Inkscape, although not in the right font, so...wrongish.
    number->box.size = mm2px(Vec(11.0, 8.0));
    number->module = module;
    addChild(number);

    name_field = createWidget<VennNameTextField>(mm2px(Vec(2.240, 71.0)));
    name_field->box.size = mm2px(Vec(26.0, 10.0));
    name_field->setModule(module);
    addChild(name_field);
    widget_updater.name_widget = name_field;

    // Compilation status and error message access.
    VennErrorWidget* math1_error_display = createWidget<VennErrorWidget>(mm2px(
        Vec(20.4, 81.0)));
    math1_error_display->box.size = mm2px(Vec(8.0, 4.0));
    addChild(math1_error_display);

    math1_field = createWidget<VennMath1TextField>(mm2px(Vec(2.240, 85.0)));
    math1_field->box.size = mm2px(Vec(26.0, 10.0));
    math1_field->setModule(module, math1_error_display);
    addChild(math1_field);
    widget_updater.math1_widget = math1_field;

    // Allows pressing TAB or SHIFT-TAB in the name or math1 text fields to
    // rotate through the circles.
    name_field->widget_updater = &widget_updater;
    math1_field->widget_updater = &widget_updater;

    // The Circles.
    display = createWidget<CircleDisplay>(
      mm2px(Vec(31.0, 1.7)));
    display->box.size = mm2px(Vec(125.0, 125.0));
    display->module = module;
    display->widget_updater = &widget_updater;
    addChild(display);

    VennKeyboardIcon* icon = createWidget<VennKeyboardIcon>(mm2px(Vec(1.0, 1.0)));
    icon->module = module;
    display->addChild(icon);  // Ensures it's drawn on top of CircleDisplay.
  }

  void step() override {
    ModuleWidget::step();
    if (module) {
      Venn* module = dynamic_cast<Venn*>(this->module);
      if (module->update_text_widgets) {
        // The underlying circle has changed from the module thread.
        // Reset the string *_field's.
        module->update_text_widgets = false;
        if (module->current_circle >= 0) {
          name_field->setText(module->circles[module->current_circle].name);
          math1_field->setText(module->circles[module->current_circle].math1);
        }
      }
      
      // Determine state of editing.
      Widget* selected = APP->event->selectedWidget;
      if (selected == this || selected == display) {
        module->editing = true;
        module->keystrokes_accepted = true;
      } else if (selected == name_field) {
        module->editing = true;
        module->keystrokes_accepted = false;  // Text input field needs the keystrokes.

        // Update name, if changed.
        if (module->current_circle >= 0) {
          if (module->circles[module->current_circle].name.compare(name_field->getText()) != 0) {
            Circle old_circle = module->circles[module->current_circle];
            module->circles[module->current_circle].name = name_field->getText();
            APP->history->push(
                new VennCircleUndoRedoAction(module->id, old_circle,
                                             module->circles[module->current_circle],
                                             module->current_circle));
          }
        }
      } else if (selected == math1_field) {
        module->editing = true;
        module->keystrokes_accepted = false;  // Text input field needs the keystrokes.
        // Update math1, if changed.
        if (module->current_circle >= 0) {
          if (module->circles[module->current_circle].math1.compare(math1_field->getText()) != 0) {
            Circle old_circle = module->circles[module->current_circle];
            module->circles[module->current_circle].math1 = math1_field->getText();
            APP->history->push(
                new VennCircleUndoRedoAction(module->id, old_circle,
                                             module->circles[module->current_circle],
                                             module->current_circle));
          }
        }
      } else {
        module->editing = false;
        module->keystrokes_accepted = false;
      }
    }
  } 

  // This allows circle editing even when the display widget itself is not selected.
  void onSelectKey(const SelectKeyEvent& e) override {
    if (display) {
      display->onSelectKey(e);
    }
  }

  void appendContextMenu(Menu* menu) override {
    Venn* module = dynamic_cast<Venn*>(this->module);
    menu->addChild(createBoolPtrMenuItem("Show Keyboard Commands", "",
                                         &(module->show_keyboard)));
    menu->addChild(createBoolPtrMenuItem("Only Compute MATH1 for a circle when inside it", "",
                                         &(module->only_compute_math1_within)));
    menu->addChild(new MenuSeparator);
        // Now add math functions.
    // description, inserted text.
    std::string math_funcs[] = {
      "Math operators: +, -, *, /",
      "b ? t : f - returns t if b is 'true' or non-zero, f otherwise",
      "Bool operators: ==, !=, >, >=, <, <=, and, or, not",
      "Notes: c#3, B1, Gb2",
      "Overall: pointx, pointy, leftx, rightx, topy, bottomy",
      "Per circle: distance, within, x, y",
      "limit(a, b, c) - returns 'a' but between b and c",
      "scale(a, b, c, d, e) - scales a in b-c range to d-e range",
      "abs(k) - this number without a negative sign",
      "ceiling(k) - integer value at or above k",
      "floor(k) - integer value at or below k",
      "log2(k) - Base 2 logarithm of k; 0 for k <= 0",
      "loge(k) - Natural logarithm of k; 0 for k <= 0",
      "log10(k) - Base 10 logarithm of k; 0 for k <= 0",
      "max(k, m) - larger of k or m",
      "min(k, m) - smaller of k or m",
      "mod(k, m) - remainder after dividing k by m",
      "pow(k, m) - k to the power of m",
      "sign(k) - -1, 0, or 1, depending on the sign of k",
      "sin(k) - sine of k, which is in radians",
    };
    MenuItem* math_menu = createSubmenuItem("MATH Cheat Sheet", "",
      [=](Menu* menu) {
          for (auto line : math_funcs) {
            menu->addChild(createMenuItem(line, "",
              [=]() { }
            ));
          }
      }
    );
    menu->addChild(math_menu);

    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Inspired by Leafcutter John's 'Forester' instrument."));
  }
};


Model* modelVenn = createModel<Venn, VennWidget>("Venn");