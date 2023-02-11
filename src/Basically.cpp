#include <algorithm>
#include <cctype>
#include <cmath>
#include <thread>
#include <unordered_map>
#include <utility>  // pair
#include <vector>

#include "code_block.h"
#include "parser/environment.h"
#include "plugin.hpp"
#include "parser/driver.hh"
#include "pcode_trans.h"
#include "st_textfield.hpp"

enum Style {
  ALWAYS_STYLE,
  TRIGGER_LOOP_STYLE,
  TRIGGER_NO_LOOP_STYLE,
  GATE,
  NO_STYLE
};

Style STYLES[] = {
  ALWAYS_STYLE,
  TRIGGER_LOOP_STYLE,
  TRIGGER_NO_LOOP_STYLE,
  GATE
};

struct Basically : Module {
  static const int DEFAULT_WIDTH = 18;

  enum ParamId {
    RUN_PARAM,
    STYLE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    IN1_INPUT,
    IN2_INPUT,
    IN3_INPUT,
    IN4_INPUT,
    RUN_INPUT,
    IN5_INPUT,
    IN6_INPUT,
    IN7_INPUT,
    IN8_INPUT,
    IN9_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUT1_OUTPUT,
    OUT2_OUTPUT,
    OUT3_OUTPUT,
    OUT4_OUTPUT,
    OUT5_OUTPUT,
    OUT6_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    RUN_LIGHT,   // Currently running.
    LIGHTS_LEN
  };

  // Structure that maps text name to enum to float* for I/O variables.
  struct InPortInfo {
    std::string name;
    InputId id;
    float* value_ptr;

    InPortInfo(const std::string a, InputId b) : name{a}, id{b} {
      value_ptr = nullptr;
    }
  };

  struct TriggerInfo {
    InputId index;
    dsp::SchmittTrigger trigger;
    bool current_value;
  };

  class ProductionEnvironment : public Environment {
    std::vector<Input>* inputs;
    std::vector<Output>* outputs;
    const ProcessArgs* args;
    Driver* driver;
    bool starting;
    std::vector<CodeBlock*>* main_blocks;
    std::vector<std::pair<Expression, CodeBlock*> >* expression_blocks;
    std::vector<bool>* running_expression_blocks;
    std::unordered_map<int, TriggerInfo*> triggers;
    std::chrono::high_resolution_clock::time_point time_start;
    // Map output index to whether or not we should clamp values for it.
    std::unordered_map<int, bool>* out_index_to_clamp;

   public:
    ProductionEnvironment(std::vector<Input>* the_inputs,
                          std::vector<Output>* the_outputs,
                          Driver* the_driver,
                          std::unordered_map<int, bool>* clamp_info) :
       inputs{the_inputs}, outputs{the_outputs}, driver{the_driver},
       out_index_to_clamp{clamp_info} {
         time_start = std::chrono::high_resolution_clock::now();
       }

     // Must be called every time the program is recompiled.
    void ResetBlocks(std::vector<CodeBlock*>* the_main_blocks,
        std::vector<std::pair<Expression, CodeBlock*> >* the_expression_blocks,
        std::vector<bool>* the_running_expression_blocks) {
          main_blocks = the_main_blocks;
          expression_blocks = the_expression_blocks;
          running_expression_blocks = the_running_expression_blocks;
     }

     // When program has been recompiled, call this to set up trigger monitoring.
     void ResetTriggers() {
      // First, clear existing map.
      for (auto t : triggers) {
        delete t.second;
      }
      triggers.clear();
      for (int index : driver->trigger_port_indexes) {
        TriggerInfo* trig = new TriggerInfo();
        trig->index = (InputId) index;
        trig->trigger.reset();
        trig->current_value = false;
        triggers[index] = trig;
      }
    }

    void UpdateTriggers() {
      for (auto t : triggers) {
        TriggerInfo* value = t.second;
        bool was_low = !(value->trigger.isHigh());
        value->trigger.process(rescale(
            inputs->at(t.first).getVoltage(), 0.1f, 2.f, 0.f, 1.f));
        value->current_value = was_low && value->trigger.isHigh();
      }
    }

    // ProcessArgs object isn't available when we first create the Environment.
    // So we need to update it when it is available.
    void SetProcessArgs(const ProcessArgs* the_args) {
      args = the_args;
    }

    // Should be called every sample.
    void SetStarting(bool start) {
      starting = start;
    }

    float GetVoltage(const PortPointer &port) override {
      if (port.port_type == PortPointer::INPUT) {
        return inputs->at(port.index).getVoltage();
      } else {
        return outputs->at(port.index).getVoltage();
      }
    }
    void SetVoltage(const PortPointer &port, float value) override {
      if (port.port_type == PortPointer::INPUT) {
        inputs->at(port.index).setVoltage(value);
      } else {
        // Force output values to -10 <= x <= 10 range.
        // Set in menu.
        if (out_index_to_clamp->at(port.index)) {
          value = clamp(value, -10.0f, 10.0f);
        }
        outputs->at(port.index).setVoltage(value);
      }
    }
    float SampleRate() override {
      return args->sampleRate;
    }
    float Connected(const PortPointer &port) override {
      if (port.port_type == PortPointer::INPUT) {
        return inputs->at(port.index).isConnected() ? 1.0f : 0.0f;
      } else {
        return outputs->at(port.index).isConnected() ? 1.0f : 0.0f;
      }
    };
    float Random(float min_value, float max_value) override {
      return rescale(rack::random::uniform(), 0.0, 1.0, min_value, max_value);
    }
    float Normal(float mean, float std_dev) override {
      return rack::random::normal() * std_dev + mean;
    }
    float Time(bool millis) override {
      if (millis) {
        std::chrono::duration<double, std::milli> elapsed =
          std::chrono::high_resolution_clock::now() - time_start;
        return elapsed.count();
      } else {
        std::chrono::duration<double> elapsed =
          std::chrono::high_resolution_clock::now() - time_start;
        return elapsed.count();
      }
    }
    void Clear() override {
      driver->Clear();
    }
    void Reset() override {
      if (main_blocks) {
        for (auto block : *main_blocks) {
          block->current_line = 0;
          block->wait_info.in_wait = false;
          // NOTE: we do not reset the run_status.
        }
        for (size_t pos = 0; pos < running_expression_blocks->size(); pos++) {
          running_expression_blocks->at(pos) = false;
          expression_blocks->at(pos).second->current_line = 0;
          expression_blocks->at(pos).second->wait_info.in_wait = false;
        }
      }
    }

