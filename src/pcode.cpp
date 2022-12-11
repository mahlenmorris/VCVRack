/* Class for manipulating PCode structures.
*/

// First order of business is to turn a vector of Lines (which may have
// nested Lines) into a flat vector of PCodes.
#include <algorithm>
#include <vector>

#include "parser/tree.h"
#include "pcode.h"

PCode PCode::Wait(const Expression &expr1) {
  PCode new_pcode;
  new_pcode.type = PCode::WAIT;
  new_pcode.expr1 = expr1;
  return new_pcode;
}

PCode PCodeTranslator::Assignment(const std::string str1, float* variable_ptr,
                                  const Expression &expr1) {
  PCode assign;
  assign.type = PCode::ASSIGNMENT;
  assign.str1 = str1;
  // Optimize a bit for assignments to OUTn values.
  auto found = out_map.find(str1);
  if (found != out_map.end()) {
    assign.out_enum_value = found->second;
  } else {
    assign.out_enum_value = -1;
  }
  assign.variable_ptr = variable_ptr;
  assign.expr1 = expr1;
  return assign;
}

// Note; caller should _probably_ reset the line position after this.
// And should definitely NOT access 'pcodes' while this is running.
void PCodeTranslator::LinesToPCode(const std::vector<Line> &lines,
                                   std::vector<PCode> *new_pcodes) {
  pcodes = new_pcodes;
  pcodes->clear();
  loops.clear();
  exits.clear();
  Exit dummy_exit("dummy", -1);
  for (auto &line : lines) {
    AddLineToPCode(line, dummy_exit);
  }
}

std::string PCode::to_string() {
  return "PCode(" + std::to_string(type) + ", " + std::to_string(jump_count) +
    ")";
}

