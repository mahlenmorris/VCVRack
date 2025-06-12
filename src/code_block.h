#ifndef CODE_BLOCK_H
#define CODE_BLOCK_H

// For Basically, defines a block of code and the infrastructure required
// to execute it in a re-entrant way.
#include <vector>

#include "parser/environment.h"  // Reading and writing module data.
#include "pcode.h"
#include "parser/tree.h"

// The execution of WAIT statements needs to be as efficient as I can make them.
// This data structure just collects all data about the ongoing WAIT, if any.
struct WaitInfo {
  int countdown_to_recompute;
  // Are we currently in a WAIT?
  bool in_wait;
  // The expression that computes the wait time could change while the wait is
  // occuring.
  bool is_volatile;
  // The number of process() calls seen in this WAIT. Goes up by one every time
  // procees is called during the wait.
  int ticks_so_far;
  // The number of ticks needed to complete the wait. If is_volatile, then
  // this needs to be recomputed when INn gets changed.
  int ticks_limit;
};

struct CodeBlock {
  enum RunStatus {
    CONTINUES,  // This block will continue running.
    STOPPED,    // This block has hit the end.
    RAN_RESET   // This block ran RESET, must bail on this sample.
  };

  // The "bytecode" instructions we follow.
  std::vector<PCode> pcodes;
  // Line where execution is currently happening.
  unsigned int current_line;
  // Some PCodes have different behaviors, depending on how execution got
  // there. 'state' helps determine the correct behavior.
  PCode::State state;
  // Efficient way to get through a WAIT instruction.
  WaitInfo wait_info;
  float samples_per_millisecond;
  Environment* environment;
  Block::Type type;
  Block::Condition condition;
  // Some (not all) STYLE settings mean that we should track the
  // running/not running status on a per-block level.
  RunStatus run_status;

  explicit CodeBlock(Environment* env) : environment{env} {
    wait_info.in_wait = false;
    current_line = 0;
    samples_per_millisecond = env->SampleRate() / 1000.0f;
    run_status = CONTINUES;
  }

  // There are times when the module itself needs to get or set a variable's
  // value. E.g., when executing a FOR-NEXT loop.
  // Now that variables are represented by either a float* or a PortPointer,
  // we have these methods for doing those operations.
  // TODO: make a class that bundles the float* and the PortPointer
  // and these methods?
  void SetVariableValue(float* variable_ptr,
     const PortPointer &assign_port, float value);

  // Just for setting values in an OUTn[]. "SingleValue" because this
  // doesn't handle assigning multiple values to a range of an array.
  void SetOUTSingleValue(const PortPointer &assign_port, int channel, float value);
  float GetVariableValue(float* variable_ptr, const PortPointer &port);

  // Only called when the global "running" status of the program is true.
  // Returns STOPPED if the block believes we should stop running.
  // TODO: how to handle EXIT ALL? Does this stop *all* blocks? Just this one? Need a new gesture
  // to just stop one block? EXIT BLOCK?
  CodeBlock::RunStatus Run(bool loops);

  void DebugPrint() {
    std::cout << "current_line = " << current_line << "\n";
    for (PCode pcode : pcodes) {
      std::cout << pcode.to_string() << "\n";
    }
  }
};

#endif // CODE_BLOCK_H