    // True ONLY when the program has just compiled.
    bool Start() override {
      return starting;
    }

    bool Trigger(const PortPointer &port) override {
      auto found = triggers.find(port.index);
      if (found != triggers.end()) {
        return found->second->current_value;
      } else {
        // Only a compilation error should cause this, but best way to fail
        // is to say false.
        return false;
      }
    }
  };

  // Class devoted to handling the lengthy (compared to single sample)
  // process of compiling the code.
  // TODO: Typing in comments recompiles every on every keystroke.
  // This is a good place to include logic for determining if a compile actually
  // changed anything. Maybe some sort of hash on the AST (Line) structure? Or
  // even just a hash on the list of tokens?
  struct CompilationThread {
    Driver* driver;
    Environment* environment;
    std::string text;  // Text to compile.
    bool running;  // TRUE if still compiling, false if completed.
    bool useful; // TRUE if last completed compile created something for module to use.
    // Product of a successful compilation.
    std::vector<CodeBlock*>* main_blocks;
    std::vector<std::pair<Expression, CodeBlock*> >* expression_blocks;
    std::vector<bool>* running_expression_blocks;

    explicit CompilationThread(Driver* drv, Environment* env) : driver{drv},
        environment{env} {
      running = false;
      useful = false;
    }

    void SetText(const std::string &new_text) {
      running = true;  // Tells caller not to use previous result.
      text = new_text;
    }

    // If compilation succeeds, sets flag saying so and prepares vectors
    // of main_blocks and expression_blocks for module to use later.
    void Compile() {
      running = true;
      useful = false;
      // We are deliberately case-insensitive.
      std::string lowercase;
      lowercase.resize(text.size());
      std::transform(text.begin(), text.end(),
                     lowercase.begin(), ::tolower);
      bool compiles = !driver->parse(lowercase);
      if (!compiles) {  // Nothing more to do.
        running = false;
        return;
      }
      PCodeTranslator translator;
      main_blocks = new std::vector<CodeBlock*>();
      expression_blocks = new std::vector<std::pair<Expression, CodeBlock*> >();
      running_expression_blocks = new std::vector<bool>();
      useful = true;
      for (auto ast_block : driver->blocks) {
        CodeBlock* new_block = new CodeBlock(environment);
        if (translator.BlockToCodeBlock(new_block, ast_block)) {
          // Different lists depending on type.
          if (new_block->type == Block::MAIN) {
            main_blocks->push_back(new_block);
          } else if (new_block->type == Block::WHEN &&
              new_block->condition == Block::EXPRESSION) {
            expression_blocks->push_back(std::make_pair(ast_block.run_condition, new_block));
            running_expression_blocks->push_back(false);
          }
        } else {
          useful = false;
          // TODO: Report errors via some new mechanism.
        }
      }
      running = false;
    }
  };


  std::map<std::string, int> out_map { {"out1", OUT1_OUTPUT},
                                       {"out2", OUT2_OUTPUT},
                                       {"out3", OUT3_OUTPUT},
                                       {"out4", OUT4_OUTPUT},
                                       {"out5", OUT5_OUTPUT},
                                       {"out6", OUT6_OUTPUT}
                                     };
  std::vector<InPortInfo> in_list { {"in1", IN1_INPUT},
                                    {"in2", IN2_INPUT},
                                    {"in3", IN3_INPUT},
                                    {"in4", IN4_INPUT},
                                    {"in5", IN5_INPUT},
                                    {"in6", IN6_INPUT},
                                    {"in7", IN7_INPUT},
                                    {"in8", IN8_INPUT},
                                    {"in9", IN9_INPUT}
                                  };

  Basically() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configButton(RUN_PARAM, "Press to run");
    configSwitch(STYLE_PARAM, 0, 3, 0, "When to run",
                 {"Always run", "Start on trigger, loop",
                  "Start on trigger, don't loop",
                  "Run when gate is open"});
    // This has distinct values.
    getParamQuantity(STYLE_PARAM)->snapEnabled = true;
    configInput(IN1_INPUT, "IN1");
    configInput(IN2_INPUT, "IN2");
    configInput(IN3_INPUT, "IN3");
    configInput(IN4_INPUT, "IN4");
    configInput(IN5_INPUT, "IN5");
    configInput(IN6_INPUT, "IN6");
    configInput(IN7_INPUT, "IN7");
    configInput(IN8_INPUT, "IN8");
    configInput(IN9_INPUT, "IN9");
    configInput(RUN_INPUT, "Trigger to start or Gate to start/stop (See Style)");
    configOutput(OUT1_OUTPUT, "OUT1");
    configOutput(OUT2_OUTPUT, "OUT2");
    configOutput(OUT3_OUTPUT, "OUT3");
    configOutput(OUT4_OUTPUT, "OUT4");
    configOutput(OUT5_OUTPUT, "OUT5");
    configOutput(OUT6_OUTPUT, "OUT6");
    configLight(RUN_LIGHT, "Lit when code is currently running.");

    environment = new ProductionEnvironment(&inputs, &outputs, &drv,
        &clamp_info);
    drv.SetEnvironment((Environment*) environment);
    // For now, we just have the one block, but we'll add more soon.
    // Add the INn variables to the variable space, and get the pointer to
    // them so module can set them.
    for (size_t i = 0; i < in_list.size(); i++) {
      drv.AddPortForName(in_list[i].name, true, in_list[i].id);
    }
    // Add the OUTn variables to the symbol table, just so we're sure they
    // are present. But we don't appear to need to keep the pointers in
    // the module.
    // Also initialize clamp_info to all True;
    for (auto output : out_map) {
      drv.AddPortForName(output.first, false, output.second);
      clamp_info[output.second] = true;
    }
    compile_in_progress = false;
    compiler = new CompilationThread(&drv, (Environment*) environment);

