#include <cmath>

#include "plugin.hpp"
#include "buffered.hpp"

// Design questions (TODO: resolve).

struct Fixation : PositionedModule {
  enum ParamId {
    POSITION_ATTN_PARAM,
    POSITION_KNOB_PARAM,
    PLAY_BUTTON_PARAM,
    SPEED_PARAM,
    LENGTH_KNOB_PARAM,
    LENGTH_ATTN_PARAM,
    COUNT_KNOB_PARAM,
    STYLE_KNOB_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    CLOCK_INPUT,
    POSITION_INPUT,
    PLAY_GATE_INPUT,
    SPEED_INPUT,
    LENGTH_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
    TRIG_OUT_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    CONNECTED_LIGHT,
    PLAY_BUTTON_LIGHT,
    LIGHTS_LEN
  };

  static constexpr float MIN_TIME = 1e-3f;
  static constexpr float MAX_TIME = 10.f;
  static constexpr float LAMBDA_BASE = MAX_TIME / MIN_TIME;

  enum PlayState {
    // We have several states we could be in.
    NO_PLAY,    // * Not playing at all.
    WAITING,    // * Playing, but not told to start yet.
    FADE_UP,    // * Starting to play.
    PLAYING,     // * Continuing to play.
    FADE_DOWN,    // * Fading out the playing -> to not playing at all.
    FADE_DOWN_TO_RESTART, // * Fading out in order to transition to a new starting place.
    FADE_DOWN_TO_WAIT    // * We've hit our repeat limit.

    // TODO: do we need a FADE_UP_TO_RESTART?
    // NB: FADE_* states are unused when buffer->cv_rate is set.

    // Assertions:
    // Can never move from playing <--> not_playing without a fade.
    // Can never move positions without a fade.
    // Can't be in WAITING and STYLE == 1.
    // Should start TRIG when move to FADE_UP.
    // clock_event should set the position and length.

    // start_play == transition from "play off" -> "play on"
    // stop_play == transition from "play on" -> "play off"
    // CLOCK == CLOCK input sees a trigger.
    // length_event == we have played for as long as we were told to.
    // fade_ended == the fade (up or down) has completed.
    // count_reached == the count of repeats has hit the limit.
    // style_changed == just starting, or style has changed.

    // NO_PLAY -> WAITING on [start_play] && STYLE (0, 2) && ![CLOCK]
    // NO_PLAY -> FADE_UP on [start_play] && STYLE (0, 2) && [CLOCK]
    // NO_PLAY -> FADE_UP on [start_play] && STYLE (1)
    // 
    // WAITING -> FADE_UP on [CLOCK] && STYLE (0, 2)
    // WAITING -> FADE_UP on [style_changed && STYLE (1)]  -- if user changed STYLE from 0 -> 1
    // WAITING -> NO_PLAY on [stop_play]
    // 
    // FADE_UP -> PLAYING on fade_ended
    // FADE_UP -> FADE_DOWN on [stop_play]
    // FADE_UP -> FADE_DOWN_TO_RESTART on [CLOCK] && STYLE (0, 2)
    // 
    // PLAYING -> FADE_DOWN on [stop_play]
    // PLAYING -> FADE_DOWN_TO_RESTART on [CLOCK] && STYLE (0, 2) 
    // PLAYING -> FADE_DOWN_TO_RESTART on [length_event] && STYLE (1)
    // PLAYING -> FADE_DOWN_TO_RESTART on [length_event] && STYLE (2) && not count_reached
    // PLAYING -> FADE_DOWN_TO_WAIT on [length_event] && STYLE (2) && count_reached
    // PLAYING -> FADE_DOWN_TO_WAIT on [style_changed && STYLE (0, 2)]
    // PLAYING -> FADE_DOWN_TO_RESTART on [style_changed && STYLE (1)]
    //
    // FADE_DOWN -> NO_PLAY on [fade_ended] and play is off
    // FADE_DOWN -> FADE_DOWN_TO_RESTART on [CLOCK event] && STYLE (0, 2)
    // FADE_DOWN -> WAITING on [start_play] && STYLE (0, 2)
    // FADE_DOWN -> WAITING on [fade_ended] && STYLE (0, 2)
    // FADE_DOWN -> FADE_UP on [start_play] && STYLE (1)
    //
    // FADE_DOWN_TO_RESTART -> FADE_DOWN on [stop_play]
    // FADE_DOWN_TO_RESTART -> FADE_UP on [fade_ended]
    //
    // FADE_DOWN_TO_WAIT -> WAITING on [fade_ended]
    // FADE_DOWN_TO_WAIT -> FADE_DOWN on [stop_play]
    //
  };