void PCodeTranslator::AddLineToPCode(const Line &line,
                                     const Exit &innermost_loop) {
  switch (line.type) {
    case Line::ASSIGNMENT: {
      pcodes->push_back(Assignment(
          line.str1, line.variable_ptr, line.expr1));
    }
    break;
    case Line::CONTINUE: {
      // See what type of continue this is, and then see what the latest
      // loop of that type I can find. Then jump to it.
      if (line.str1 == "all") {
        // Jump to beginning of program.
        PCode jump_back;
        jump_back.type = PCode::RELATIVE_JUMP;
        // jump_count must be negative to go backwards in program to FORLOOP.
        jump_back.jump_count = 0 - pcodes->size();
        pcodes->push_back(jump_back);
      } else {
        std::string loop_type = line.str1;
        auto result = std::find_if(loops.rbegin(), loops.rend(),
            [loop_type](Loop l) { return loop_type == l.loop_type; });
        if (result != loops.rend()) {  // Found it, which is to be expected.
          int jump_to = result->line_number;
          PCode jump_back;
          jump_back.type = PCode::RELATIVE_JUMP;
          // jump_count must be negative to go backwards in program to FORLOOP.
          jump_back.jump_count = jump_to - pcodes->size();
          pcodes->push_back(jump_back);
        } else {
          // This is really an error, but we'll note it and let it go for now?
          //INFO("BASICally error: 'continue for' statement seen outside of for loop.");
          // TODO: Maybe give a way for this to register errors later?
        }
      }
    }
    break;
    case Line::EXIT: {
      // Like a CONTINUE, but different. We place a RELATIVE_JUMP, but we don't
      // know where to jump to until we've placed the entire loop.
      // So, the loop will look in 'exits' for any
      // unfinished EXIT lines it needs to complete.
      // Add distinct behavior for "exit all".
      PCode jump_out;
      jump_out.type = PCode::RELATIVE_JUMP;
      if (line.str1 == "all") {
        // Tell program to stop running.
        jump_out.stop_execution = true;
        // TODO: I think runner will reset the line to zero.
        pcodes->push_back(jump_out);
      } else {
        Exit this_exit(innermost_loop);
        this_exit.exit_line_number = pcodes->size();
        pcodes->push_back(jump_out);
        exits.push_back(this_exit);
      }
    }
    break;
    case Line::WAIT: {
      pcodes->push_back(PCode::Wait(line.expr1));
    }
    break;
    case Line::IFTHEN: {
      // IFNOT
      // then Statements
      // ...
      PCode new_pcode;
      new_pcode.type = PCode::IFNOT;
      new_pcode.expr1 = line.expr1;
      pcodes->push_back(new_pcode);
      // Need to find this IFNOT PCode later, so I can fill in jump_count
      // after adding all of the statements.
      int ifnot_position = pcodes->size() - 1;
      // Add all of the THEN-clause Lines. Note that some of these might
      // also be control-flow Lines of unknown PCode length.
      for (auto &loop_line : line.statements[0].lines) {
        AddLineToPCode(loop_line, innermost_loop);
      }
      pcodes->at(ifnot_position).jump_count = pcodes->size() - ifnot_position;
    }
    break;
    case Line::IFTHENELSE: {
      // IFNOT
      // then Statements
      // ...
      // RELATIVE_JUMP
      // else Statements
      PCode ifnot;
      ifnot.type = PCode::IFNOT;
      ifnot.expr1 = line.expr1;
      pcodes->push_back(ifnot);
      // Need to find this IFNOT PCode later, so I can fill in jump_count
      // after adding all of the statements.
      int ifnot_position = pcodes->size() - 1;
      // Add all of the THEN-clause Lines. Note that some of these might
      // also be control-flow Lines of unknown PCode length.
      for (auto &loop_line : line.statements[0].lines) {
        AddLineToPCode(loop_line, innermost_loop);
      }
      // After THEN statements, need to skip over the ELSE statements.
      PCode jump;
      jump.type = PCode::RELATIVE_JUMP;
      pcodes->push_back(jump);
      // Need to find jump later.
      int jump_position = pcodes->size() - 1;
      // Finish the ifnot.
      pcodes->at(ifnot_position).jump_count = pcodes->size() - ifnot_position;
      // Add the ELSE clause.
      for (auto &loop_line : line.statements[1].lines) {
        AddLineToPCode(loop_line, innermost_loop);
      }
      // Finish the jump.
      pcodes->at(jump_position).jump_count = pcodes->size() - jump_position;
    }
    break;
    case Line::FORNEXT: {
      // ASSIGNMENT var = Expression
      // FORLOOP (var, expr1, expr2)
      // statements
      // WAIT 0
      // RELATIVE_JUMP (back to FORLOOP).
      PCode assign = Assignment(line.str1, line.variable_ptr, line.expr1);
      // Tells the FORLOOP to re-evaluate limit
      assign.state = PCode::ENTERING_FOR_LOOP;
      pcodes->push_back(assign);
      PCode forloop;
      forloop.type = PCode::FORLOOP;
      forloop.str1 = line.str1;  // Variable name.
      forloop.variable_ptr = line.variable_ptr;
      forloop.expr1 = line.expr2;  // Limit.
      forloop.expr2 = line.expr3;  // Step.
      pcodes->push_back(forloop);
      // Need to find this FORLOOP PCode later, so I can fill in jump_count
      // after adding all of the statements.
      int forloop_position = pcodes->size() - 1;
      // Add to stack.
      loops.push_back(Loop("for", forloop_position));
      // Any "exit for" statements we add must be pointed back to the end of
      // *this* loop.
      Exit exit("for", forloop_position);
      for (auto &loop_line : line.statements[0].lines) {
        AddLineToPCode(loop_line, exit);
      }
      // Remove from stack.
      loops.pop_back();  // TODO: confirm it is the "for" item we placed?
      // Insert smallest possible WAIT.
      pcodes->push_back(PCode::Wait(Expression::Number(0.0f)));
      PCode jump_back;
      jump_back.type = PCode::RELATIVE_JUMP;
      // jump_count must be negative to go backwards in program to FORLOOP.
      jump_back.jump_count = forloop_position - pcodes->size();
      pcodes->push_back(jump_back);
      // Tell the FORLOOP where to go when exiting loop.
      pcodes->at(forloop_position).jump_count =
          pcodes->size() - forloop_position;
      // Now resolve any relevant EXIT jumps.
      for (Exit exit : exits) {
        if (exit.exit_type == "for" &&
            exit.loop_start_Line_number == forloop_position) {
          pcodes->at(exit.exit_line_number).jump_count =
              pcodes->size() - exit.exit_line_number;
          // We _could_ erase this item in 'exits', but that invalidates
          // iterators, so we'll just leave it. It won't get matched again.
        }
      }
    }
  }
}