    // Filling in empty values for these until something compiles.
    main_blocks = new std::vector<CodeBlock*>();
    expression_blocks = new std::vector<std::pair<Expression, CodeBlock*> >();
    running_expression_blocks = new std::vector<bool>();
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "text", json_stringn(text.c_str(), text.size()));
    json_object_set_new(rootJ, "width", json_integer(width));

    // OUTn clamping.
    json_t* clampJ = json_object();
    for (const auto& key_value : clamp_info ) {
      // Store only the false values, since those will be less numerous,
      // and any new OUTn's should default to True.
      if (!key_value.second) {
        json_object_set_new(clampJ, std::to_string(key_value.first).c_str(),
                            json_integer(0));
      }
    }
    if (json_object_size(clampJ) > 0) {
      json_object_set_new(rootJ, "clamp", clampJ);
    }

    if (allow_error_highlight) {
      json_object_set_new(rootJ, "allow_error_highlight", json_integer(1));
    }
    if (blue_orange_light) {
      json_object_set_new(rootJ, "blue_orange_light", json_integer(1));
    }
    json_object_set_new(rootJ, "screen_colors", json_integer(screen_colors));
    if (title_text.length() > 0) {
      json_object_set_new(rootJ, "title_text",
                          json_stringn(title_text.c_str(), title_text.size()));
    }
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* textJ = json_object_get(rootJ, "text");
		if (textJ) {
			text = json_string_value(textJ);
      previous_text = text;
  		editor_refresh = true;
      module_refresh = true;
    }
    json_t* title_textJ = json_object_get(rootJ, "title_text");
		if (title_textJ) {
			title_text = json_string_value(title_textJ);
    }
    json_t* widthJ = json_object_get(rootJ, "width");
		if (widthJ)
			width = json_integer_value(widthJ);

    // OUTn clamping.
    json_t* clampJ = json_object_get(rootJ, "clamp");
		if (clampJ) {
      const char *key;
      json_t *value;
      // Assuming that all values in clamp_info have been set to true.
      json_object_foreach(clampJ, key, value) {
        int index = strtol(key, NULL, 10);
        clamp_info[index] = false;
      }
    }

    json_t* screenJ = json_object_get(rootJ, "screen_colors");
		if (screenJ)
			screen_colors = json_integer_value(screenJ);
    json_t* error_highlightJ = json_object_get(rootJ, "allow_error_highlight");
    if (error_highlightJ) {
      allow_error_highlight = json_integer_value(error_highlightJ) == 1;
    } else {
      allow_error_highlight = false;
    }
    json_t* blue_orange_lightJ = json_object_get(rootJ, "blue_orange_light");
    if (blue_orange_lightJ) {
      blue_orange_light = json_integer_value(blue_orange_lightJ) == 1;
    } else {
      blue_orange_light = false;
    }
  }

  void ResetToProgramStart() {
    environment->Reset();
    // Note that we do not clear the variable/array state.
    // Keeping previously defined variables makes live-coding work.
    // TODO: add CLEAR command for just this.
  }

  void processBypass(const ProcessArgs& args) override {
    outputs[OUT1_OUTPUT].setVoltage(0.0f);
    outputs[OUT2_OUTPUT].setVoltage(0.0f);
    outputs[OUT3_OUTPUT].setVoltage(0.0f);
    outputs[OUT4_OUTPUT].setVoltage(0.0f);
    outputs[OUT5_OUTPUT].setVoltage(0.0f);
    outputs[OUT6_OUTPUT].setVoltage(0.0f);
  }

  void process(const ProcessArgs& args) override {
    Style style = STYLES[(int) params[STYLE_PARAM].getValue()];
    // Changing from a style that doesn't care about stopping to one that does
    // can mess things up. So if we see the style change, we reset the
    // run_status on all the blocks.
    if (style != previous_style) {
      previous_style = style;
      // Tell each ALSO block they are primed to run.
      for (CodeBlock* block : *main_blocks) {
        block->run_status = CodeBlock::CONTINUES;
      }
    }
    bool loops = (style != TRIGGER_NO_LOOP_STYLE);
    // ProcessArgs not available when we first create the Environment.
    environment->SetProcessArgs(&args);
    // Might be set true below, but for vast majority of samples this is false.
    environment->SetStarting(false);

    // If a compilation is already in progress, see if it has completed.
    if (compile_in_progress) {
      if (compiler->running) {
        // OK, we'll keep waiting.
      } else {
        compile_in_progress = false;
        if (compiler->useful) {
          // Got something we can use. First, clean upi the old ones.
          if (main_blocks) {
            for (auto block : *main_blocks) {
              delete block;
            }
            delete main_blocks;
          }
          if (expression_blocks) {
            for (auto p : *expression_blocks) {
              delete p.second;
            }
            delete expression_blocks;
          }
          if (running_expression_blocks) {
            delete running_expression_blocks;
          }
          // Now replace with the new ones.
          main_blocks = compiler->main_blocks;
          expression_blocks = compiler->expression_blocks;
          running_expression_blocks = compiler->running_expression_blocks;
          environment->ResetBlocks(main_blocks, expression_blocks,
              running_expression_blocks);
          // Recompiled; cannot trust program state. But note we are leaving
          // *variable* state intact.
          ResetToProgramStart();
          // Tell the start() function that we are starting.
          environment->SetStarting(true);
          environment->ResetTriggers();
          compiles = true;
        } else {
          compiles = false;
        }
        // No longer need thread that was running it.
        compile_thread->join();  // It should be done by now, right?
        delete compile_thread;
      }
    } else {
      // Do not currently have compile in progress.
      if (module_refresh && !text.empty()) {
        compile_in_progress = true;
        module_refresh = false;
        // Start a new compile.
        compiler->SetText(text);
        compile_thread = new std::thread(&CompilationThread::Compile, compiler);
      }
    }

    // Update the environment's notion of our trigger state.
    environment->UpdateTriggers();

    // Determine if we are running or not.
    // This section is more complicated than needed to avoid unneeded
    // work every process() call.
    bool prev_running = running;
    if (style == ALWAYS_STYLE) {
      if (compiles) running = true;
    } else if (style == TRIGGER_LOOP_STYLE || style == TRIGGER_NO_LOOP_STYLE) {
      if (!running) {
        bool run_was_low = !runTrigger.isHigh();
        runTrigger.process(rescale(
            inputs[RUN_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
        bool run_button_pressed = params[RUN_PARAM].getValue() > 0.1f;
        bool asked_to_start_run = (run_button_pressed ||
                                   (run_was_low && runTrigger.isHigh()));
        if (asked_to_start_run) {
          running = true;
          ResetToProgramStart();
          // Tell each ALSO block they are primed to run.
          for (CodeBlock* block : *main_blocks) {
            block->run_status = CodeBlock::CONTINUES;
          }
        }
      }
    } else {  // style == GATE
      runTrigger.process(rescale(
          inputs[RUN_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
      if (runTrigger.isHigh()) {
        running = true;
      } else {
        running = params[RUN_PARAM].getValue() > 0.1f;
      }
    }
    if (main_blocks->empty() && expression_blocks->empty()) {
      // TODO: If we have a START block but no main blocks, are we running?
      // No code to run --> don't say we're running!
      running = false;
    }
    if (!prev_running && running) {
      // Flash the run light for a brief part of  asecond.
      // Compute how many samples to show the light.
      // TODO: should we set run_starts here as well?
      run_light_countdown = std::floor(args.sampleRate / 20.0f);
    }

    // If RESET is called anywhere in here, we need to end execution,
    // or we end up in an infinite loop.
    if (running) {
      CodeBlock::RunStatus run_status = CodeBlock::CONTINUES;
      // WHEN EXPRESSION blocks:
      for (size_t pos = 0; pos < expression_blocks->size(); pos++) {
        // If already running, don't test the expression.
        if (!running_expression_blocks->at(pos)) {
          // Run expression, see if now true.
          running_expression_blocks->at(pos) =
            !Expression::is_zero(expression_blocks->at(pos).first.Compute());
        }
        // If now running, Run() a step.
        if (running_expression_blocks->at(pos)) {
          run_status = expression_blocks->at(pos).second->Run(false);
          if (run_status == CodeBlock::RAN_RESET) {
            break;
          }
          running_expression_blocks->at(pos) = run_status == CodeBlock::CONTINUES;
        }
      }
      if (run_status != CodeBlock::RAN_RESET) {
        // See if any are still running after this step.
        bool any_main_blocks_running = false;
        for (CodeBlock* block : *main_blocks) {
          if (block->run_status != CodeBlock::STOPPED) {
            run_status = block->Run(loops);
            if (run_status == CodeBlock::CONTINUES) {
              any_main_blocks_running = true;
            }
            if (run_status == CodeBlock::RAN_RESET) {
              break;
            }
          }
        }
        if (!main_blocks->empty() && !any_main_blocks_running) {
          running = false;
        }
      }
    }

    // Lights.
    if (run_light_countdown > 0) {
      run_light_countdown--;
    }
    lights[RUN_LIGHT].setBrightness(
      (running || run_light_countdown > 0) ? 1.0f : 0.0f);
  }

  dsp::SchmittTrigger runTrigger;
  // Set when module changes the text (like at start).
  // Set when editing window needs to refresh based on text.
  bool editor_refresh = false;
  // Set when module needs to refresh (e.g., compile) text.
  bool module_refresh = true;
  bool compiles = false;
  bool running = false;
  Style previous_style = NO_STYLE;
  Driver drv;
  CompilationThread* compiler;
  std::thread* compile_thread;
  bool compile_in_progress = false;
  ProductionEnvironment* environment;
  // The untitled default block and any ALSO - END ALSO blocks.
  // All main blocks are always running, so we don't need a distinct list.
  std::vector<CodeBlock*>* main_blocks;
  // All WHEN EXPRESSION blocks. Is a vector to maintain order.
  std::vector<std::pair<Expression, CodeBlock*> >* expression_blocks;
  // Which ones are running.
  std::vector<bool>* running_expression_blocks;
  std::unordered_map<int, bool> clamp_info;

  ///////
  // UI related
  // Full text of program.  Also used by BasicallyTextField for editing.
  std::string text;
  // We need to the immediately previous version of the text around to
  // make undo and redo work; otherwise, we don't know what the change was.
  std::string previous_text;
  // We want to track the previous placement of the cursor, so that an
  // undo can take us back there.
  int previous_cursor = 0;
  bool allow_error_highlight = true;
  bool blue_orange_light = false;
  // Green on Black is the default.
  long long int screen_colors = 0x00ff00000000;
  // Keeps lights lit long enough to see.
  int run_light_countdown = 0;
  // width (in "holes") of the whole module. Changed by the resize bar on the
  // right (within limits), and informs the size of the display and text field.
  // Saved in the json for the module.
  int width = Basically::DEFAULT_WIDTH;
  // Program-created visible title for the program.
  std::string title_text;
  // The undo/redo sometimes needs to reset the cursor position.
  // But we don't actully have a good pointer to the text field.
  // drawLayer() uses this if it's > -1;
  int cursor_override = -1;
};

// Adds support for undo/redo in the text field where people type programs.
struct TextEditAction : history::ModuleAction {
  std::string old_text;
  std::string new_text;
  int old_cursor, new_cursor;
  int old_width;
  int new_width;

  TextEditAction(int64_t id, std::string oldText, std::string newText,
     int old_cursor_pos, int new_cursor_pos) : old_text{oldText},
         new_text{newText}, old_cursor{old_cursor_pos},
         new_cursor{new_cursor_pos} {
    moduleId = id;
    name = "edit code";
    old_width = new_width = -1;
  }
  TextEditAction(int64_t id, int old_width, int new_width) :
      old_width{old_width}, new_width{new_width} {
    moduleId = id;
    name = "change module width";
  }
  void undo() override {
    Basically *module = dynamic_cast<Basically*>(APP->engine->getModule(moduleId));
    if (module) {
      if (old_width < 0) {
        module->text = this->old_text;
        // Tell UI it needs to refresh because 'text' has changed.
        module->editor_refresh = true;
        // Tell module it needs to re-evaluate 'text'.
        module->module_refresh = true;
        module->cursor_override = old_cursor;
      } else {
        module->width = this->old_width;
      }
    }
  }

  void redo() override {
    Basically *module = dynamic_cast<Basically*>(APP->engine->getModule(moduleId));
    if (module) {
      if (old_width < 0) {
        module->text = this->new_text;
        // Tell UI it needs to refresh because 'text' has changed.
        module->editor_refresh = true;
        // Tell module it needs to re-evaluate 'text'.
        module->module_refresh = true;
        module->cursor_override = new_cursor;
      } else {
        module->width = this->new_width;
      }
    }
  }
};

struct ModuleResizeHandle : OpaqueWidget {
	Vec dragPos;
	Rect originalBox;
	Basically* module;

	ModuleResizeHandle() {
    // One hole wide and full length tall.
		box.size = Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	}

	void onDragStart(const DragStartEvent& e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT)
			return;

		dragPos = APP->scene->rack->getMousePos();
		ModuleWidget* mw = getAncestorOfType<ModuleWidget>();
		assert(mw);
		originalBox = mw->box;
	}

	void onDragMove(const DragMoveEvent& e) override {
		ModuleWidget* mw = getAncestorOfType<ModuleWidget>();
		assert(mw);
    int original_width = module->width;

		Vec newDragPos = APP->scene->rack->getMousePos();
		float deltaX = newDragPos.x - dragPos.x;

		Rect newBox = originalBox;
		Rect oldBox = mw->box;
    // Minimum and maximum number of holes we allow the module to be.
		const float minWidth = 7 * RACK_GRID_WIDTH;
    const float maxWidth = 64 * RACK_GRID_WIDTH;
		newBox.size.x += deltaX;
		newBox.size.x = std::fmax(newBox.size.x, minWidth);
    newBox.size.x = std::fmin(newBox.size.x, maxWidth);
		newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;

		// Set box and test whether it's valid.
		mw->box = newBox;
		if (!APP->scene->rack->requestModulePos(mw, newBox.pos)) {
			mw->box = oldBox;
		}
		module->width = std::round(mw->box.size.x / RACK_GRID_WIDTH);
    if (original_width != module->width) {
      // Make this an undo action. If I don't do this, undoing a different
      // module's move will cause them to overlap.
      APP->history->push(
        new TextEditAction(module->id, original_width, module->width));
    }
	}

	void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      // Draw two lines to give people something to grab for.
      // Lifted from the VCV Blank module.
  		for (float x = 5.0; x <= 10.0; x += 5.0) {
  			nvgBeginPath(args.vg);
  			const float margin = 5.0;
  			nvgMoveTo(args.vg, x + 0.5, margin + 0.5);
  			nvgLineTo(args.vg, x + 0.5, box.size.y - margin + 0.5);
  			nvgStrokeWidth(args.vg, 1.0);
  			nvgStrokeColor(args.vg, nvgRGBAf(0.5, 0.5, 0.5, 0.5));
  			nvgStroke(args.vg);
  		}
    }
	}
};

struct TitleTextField : widget::OpaqueWidget {
  Basically* module;
  std::string fontPath;

  TitleTextField() {
    // TODO: Pick better font.
    fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));
    if (layer == 1) {
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();
      // No background color!

      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (font && module) {
        std::string text = module->title_text;
        nvgFillColor(args.vg, color::BLACK);
        // The longer the text, the smaller the font. 20 is our largest size,
        // and it handles 10 chars of this font. 10 is smallest size, it can
        // handle 25 chars.
        int font_size = 24;
        std::vector<std::string> lines;
        if ((int) text.length() > 8) {
          font_size = 15;
          int nearest_to_mid = -1;
          int text_length = (int) text.length();
          // Look for a space we can break on.
          for (int i = 0; i < text_length; i++) {
            if (text.at(i) == ' ') {
              if (abs(i - (text_length / 2)) <
                abs(nearest_to_mid - (text_length / 2))) {
                  nearest_to_mid = i;
              }
            }
          }
          if (nearest_to_mid == -1) {
            // Just split it for them.
            lines.push_back(text.substr(0, text_length / 2));
            lines.push_back(text.substr(text_length / 2));
          } else {
            lines.push_back(text.substr(0, nearest_to_mid));
            lines.push_back(text.substr(nearest_to_mid + 1));
          }
        } else {
          lines.push_back(text);
        }
        nvgFontSize(args.vg, font_size);
        nvgTextAlign(args.vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -2);
        // Place on the line just off the left edge.
        for (int i = 0; i < (int) lines.size(); i++) {
          nvgText(args.vg, bounding_box.x / 2, i * 12, lines[i].c_str(), NULL);
        }
      }
    }
    Widget::drawLayer(args, layer);
    nvgResetScissor(args.vg);
  }
};