  // DO NOT SUBMIT!
  // PlayState prev_play_state = NO_PLAY;


  const double FADE_INCREMENT = 0.02;

  // We look for the nearest Memory every NN samples. This saves CPU time.
  int find_memory_countdown = 0;
  std::shared_ptr<Buffer> buffer;

  // Where we are in the movement, before taking POSITION parameters into account.
  // Always 0.0 <= playback_position < length when Looping.
  // Always 0.0 <= playback_position < 2 * length when Bouncing.
  double playback_position;

  // Where we are in memory (for the timestamp indicator).
  double display_position;

  // When we complete the FADE_DOWN_TO_RESTART, this is where we should pick up from.
  double position_for_restart;

  bool playing;
  int previous_style;
  dsp::SchmittTrigger play_trigger;
  dsp::SchmittTrigger clock_trigger;

  // Cache the buffer length locally.
  int length = 0;

  // To fade volume when near a recording head.
  double fade = 1.0f;

  // When fading at the start or end of adjustments.
  double play_fade = 1.0;
  PlayState play_state;
  bool speed_is_voct = false;  // Saved in the patch.

  // Timer to track the position within the envelope.
  int length_countdown = -1;
  // Total length as of last enquiry.
  int length_in_samples;

  // Only used for STYLE 2 - number of repeats so far completed.
  int repeat_count;

  // Makes the output trigger at TRIG when needed.
  dsp::PulseGenerator trig_generator;

  Fixation() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(POSITION_ATTN_PARAM, -1.0f, 1.0f, 0.f, "Attenuverter on POSITION input.", "%", 0, 100);
    configParam(POSITION_KNOB_PARAM, 0.f, 10.f, 0.f, "Position where playback will start on each CLOCK trigger.");
    configInput(POSITION_INPUT, "Multiplied by the attenuverter, added to the POSITION value.");

    configParam(LENGTH_ATTN_PARAM, -1.0f, 1.0f, 0.f, "Attenuverter on LENGTH input.", "%", 0, 100);
    configParam(LENGTH_KNOB_PARAM, 0.f, 1.f, 0.5f, "Maximum LENGTH of playback.",
                " ms", LAMBDA_BASE, MIN_TIME * 1000);
    configInput(LENGTH_INPUT, "Multiplied by the attenuverter, added to the LENGTH value.");

    configParam(COUNT_KNOB_PARAM, 1, 128, 1,
      "Number of repetitions per CLOCK (in STYLE 'CLOCK starts COUNT repeats...')");
    getParamQuantity(COUNT_KNOB_PARAM)->snapEnabled = true;
    // TODO: Perhaps a fourth style like style 2, but treats CLOCK like a GATE, and keeps playing
    // until the gate closes?
    // Maybe this is a behavior on all of them? Or is that really the same as a gate to the PLAY input?
    configSwitch(STYLE_KNOB_PARAM, 0, 2, 0, "Play Style",
                 {"CLOCK only: LENGTH and COUNT ignored",
                  "Always plays LENGTH: CLOCK and COUNT ignored",
                  "CLOCK starts COUNT repeats of size LENGTH"});
    // This has distinct values.
    getParamQuantity(STYLE_KNOB_PARAM)->snapEnabled = true;

    configOutput(TRIG_OUT_OUTPUT, "Raises a trigger at the start of each play");

    configSwitch(PLAY_BUTTON_PARAM, 0, 1, 0, "Press to start/stop this playback head",
                 {"Silent", "Playing"});
    configParam(SPEED_PARAM, -10.0f, 10.0f, 1.0f, "Playback speed/direction");
    configInput(CLOCK_INPUT, "Resets playback to the location specified by the combination of POSITION values.");
    configInput(PLAY_GATE_INPUT, "Gate to start/stop playing");
    configInput(SPEED_INPUT, "Playback speed (added to knob value)");
    configOutput(LEFT_OUTPUT, "Left");
    configOutput(RIGHT_OUTPUT, "Right");

