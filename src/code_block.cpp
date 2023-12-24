// Logic for running a sequence of PCode objects known as a CodeBlock.

#include "code_block.h"

#include <math.h>

#include "parser/environment.h"

// There are times when the module itself needs to get or set a variable's
// value. E.g., when executing a FOR-NEXT loop.
// Now that varaibles are represented by both a float* and a PortPointer,
// we have these methods for doing those operations.
// TODO: make a class that bundles the float* and the PortPointer
// and these methods?
void CodeBlock::SetVariableValue(float* variable_ptr,
    const PortPointer &assign_port, float value) {
  if (assign_port.port_type == PortPointer::NOT_PORT) {
      *variable_ptr = value;
  } else {
    environment->SetVoltage(assign_port, value);
  }
}

float CodeBlock::GetVariableValue(float* variable_ptr,
    const PortPointer &port) {
  if (port.port_type == PortPointer::NOT_PORT) {
    return *variable_ptr;
  } else {
    return environment->GetVoltage(port);
  }
}

CodeBlock::RunStatus CodeBlock::Run(bool loops) {
  // Recompute the wait time, but only if we _need_ to.
  // Consumed by the PCode::WAIT instruction.
  bool need_to_update_wait = false;
  // Need to determine if:
  // * We need to update the inputs at all.
  // * We are in a WAIT *and* we need to recompute the wait time.
  // These are related questions.
  if (wait_info.in_wait && wait_info.is_volatile) {
    // In a WAIT, and the wait interval relies on things that change?
    wait_info.countdown_to_recompute--;
    if (wait_info.countdown_to_recompute <= 0) {
      wait_info.countdown_to_recompute = std::floor(samples_per_millisecond);
      need_to_update_wait = true;
    }
  }

  // Run the PCode vector from the current spot in it.
  bool waiting = false;
  bool running = true;

  while (running && !waiting) {
    PCode* pcode = &(pcodes[current_line]);
    switch (pcode->type) {
      case PCode::ARRAY_ASSIGNMENT: {
        pcode->DoArrayAssignment();
        current_line++;
      }
      break;
      case PCode::ASSIGNMENT: {
        float rhs = pcode->expr1.Compute();
        SetVariableValue(pcode->variable_ptr,
                         pcode->assign_port, rhs);
        current_line++;
      }
      break;
      case PCode::CLEAR: {
        environment->Clear();
        current_line++;
      }
      break;
      case PCode::RESET: {
        environment->Reset();
        run_status = RAN_RESET;
        return run_status;
      }
      break;
      case PCode::WAIT: {
        if (wait_info.in_wait) {
          // We're currently running through this statement's wait period.
          // Update it by one tick.
          wait_info.ticks_so_far++;
          // If the wait period may have changed, recompute it.
          if (need_to_update_wait) {
            wait_info.ticks_limit = std::floor(
                pcode->expr1.Compute() * samples_per_millisecond);
          }
          if (wait_info.ticks_so_far >= wait_info.ticks_limit) {
            // WAIT has completed, immediately execute next line.
            wait_info.in_wait = false;
            current_line++;
          }
          // We set 'waiting' after this IF.
        } else {
          // Just arriving at this WAIT statement.
          int ticks = std::floor(
              pcode->expr1.Compute() * samples_per_millisecond);
          // A "WAIT 0" (or WAIT -1!) means we should stop running for
          // this process() call but push to the next line. No reason to
          // create a WaitInfo.
          if (ticks <= 0) {
            current_line++;
            waiting = true;
          } else {
            // This WAIT is longer than a single tick, so we should fill in
            // the WaitInfo for it.
            wait_info.is_volatile = pcode->expr1.Volatile();
            // It wastes a lot of CPU seeing if we need to recompute every tick.
            // We instead only consider doing so every millisecond.
            wait_info.countdown_to_recompute = std::floor(
              samples_per_millisecond);
            wait_info.ticks_limit = ticks;
            wait_info.ticks_so_far = 0;
            wait_info.in_wait = true;
          }
        }
        if (wait_info.in_wait) {
          waiting = true;
        }
      }
      break;
      case PCode::IFNOT: {
        // All this PCode does is determine where to move current_line to.
        bool expr_val = !Expression::is_zero(pcode->expr1.Compute());
        if (!expr_val) {
          current_line += pcode->jump_count;
        } else {
          current_line++;
        }
      }
      break;
      case PCode::RELATIVE_JUMP: {
        if (pcode->stop_execution) {
          current_line = 0;
          wait_info.in_wait = false;
          running = false;
        } else {
          // This just specifies a jump of the current_line.
          current_line += pcode->jump_count;
        }
      }
      break;
      case PCode::FORLOOP: {
        float loop_var_value;
        if (state == PCode::ENTERING_FOR_LOOP) {
          loop_var_value = GetVariableValue(pcode->variable_ptr,
              pcode->assign_port);
          pcode->limit = pcode->expr1.Compute();
          pcode->step = pcode->expr2.Compute();
        } else {
          loop_var_value = GetVariableValue(pcode->variable_ptr,
              pcode->assign_port) + pcode->step;
          SetVariableValue(pcode->variable_ptr,
            pcode->assign_port, loop_var_value);
        }
        bool done = false;
        // If "Step" is negative, we wait until value is _below_ limit.
        if (pcode->step >= 0.0f) {
          done = loop_var_value > pcode->limit;
        } else {
          done = loop_var_value < pcode->limit;
        }
        if (done) {
          current_line += pcode->jump_count;
        } else {
          current_line++;
        }
      }
      break;
      case PCode::PRINT: {
        // Get the string values of expr_list, and concatenate them together.
        std::string result;
        for (Expression expr : pcode->expr_list.expressions) {
          result.append(expr.ComputeString());
        }
        // Note: once tested that result was not empty, but sending empty string
        // could be intended behavior, I realize.
        environment->Send(pcode->assign_port, result);
        current_line++;
      }
      break;
    }
    // pcode->state is only set in a select few situations, like the assignment
    // before a FORLOOP.
    state = pcode->state;
    if (current_line >= pcodes.size()) {
      current_line = 0;
      waiting = true;  // Implicit WAIT at end of program.
      if (!loops) {
        running = false;
      }
    }
  }
  run_status = running ? CONTINUES : STOPPED;
  return run_status;
}