// Class for the editor.
struct BasicallyTextField : STTextField {
	Basically* module;
  long long int color_scheme;

  NVGcolor int_to_color(int color) {
    return nvgRGB(color >> 16, (color & 0xff00) >> 8, color & 0xff);
  }

  // bgColor seems to have no effect if I don't do this. Drawing a background
  // and then letting LedDisplayTextField draw the rest will fixes that.
  void drawLayer(const DrawArgs& args, int layer) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));

    if (layer == 1) {
  		// background only
      nvgBeginPath(args.vg);
      nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
      nvgFillColor(args.vg, bgColor);
      nvgFill(args.vg);

      // Let's try highlighting a line, though, if needed and allowed to.
      if (module && module->allow_error_highlight) {
        // Highlight the line with an error, if any.
        if (module->drv.errors.size() > 0) {
          int line_number = module->drv.errors[0].line - extended.lines_above;
          nvgBeginPath(args.vg);
          int topFudge = textOffset.y + 5;  // I'm just trying things until they work.
          // textOffset is in ledDisplayTextField.
          nvgRect(args.vg, 0, topFudge + 12 * (line_number - 1), box.size.x, 12);
          nvgFillColor(args.vg,
              module->blue_orange_light ? SCHEME_ORANGE : nvgRGB(128, 0, 0));
          nvgFill(args.vg);
        }
      }
      if (module && module->cursor_override >= 0) {
        // Undo/redo must have just happened.
        // Move cursor (with no selection) to where the cursor was when we
        // did edit.
        cursor = module->cursor_override;
        selection = module->cursor_override;
        module->cursor_override = -1;
        // Since we just forcibly moved the cursor, need to reposition window
        // to show it.
        extended.RepositionWindow(cursor);
      }
  	}
  	STTextField::drawLayer(args, layer);  // Draw text.
  	nvgResetScissor(args.vg);
  }

	void step() override {
		STTextField::step();
    if (module && (color_scheme != module->screen_colors ||
                   module->editor_refresh)) {
      // Note: this doesn't actully care about editor_refresh. But this cleared
      // up a bug about duplicated windows not keeping the same color.
      color_scheme = module->screen_colors;
      color = int_to_color(color_scheme >> 24);
      bgColor = int_to_color(color_scheme & 0xffffff);
    }
		if (module && module->editor_refresh) {
      // TODO: is this checked often enough? I don't know when step()
      // is called.
      // Text has been changed, editor needs to update itself.
      // This happens when the module loads, and on undo/redo.
			textUpdated();
			module->editor_refresh = false;
		}
	}

  // User has updated the text.
	void onChange(const ChangeEvent& e) override {
		if (module) {
      // Sometimes the text isn't actually different. If I don't check
      // this, I might get spurious history events.
      // TODO: do I need this check anymore?
      if (module->text != module->previous_text) {
        APP->history->push(
          new TextEditAction(module->id, module->previous_text,
                             module->text, module->previous_cursor, cursor));
        module->previous_text = module->text;
        module->module_refresh = true;
      }
      module->previous_cursor = cursor;
    }
	}
};