    line_record.position = 0.0;
    line_record.type = FIXATION;
    playing = false;
    play_state = NO_PLAY;
    playback_position = -1;
    previous_style = -1;
  }
  
  const float octaves[8] = {-2, -1, -.5, -.25, .25, .5, 1, 2};
  const float notes[7] = {1, 9.0/8.0, 5.0/4.0, 4.0/3.0, 3.0/2.0, 5.0/3.0, 15.0/8.0};

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "speed_is_voct", json_integer(speed_is_voct ? 1 : 0));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* speedJ = json_object_get(rootJ, "speed_is_voct");
    if (speedJ) {
      speed_is_voct = json_integer_value(speedJ) == 1;
    }
  }

  // Overriding solely to make sure Adjust isn't left in a non-zero state.
  // And also, we should randomize the position.
  void onRandomize(const RandomizeEvent& e) override {
    // Randomize all parameters
    Module::onRandomize(e);

    // For fun, instead of picking a completely random speed, let's pick from a just
    // intonation scale.
    // Reference: https://en.m.wikipedia.org/wiki/Just_intonation#Diatonic_scale
    params[SPEED_PARAM].setValue(octaves[(int) (random::uniform() * 8)] * 
                                 notes[(int) (random::uniform() * 7)]);
    if (length > 0) {
      playback_position = (int) (((double) length) * random::uniform());
    }
  }

  double GetPosition() {
    double position = (params[POSITION_KNOB_PARAM].getValue() * length / 10.0);
    if (params[POSITION_ATTN_PARAM].getValue() != 0.0f) {
      position += params[POSITION_ATTN_PARAM].getValue() *
        inputs[POSITION_INPUT].getVoltage() * length / 10.0;
    }
    return position;
  }

  int GetLength(const ProcessArgs& args) {
    float length_exp = params[LENGTH_KNOB_PARAM].getValue() +
      (params[LENGTH_ATTN_PARAM].getValue() * inputs[LENGTH_INPUT].getVoltage() / 10.0);
    length_exp = rack::math::clamp(length_exp, 0.0f, 1.0f);
    double length = pow(LAMBDA_BASE, length_exp) * args.sampleRate / 1000;
    // Counting samples goes wonky if we get below one sample of length.
    return std::max((int) floor(length), 1);
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
      // Bad things happen if this are zero, which sometimes happens on startup.
      length = std::max(buffer->length, 1);

      // This affects all behavior, so let's get it up front.
      int style = params[STYLE_KNOB_PARAM].getValue();
      bool style_clock = (style == 0 || style == 2);

      // Assemble all of the events that can change our state.
      // * CLOCK.
      bool clock_was_high = clock_trigger.isHigh();
      clock_trigger.process(rescale(inputs[CLOCK_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
      bool clock_event = !clock_was_high && clock_trigger.isHigh();

      // * length_event.
      bool length_event = false;
      if (length_countdown > 0) {
        --length_countdown;
        if (length_countdown == 0) {
          length_event = true;
        }
      }

      // * start_play and stop_play
      play_trigger.process(rescale(
          inputs[PLAY_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
      bool now_playing = ((params[PLAY_BUTTON_PARAM].getValue() > 0.1f) || play_trigger.isHigh());
      bool start_play = !playing && now_playing;
      bool stop_play = playing && !now_playing;
      playing = now_playing;

      // * fade_ended
      bool fade_ended = false;
      if (play_state == FADE_UP) {
        if (play_fade < 1.0) {
          play_fade = std::min(play_fade + FADE_INCREMENT, 1.0);
        }
        if (play_fade > 0.99) {
          play_fade = 1.0;
          fade_ended = true;
        }
      } else if (play_state == FADE_DOWN ||
                 play_state == FADE_DOWN_TO_RESTART ||
                 play_state == FADE_DOWN_TO_WAIT) {
        if (play_fade > 0.0) {
          play_fade = std::max(play_fade - FADE_INCREMENT, 0.0);
        }
        if (play_fade < 0.01) {
          play_fade = 0.0;
          fade_ended = true;
        }
      }

      bool style_changed = previous_style != style;
      previous_style = style;

      // TODO: simplify opportunities:
      // * if clock then get position and length.

      if (style == 1) {  // Always plays LENGTH: CLOCK and COUNT ignored.
          // length_countdown < 0 -> we just switched to this STYLE.
        if (length_event || length_countdown < 0) {
          position_for_restart = GetPosition();
          length_in_samples = GetLength(args);
          length_countdown = length_in_samples;
        }
      }

      switch (play_state) {
        case NO_PLAY: {
          // Even when not playing, POSITION is probably changing,
          // and we should reflect that.
          playback_position = GetPosition();
          if (start_play) {
            if (style == 1) {
              playback_position = GetPosition();
              trig_generator.trigger(1e-3f);
              play_state = buffer->cv_rate ? PLAYING: FADE_UP;
              length_countdown = GetLength(args);
            } else {
              if (clock_event) { // in case PLAY_GATE and CLOCK are same sample.
                // This is a clone of the code from WAITING state.

                // Since we don't need to fade down, we can TRIG now.
                trig_generator.trigger(1e-3f);
                play_state = buffer->cv_rate ? PLAYING: FADE_UP;
                playback_position = GetPosition();
                length_in_samples = GetLength(args);
                length_countdown = length_in_samples;
                repeat_count = 0;
              } else {
                play_state = WAITING;
              }
            }
          }
        }
        break;

        case WAITING: {
          // Even when not playing, POSITION might be changing,
          // and we should reflect that.
          playback_position = GetPosition();
          if (stop_play) {
            play_state = NO_PLAY;
          } else if (style == 1) {  // user changed STYLE from 0 -> 1, I'm fairly sure.
            trig_generator.trigger(1e-3f);
            play_state = buffer->cv_rate ? PLAYING: FADE_UP;
            playback_position = GetPosition();
            length_countdown  = GetLength(args);
          } else if (clock_event && style_clock) {
            // Since we don't need to fade down, we can TRIG now.
            trig_generator.trigger(1e-3f);
            play_state = buffer->cv_rate ? PLAYING: FADE_UP;
            playback_position = GetPosition();
            length_in_samples  = GetLength(args);
            length_countdown = length_in_samples;
            repeat_count = 0;
          }
        }
        break;

        case FADE_UP: {
          if (stop_play) {
            play_state = buffer->cv_rate ? NO_PLAY :FADE_DOWN;
          } else if (fade_ended) {
            play_state = PLAYING;
          } else if (clock_event && style_clock) {
            play_state = buffer->cv_rate ? PLAYING : FADE_DOWN_TO_RESTART;
            position_for_restart = GetPosition();
            length_in_samples  = GetLength(args);
          }
        }
        break;

        case PLAYING: {
          if (stop_play) {
            play_state = buffer->cv_rate ? NO_PLAY : FADE_DOWN;
          } else if (style_changed) {
            if (style_clock) {
              // Wait for next CLOCK before playing again.
              play_state = buffer->cv_rate ? WAITING : FADE_DOWN_TO_WAIT;
            } else {
              // Go back to position and run for length.
              play_state = buffer->cv_rate ? WAITING : FADE_DOWN_TO_RESTART;
            }
          } else if (clock_event && style_clock) {
            if (buffer->cv_rate) {
              // Just like if FADE_DOWN_TO_RESTART had a fade_ended event.
              trig_generator.trigger(1e-3f);
              playback_position = GetPosition();
              if (style == 1 || style == 2) {
                length_countdown = GetLength(args);
              }              
            } else {
              play_state = FADE_DOWN_TO_RESTART;
              position_for_restart = GetPosition();
              length_in_samples = GetLength(args);
            }
          } else if (length_event && style == 1) {
            if (buffer->cv_rate) {
              // Just like if FADE_DOWN_TO_RESTART had a fade_ended event.
              trig_generator.trigger(1e-3f);
              playback_position = GetPosition();
              length_countdown = GetLength(args);
            } else {
              play_state = FADE_DOWN_TO_RESTART;
              length_in_samples = GetLength(args);
            }
          } else if (length_event && style == 2) {
            ++repeat_count;
            if (repeat_count >= params[COUNT_KNOB_PARAM].getValue()) {
              play_state = buffer->cv_rate ? WAITING : FADE_DOWN_TO_WAIT;
              length_countdown = -1;
            } else {
              if (buffer->cv_rate) {
                // Just like if FADE_DOWN_TO_RESTART had a fade_ended event.
                trig_generator.trigger(1e-3f);
                playback_position = GetPosition();
                length_countdown = GetLength(args);
              } else {
                play_state = FADE_DOWN_TO_RESTART;
                length_countdown = GetLength(args);
              }
            }
          }
        }
        break;

        case FADE_DOWN: {
          if (fade_ended && !playing) {
            play_state = NO_PLAY;
          } else if (clock_event && style_clock) {
            play_state = FADE_DOWN_TO_RESTART;
            position_for_restart = GetPosition();
            length_in_samples = GetLength(args);
          } else if (start_play && style_clock) {
            play_state = WAITING;
          } else if (fade_ended && style_clock) {
            play_state = WAITING;
          } else if (start_play && style == 1) {
            play_state = FADE_UP;
          }
        }
        break;

        case FADE_DOWN_TO_RESTART: {
          if (stop_play) {
            play_state = FADE_DOWN;
          } else if (fade_ended) {
            trig_generator.trigger(1e-3f);
            playback_position = GetPosition();
            play_state = FADE_UP;
            if (style == 1 || style == 2) {
              length_countdown = GetLength(args);
            }
          }
        }
        break;

        case FADE_DOWN_TO_WAIT: {
          if (stop_play) {
            play_state = FADE_DOWN;
          } else if (fade_ended) {
            play_state = WAITING;
          }
        }
        break;
      }

      // We're probably still moving.
      // 'movement' is combination of speed input and speed param.
      // NB: in v/oct case, we subtract the default 1.0 value for SPEED_PARAM.
      double speed = speed_is_voct ?
          std::pow(2.0, inputs[SPEED_INPUT].getVoltage() + params[SPEED_PARAM].getValue() - 1.0) :
          inputs[SPEED_INPUT].getVoltage() + params[SPEED_PARAM].getValue();
      double movement = (play_state == NO_PLAY || play_state == WAITING) ? 0.0 : speed;
      
      playback_position += movement;
      // Fix the position, now that the movement has occured.
      if (playback_position < 0.0) {
        playback_position += length;
      } else if (playback_position >= length) {
        playback_position -= length;
      }

      display_position = playback_position;

      while (display_position >= length) {
        display_position -= length;
      }
      while (display_position < 0.0) {
        display_position += length;
      }

      line_record.position = display_position;

      if (play_state != NO_PLAY && play_state != WAITING) {  // These states are silent.
        // Determine values to emit.
        double closest_head_distance = buffer->NearHead(display_position);
        if (closest_head_distance <= FADE_DISTANCE) {
          // value of fade is simply a measure of how close we are.
          // Don't let it get above 1.0.
          fade = std::min(1.0, closest_head_distance / FADE_DISTANCE);
        } else {
          fade = 1.0;
        }

        FloatPair gotten;
        buffer->Get(&gotten, display_position);

        double left = fade * play_fade * gotten.left;
        double right = fade * play_fade * gotten.right;

        // If the values we're outputting here are at or very close to zero,
        // we can end a FADE_DOWN_* immediately. 
        if ((play_state == FADE_DOWN ||
             play_state == FADE_DOWN_TO_RESTART ||
             play_state == FADE_DOWN_TO_WAIT) &&
            fabs(left) < 0.1 &&
            fabs(right) < 0.1) {
          // Doing this can make the timing slightly off? Is that bad?
          // If we're playing to length, but we keep shortening, then that could throw off beat!
          // But I think it's good when the CLOCK induces the change.
          // Logic above will change us to FADE_UP and change position.
          play_fade = 0.0;
        }
        // The equivalent for FADE_UP -> PLAYING transition.
        // Happens if the distance between the faded and unfaded values is < 0.1.
        if (play_state == FADE_UP &&
            fabs(left - gotten.left) < 0.1 &&
            fabs(right - gotten.right) < 0.1) {
          play_fade = 1.0;    
        }

        outputs[LEFT_OUTPUT].setVoltage(left);
        outputs[RIGHT_OUTPUT].setVoltage(right);
        lights[PLAY_BUTTON_LIGHT].setBrightness(1.0f);
      } else {
        outputs[LEFT_OUTPUT].setVoltage(0.0f);
        outputs[RIGHT_OUTPUT].setVoltage(0.0f);
        lights[PLAY_BUTTON_LIGHT].setBrightness(0.0f);
      }
      outputs[TRIG_OUT_OUTPUT].setVoltage(
        trig_generator.process(args.sampleTime) ? 10.0 : 0.0f);
    } else {
      // Can only be playing if connected.
      lights[PLAY_BUTTON_LIGHT].setBrightness(0.0f);
    }
    lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);

  // DO NOT SUBMIT!
  /*
  if (play_state != prev_play_state) {
    constexpr const char* state_names[] = {
      "NO_PLAY",    // * Not playing at all.
      "WAITING",
      "FADE_UP",    // * Starting to play.
      "PLAYING",     // * Continuing to play.
      "FADE_DOWN",    // * Fading out the playing -> to not playing at all.
      "FADE_DOWN_TO_RESTART", // * Fading out in order to transition to a new starting place.
      "FADE_DOWN_TO_WAIT"    // We've hit our repeat limit.
    };

    prev_play_state = play_state;
    WARN("CV = %s", buffer->cv_rate ? "true" : "false");
    WARN("New state: %s", state_names[play_state]);
    WARN("length_countdown: %d", length_countdown);
    WARN("length_in_samples: %d", length_in_samples);
    WARN("playback_position: %f", playback_position);
  }
  */
  }
};

struct FixationWidget : ModuleWidget {
  FixationWidget(Fixation* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Fixation.svg"),
                         asset::plugin(pluginInstance, "res/Fixation-dark.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.166, 15.743)), module, Fixation::CLOCK_INPUT));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.035, 25.737)), module, Fixation::POSITION_KNOB_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(15.24, 25.737)), module, Fixation::POSITION_ATTN_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.236, 25.737)), module, Fixation::POSITION_INPUT));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.035, 40.188)), module, Fixation::LENGTH_KNOB_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(15.24, 40.188)), module, Fixation::LENGTH_ATTN_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.236, 40.188)), module, Fixation::LENGTH_INPUT));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(8.575, 56.279)), module, Fixation::COUNT_KNOB_PARAM));
    RoundBlackSnapKnob* style_knob = createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(21.59, 56.279)),
        module, Fixation::STYLE_KNOB_PARAM);
    style_knob->minAngle = -0.28f * M_PI;
    style_knob->maxAngle = 0.28f * M_PI;
    addParam(style_knob);

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.59, 70.509)), module, Fixation::TRIG_OUT_OUTPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.575, 97.087)), module, Fixation::SPEED_INPUT));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(21.59, 97.087)), module, Fixation::SPEED_PARAM));

    // Play button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(21.59, 84.36)),
                                             module, Fixation::PLAY_BUTTON_PARAM,
                                             Fixation::PLAY_BUTTON_LIGHT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.575, 84.36)), module, Fixation::PLAY_GATE_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.575, 112.0)), module, Fixation::LEFT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.59, 112.0)), module, Fixation::RIGHT_OUTPUT));

    ConnectedLight* connect_light = createLightCentered<ConnectedLight>(
      mm2px(Vec(15.24, 3.0)), module, Fixation::CONNECTED_LIGHT);
    connect_light->pos_module = module;
    addChild(connect_light);
  }

  void appendContextMenu(Menu* menu) override {
    Fixation* module = dynamic_cast<Fixation*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Use Speed as V/Oct", "",
                                          &module->speed_is_voct));

    // Be a little clearer how to make this module do anything.
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel(
      "Fixation only works when touching a group of modules with a Memory"));
    menu->addChild(createMenuLabel(
      "module to the left. See my User Manual for details and usage videos."));
  }
};

Model* modelFixation = createModel<Fixation, FixationWidget>("Fixation");