static std::string module_browser_text =
  "' Write simple code here.\n' For example:\nfor i = 1 to 5 step 0.2\n"
  "  out1 = i * in2 * 0.4\n  wait 100\nnext";

struct BasicallyDisplay : LedDisplay {
  BasicallyTextField* textField;

	void setModule(Basically* module) {
		textField = createWidget<BasicallyTextField>(Vec(0, 0));
		textField->box.size = box.size;
		textField->module = module;
    // If this is the module browser, 'module' will be null!
    if (module != nullptr) {
      textField->text = &(module->text);
    } else {
      // Show something inviting when being shown in the module browser.
      textField->text = &module_browser_text;
    }
    textField->textUpdated();
		addChild(textField);
	}
  // The BasicallyWidget changes size, so we have to reflaect that.
  void step() override {
    // At smallest szie, hide the screen.
    if (textField->module && textField->module->width <= 7) {
      hide();
    } else {
      show();
    }
    textField->box.size = box.size;
    LedDisplay::step();
	}

  // Text insertions from the menu.
  void insertText(const std::string &fragment) {
    textField->insertText(fragment);
  }
};

struct ErrorWidget;
struct ErrorTooltip : ui::Tooltip {
	ErrorWidget* errorWidget;
  std::string error_text;

  ErrorTooltip(const std::string &text) : error_text{text} {}

	void step() override;
};

struct ErrorWidget : widget::OpaqueWidget {
  Basically* module;
  std::string fontPath;
  ErrorTooltip* tooltip;

  ErrorWidget() {
    // TODO: Pick better font.
    fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
    tooltip = NULL;
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
  	if (!module)
  		return;
    std::string tip_text;
    if (module->compiles) {
      tip_text = "Program compiles!";
    } else {
      if (module->text.empty()) {
        tip_text = "Type in some code over there ->";
      } else {
        if (module->drv.errors.size() > 0) {
          Error err = module->drv.errors[0];
          // Remove "syntax error, " from message.
          std::string msg = err.message;
          if (msg.rfind("syntax error, ", 0) == 0) {
            msg = msg.substr(14);
          }
          tip_text = "Line " + std::to_string(err.line) + ": " + msg;
        }
      }
    }
  	ErrorTooltip* new_tooltip = new ErrorTooltip(tip_text);
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
      bool good = (module) ? module->compiles : true;
      bool blue_orange = (module) ? module->blue_orange_light : false;
      // Fill the rectangle with either blue or orange.
      // For color blind users, these are better choices than green/red.
      NVGcolor main_color = blue_orange ?
          (good ? SCHEME_BLUE : SCHEME_ORANGE) :
          (good ? SCHEME_GREEN : color::RED);
      nvgBeginPath(args.vg);
      nvgRect(args.vg, 0.5, 0.5,
              bounding_box.x - 1.0f, bounding_box.y - 1.0f);
      nvgFillColor(args.vg, main_color);
      nvgFill(args.vg);

      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (font) {
        nvgFillColor(args.vg,  blue_orange ?
           (good ? color::WHITE : color::BLACK) :
           (good ? color::BLACK : color::WHITE));
        nvgFontSize(args.vg, 13);
        nvgTextAlign(args.vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -2);

        std::string text = (good ? "Good" : "Fix");
        // Place on the line just off the left edge.
        nvgText(args.vg, bounding_box.x / 2, 0, text.c_str(), NULL);
      }
    }
    Widget::drawLayer(args, layer);
  }
};

void ErrorTooltip::step() {
  text = error_text;
	Tooltip::step();
	// Position at bottom-right of parameter
	box.pos = errorWidget->getAbsoluteOffset(errorWidget->box.size).round();
	// Fit inside parent (copied from Tooltip.cpp)
	assert(parent);
	box = box.nudge(parent->box.zeroPos());
}

struct TextFieldMenuItem : TextField {
	TextFieldMenuItem() {
    box.size = Vec(120, 20);
    multiline = false;
  }
};

struct ProgramNameMenuItem : TextFieldMenuItem {
	Basically* module;

	ProgramNameMenuItem(Basically* basically_module) {
    module = basically_module;
    if (module) {
      text = module->title_text;
    } else {
      text = "";
    }
  }
	void onChange(const event::Change& e) override {
    TextFieldMenuItem::onChange(e);
    if (module) {
      module->title_text = text;
    }
  }
};

struct BasicallyWidget : ModuleWidget {
  Widget* topRightScrew;
	Widget* bottomRightScrew;
	Widget* rightHandle;
	BasicallyDisplay* codeDisplay;

  BasicallyWidget(Basically* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Basically.svg")));

    // Set reasonable initial size of module. Will likely get updated below.
    box.size = Vec(RACK_GRID_WIDTH * Basically::DEFAULT_WIDTH, RACK_GRID_HEIGHT);
		if (module) {
      // Set box width from loaded Module when available.
			box.size.x = module->width * RACK_GRID_WIDTH;
    } else {
      // Like when showing the module in the module browser.
      box.size.x = Basically::DEFAULT_WIDTH * RACK_GRID_WIDTH;
    }

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    topRightScrew = createWidget<ScrewSilver>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
    addChild(topRightScrew);
    // TODO: this next line's Y coordinate is very odd.
    addChild(createWidget<ScrewSilver>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    bottomRightScrew = createWidget<ScrewSilver>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH,
            RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
    addChild(bottomRightScrew);

    codeDisplay = createWidget<BasicallyDisplay>(
      mm2px(Vec(31.149, 5.9)));
		codeDisplay->box.size = mm2px(Vec(60.0, 117.0));
    codeDisplay->box.size.x = box.size.x - RACK_GRID_WIDTH * 7.1;
		codeDisplay->setModule(module);

		addChild(codeDisplay);

    // Controls.
    // Run button/trigger/gate.
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(6.496, 17.698)), module, Basically::RUN_INPUT));
    // Making this a Button and not a Latch means that it pops back up
    // when you let go.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(15.645, 17.698)),
                                             module, Basically::RUN_PARAM,
                                             Basically::RUN_LIGHT));

    Trimpot* style_knob = createParamCentered<Trimpot>(
        mm2px(Vec(6.496, 28.468)), module, Basically::STYLE_PARAM);
    style_knob->minAngle = -0.28f * M_PI - (M_PI / 2.0);
    style_knob->maxAngle = 0.28f * M_PI - (M_PI / 2.0);
    style_knob->snap = true;
    addParam(style_knob);

    // Compilation status and error message access.
    // Want the middle of this to be at x=15.645
    ErrorWidget* display = createWidget<ErrorWidget>(mm2px(
        Vec(15.645 - 4.0, 33)));
    display->box.size = mm2px(Vec(8.0, 4.0));
    display->module = module;
    addChild(display);

    // User created title for the "program"?
    // Want the middle of this to be at x=15.645
    TitleTextField* title = createWidget<TitleTextField>(mm2px(
        Vec(15.645 - 15.0, 40)));
    title->box.size = mm2px(Vec(30.0, 10.0));
    title->module = module;
    addChild(title);

    // Data Inputs
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.496, 57.35)),
      module, Basically::IN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.645, 57.35)),
      module, Basically::IN2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.794, 57.35)),
      module, Basically::IN3_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.496, 71.35)),
      module, Basically::IN4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.645, 71.35)),
      module, Basically::IN5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.794, 71.35)),
      module, Basically::IN6_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.496, 83.65)),
      module, Basically::IN7_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.645, 83.65)),
      module, Basically::IN8_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.794, 83.65)),
      module, Basically::IN9_INPUT));

    // The Outputs
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.496, 101.601)),
      module, Basically::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.645, 101.601)),
      module, Basically::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24.794, 101.601)),
      module, Basically::OUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.496, 115.601)),
      module, Basically::OUT4_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.645, 115.601)),
      module, Basically::OUT5_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24.794, 115.601)),
      module, Basically::OUT6_OUTPUT));

    // Resize bar on right.
    ModuleResizeHandle* rightHandle = new ModuleResizeHandle;
		this->rightHandle = rightHandle;
		rightHandle->module = module;
    // Make sure the handle is correctly placed if drawing for the module
    // browser.
    rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
		addChild(rightHandle);
  }

  void step() override {
		Basically* module = dynamic_cast<Basically*>(this->module);
    // While this is really only useful to call when the width changes,
    // I don't think it's currently worth the effort to ONLY call it then.
    // And maybe the *first* time step() is called.
		if (module) {
			box.size.x = module->width * RACK_GRID_WIDTH;
		} else {
      // Like when showing the module in the module browser.
      box.size.x = Basically::DEFAULT_WIDTH * RACK_GRID_WIDTH;
    }

    // Adjust size of area we display code in.
    // "7.5" here is ~6 on the left side plus ~1.1 on the right.
		codeDisplay->box.size.x = box.size.x - RACK_GRID_WIDTH * 7.1;
    // Move the right side screws to follow.
		topRightScrew->box.pos.x = box.size.x - 30;
		bottomRightScrew->box.pos.x = box.size.x - 30;
		rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;

		ModuleWidget::step();
	}

  void appendContextMenu(Menu* menu) override {
    Basically* module = dynamic_cast<Basically*>(this->module);
    // Add color choices.
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Title above IN1-3"));
    menu->addChild(new ProgramNameMenuItem(module));
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Screen colors"));
    long long int default_colors[] = {0x00ff00000000,
                                      0xffffff000000,
                                      0xffd714000000,
                                      0xffc000000000,
                                      0x29b2ef000000,
                                      0x000000ffffff,
                                      0x29b2efffffff
                                    };
    std::string color_names[] = {"Green on Black",
                                 "White on Black",
                                 "Yellow on Black (like Notes)",
                                 "Amber on Black",
                                 "Blue on Black",
                                 "Black on White",
                                 "Blue on White"};
    for (int i = 0; i < std::end(default_colors) - std::begin(default_colors);
         i++) {
      long long int scheme = default_colors[i];
      menu->addChild(createCheckMenuItem(color_names[i], "",
          [=]() {return scheme == module->screen_colors;},
          [=]() {module->screen_colors = scheme;}
      ));
    }
    // Options
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Highlight error line", "",
                                          &module->allow_error_highlight));
    menu->addChild(createBoolPtrMenuItem("Colorblind-friendly status light", "",
                                          &module->blue_orange_light));
    // Clamping
    MenuItem* clamp_menu = createSubmenuItem("Clamp OUTn values to (-10V, 10V)", "",
      [=](Menu* menu) {
          for (auto line : module->out_map) {
            // We use out_map, but want the name capitalized.
            std::string upper(line.first);
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            menu->addChild(createBoolMenuItem(upper, "",
                [=]() {return module->clamp_info[line.second];},
                [=](bool checked) {module->clamp_info[line.second] = checked;}
            ));
          }
      }
    );
    menu->addChild(clamp_menu);

    // Add syntax insertions.
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel(
      "Language hints (selecting inserts code)"));
    std::pair<std::string, std::string> syntax[] = {
      {"OUT1 = IN1 + offset[n]", "OUT1 = IN1 + offset[n]\n"},
      {"WAIT 200", "WAIT 200\n"},
      {"' I'm a comment!", "' I'm a comment! Only humans read me.\n"},
      {"IF IN1 == 0 THEN OUT1 = IN2 * IN2 END IF\n",
       "IF IN1 == 0 THEN\n  OUT1 = IN2 * IN2\nEND IF\n"},
      {"IF IN1 == 0 THEN OUT1 = IN2 * IN2 ELSEIF IN1 < 2 THEN OUT1 = IN2 END IF\n",
       "IF IN1 == 0 THEN\n  OUT1 = IN2 * IN2\nELSEIF IN1 < 2 THEN\n  OUT1 = IN2\nEND IF\n"},
      {"IF IN1 == 0 THEN OUT1 = IN2 * IN1 ELSE OUT1 = -5 END IF\n",
       "IF IN1 == 0 THEN\n  OUT1 = IN2 * IN1\nELSE\n  OUT1 = -5\nEND IF\n"},
      {"IF IN1 == 0 THEN OUT1 = IN2 * IN1 ELSEIF IN1 < 2 THEN OUT1 = IN2 ELSE OUT1 = -5 END IF\n",
       "IF IN1 == 0 THEN\n  OUT1 = IN2 * IN1\nELSEIF IN1 < 2 THEN\n  OUT1 = IN2\nELSE\n  OUT1 = -5\nEND IF\n"},
      {"FOR i = 0 TO 10 foo = IN1 + i NEXT\n",
       "FOR i = 0 TO 10\n  foo = IN1 + i\nNEXT\n"},
      {"FOR i = 0 TO 10 STEP 0.2 foo = IN1 + i NEXT\n",
       "FOR i = 0 TO 10 STEP 0.2\n  foo = IN1 + i\nNEXT\n"},
      {"CONTINUE FOR", "CONTINUE FOR\n"},
      {"EXIT FOR", "EXIT FOR\n"},
      {"CONTINUE ALL", "CONTINUE ALL\n"},
      {"EXIT ALL", "EXIT ALL\n"},
      {"CLEAR ALL", "CLEAR ALL\n"},
      {"RESET", "RESET\n"},
      {"ALSO ... END ALSO", "ALSO\n  out1 = mod(out1 + random(0, 0.1))\nEND ALSO"},
      {"WHEN start() limit = 200 curr = 0 END WHEN",
       "WHEN start()\n  limit = 200\n  curr = 0\nEND WHEN"}
    };
    MenuItem* syntax_menu = createSubmenuItem("Syntax", "",
      [=](Menu* menu) {
          for (auto line : syntax) {
            menu->addChild(createMenuItem(line.first, "",
              [=]() { codeDisplay->insertText(line.second); }
            ));
          }
      }
    );
    menu->addChild(syntax_menu);
    // Now add math functions.
    // description, inserted text.
    std::pair<std::string, std::string> math_funcs[] = {
      {"abs(x) - this number without a negative sign", "abs(IN1)"},
      {"ceiling(x) - integer value at or above x", "ceiling(IN1)"},
      {"connected(x) - 1 if named port x has a cable attached, 0 if not",
       "connected(IN1)"},
      {"floor(x) - integer value at or below x", "floor(IN1)"},
      {"log2(x) - Base 2 logarithm of x; 0 for x <= 0", "log2(in1)"},
      {"loge(x) - Natural logarithm of x; 0 for x <= 0", "loge(in2)"},
      {"log10(x) - Base 10 logarithm of x; 0 for x <= 0", "log10(in3)"},
      {"max(x, y) - larger of x or y", "max(IN1, -5)"},
      {"min(x, y) - smaller of x or y", "min(IN1, 5)"},
      {"mod(x, y) - remainder after dividing x by y", "mod(IN1, 1)"},
      {"normal(mean, std_dev) - bell curve distribution of random number",
       "normal(0, 1)"},
      {"pow(x, y) - x to the power of y", "pow(IN1, 0.5)"},
      {"random(x, y) - uniformly random number: x <= random(x, y) < y",
       "random(-1, 1)"},
      {"sample_rate() - sample rate as set in menu. SOMETIMES also be number of times BASICally is called per second (e.g., 44100)",
       "sample_rate()"},
      {"sign(x) - -1, 0, or 1, depending on the sign of x", "sign(IN1)"},
      {"sin(x) - sine of x, which is in radians", "sin(IN1)"},
      {"start() - 1 only for the moment when the program is loaded or changed",
       "WHEN start()"},
      {"time() - Number of seconds since this BASICally module started running",
       "IF time() > 60 THEN ' It's been a minute."},
      {"time_millis() - Number of milliseconds since this BASICally module started running",
       "IF time_millis() > 1000 THEN ' It's been a second."},
      {"trigger(x) - 1 only for the moment when the IN port x receives a trigger",
       "WHEN trigger(IN9)"}
    };
    MenuItem* math_menu = createSubmenuItem("Math", "",
      [=](Menu* menu) {
          for (auto line : math_funcs) {
            menu->addChild(createMenuItem(line.first, "",
              [=]() { codeDisplay->insertText(line.second); }
            ));
          }
      }
    );
    menu->addChild(math_menu);
  }
};

Model* modelBasically = createModel<Basically, BasicallyWidget>("BASICally");
