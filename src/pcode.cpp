/* Class for manipulating PCode structures.
*/

// First order of business is to turn a vector of Lines (which may have
// nested Lines) into a flat vector of PCodes.
#include <algorithm>
#include <cmath>
#include <vector>

#include "parser/tree.h"
#include "pcode.h"
#include "pcode_trans.h"

PCode PCode::Wait(const Expression &expr1) {
  PCode new_pcode;
  new_pcode.type = PCode::WAIT;
  new_pcode.expr1 = expr1;
  return new_pcode;
}

void PCode::DoArrayAssignment() {
  int index = (int) floor(expr1.Compute());
  // Nothing we can do when index is negative, and we have no runtime
  // error mechanism.
  if (index < 0) return;
  // The logic is different if we are assigning a single value vs. a list.
  // With a list, we want to ensure that _all_ of the positions we will add to
  // are available before we start.
  int required_size = index + 1;
  if (expr_list.size() > 0) {
    // foo[1] = { 6, 5, 4, 3} -> foo[1] = 6, foo[2] = 5, ...
    required_size = index + expr_list.size() + 1;
  }
  if (required_size > (int) array_ptr->size()) {
    // Need to build out the vector until we reach the point before we can add
    // this value. NB: this has potential to wreck responsiveness.
    // TODO: should i ALSO be testing capacity()?
    // Note that adding this call to reserve() had very bad CPU results.
    //array_ptr->reserve(required_size);
    array_ptr->resize(required_size, 0.0f);
  }

  // Go ahead and assign.
  if (expr_list.size() > 0) {
    for (int i = 0; i < expr_list.size(); i++) {
      array_ptr->at(index + i) = expr_list.expressions[i].Compute();
    }
  } else {
    array_ptr->at(index) = expr2.Compute();
  }
}

// Just different types than the float version.
void PCode::DoStringArrayAssignment() {
  int index = (int) floor(expr1.Compute());
  // Nothing we can do when index is negative, and we have no runtime
  // error mechanism.
  if (index < 0) return;
  // The logic is different if we are assigning a single value vs. a list.
  // With a list, we want to ensure that _all_ of the positions we will add to
  // are available before we start.
  int required_size = index + 1;
  if (expr_list.size() > 0) {
    // foo$[1] = { 6, 5, 4, 3} -> foo[1] = 6, foo[2] = 5, ...
    required_size = index + expr_list.size() + 1;
  }
  if (required_size > (int) str_array_ptr->size()) {
    // Need to build out the vector until we reach the point before we can add
    // this value. NB: this has potential to wreck responsiveness.
    // TODO: should i ALSO be testing capacity()?
    // Note that adding this call to reserve() had very bad CPU results.
    // str_array_ptr->reserve(required_size);
    str_array_ptr->resize(required_size, "");
  }

  // Go ahead and assign.
  if (expr_list.size() > 0) {
    for (int i = 0; i < expr_list.size(); i++) {
      str_array_ptr->at(index + i) = expr_list.expressions[i].ComputeString();
    }
  } else {
    str_array_ptr->at(index) = expr2.ComputeString();
  }
}

bool PCodeTranslator::BlockToCodeBlock(CodeBlock* dest, const Block &source) {
  LinesToPCode(source.lines, &(dest->pcodes));
  dest->type = source.type;
  dest->condition = source.condition;
  return true;
}

PCode PCodeTranslator::Assignment(const std::string str1, float* variable_ptr,
                                  const PortPointer &port,
                                  const Expression &expr1) {
  PCode assign;
  assign.type = PCode::ASSIGNMENT;
  assign.str1 = str1;
  assign.variable_ptr = variable_ptr;
  assign.assign_port = port;
  assign.expr1 = expr1;
  return assign;
}

PCode PCodeTranslator::StringAssignment(const std::string str1,
  std::string* str_variable_ptr, const Expression &expr1) {
  PCode assign;
  assign.type = PCode::STRING_ASSIGNMENT;
  assign.str1 = str1;
  assign.str_variable_ptr = str_variable_ptr;
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

void PCodeTranslator::AddElseifs(std::vector<int>* jump_positions,
                                 const Statements &elseifs,
                                 const Exit &innermost_loop,
                                 bool last_falls_through) {
  // Each 'elseif' is effectively an IFTHEN with no elseifs in it. I think?
  for (int i = 0; i < (int) elseifs.lines.size(); i++) {
    const Line &line = elseifs.lines[i];
    // IFNOT
    // then Statements
    // (if not the last elseif) JUMP past all elseifs
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
    // If we're not doing the last elseif, then we'll need to add a JUMP to
    // the end of the whole statement.
    if (!last_falls_through || ((int) elseifs.size()) - i > 1) {
      PCode jump_over_elseifs;
      jump_over_elseifs.type = PCode::RELATIVE_JUMP;
      pcodes->push_back(jump_over_elseifs);
      jump_positions->push_back(pcodes->size() - 1);
    }
    pcodes->at(ifnot_position).jump_count = pcodes->size() - ifnot_position;
  }
}

void PCodeTranslator::AddLineToPCode(const Line &line,
                                     const Exit &innermost_loop) {
  switch (line.type) {
    case Line::ARRAY_ASSIGNMENT: {
      PCode assign;
      assign.type = PCode::ARRAY_ASSIGNMENT;
      assign.assign_port = line.assign_port;
      assign.array_ptr = line.array_ptr;
      assign.expr1 = line.expr1;
      assign.expr2 = line.expr2;
      assign.expr_list = line.expr_list;
      pcodes->push_back(assign);
    }
    break;
    case Line::STRING_ARRAY_ASSIGNMENT: {
      PCode assign;
      assign.type = PCode::STRING_ARRAY_ASSIGNMENT;
      assign.str_array_ptr = line.str_array_ptr;
      assign.expr1 = line.expr1;
      assign.expr2 = line.expr2;
      assign.expr_list = line.expr_list;
      pcodes->push_back(assign);
    }
    break;
    case Line::ASSIGNMENT: {
      pcodes->push_back(Assignment(
          line.str1, line.variable_ptr, line.assign_port, line.expr1));
    }
    break;
    case Line::STRING_ASSIGNMENT: {
      pcodes->push_back(StringAssignment(
          line.str1, line.str_variable_ptr, line.expr1));
    }
    break;
    case Line::CLEAR: {
      PCode clear;
      // Currently no more complicated than that.
      clear.type = PCode::CLEAR;
      pcodes->push_back(clear);
    }
    break;
    case Line::RESET: {
      PCode reset;
      // Currently no more complicated than that.
      reset.type = PCode::RESET;
      pcodes->push_back(reset);
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
      // (if elseifs) JUMP past all elseifs
      // (elseifs)
      // IFNOT
      // then Statements
      // ...
      // The IFNOT's jump to the next IFNOT.
      // The JUMPs at the bottom of each set of statements passes to the end
      // of the whole structure.
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
      // All JUMP's that need to be updated to point past the whole structure.
      std::vector<int> jump_positions;
      if (line.statements[1].size() > 0) {
        // There is at least one "elseif" clause.
        // Need to put a JUMP here so the THEN case will passover the elseifs
        // I'm about to add.
        PCode jump_over_elseifs;
        jump_over_elseifs.type = PCode::RELATIVE_JUMP;
        pcodes->push_back(jump_over_elseifs);
        jump_positions.push_back(pcodes->size() - 1);
      }
      pcodes->at(ifnot_position).jump_count = pcodes->size() - ifnot_position;
      // Now add the elseifs.
      AddElseifs(&jump_positions, line.statements[1], innermost_loop, true);
      // Now resolve the jumps, if any.
      for (int position : jump_positions) {
        pcodes->at(position).jump_count = pcodes->size() - position;
      }
    }
    break;
    case Line::ELSEIF: {
      // The compiler is broken if we land here.
    }
    break;
    case Line::IFTHENELSE: {
      // IFNOT
      // then Statements
      // (if elseifs) JUMP past all elseifs
      // (elseifs)
      // IFNOT
      // then Statements
      // ...
      // RELATIVE_JUMP
      // else Statements
      // The IFNOT's jump to the next IFNOT. The last IFNOT jumps to the start
      // of the else.
      // The JUMPs at the bottom of each set of statements passes to the end
      // of the whole structure.
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
      // All JUMP's that need to be updated to point past the whole structure.
      std::vector<int> jump_positions;

      // After THEN statements, need to skip over the ELSE statements.
      PCode jump;
      jump.type = PCode::RELATIVE_JUMP;
      pcodes->push_back(jump);
      // Need to find jump later.
      jump_positions.push_back(pcodes->size() - 1);
      // Finish the ifnot.
      pcodes->at(ifnot_position).jump_count = pcodes->size() - ifnot_position;
      // Now add the elseifs.
      AddElseifs(&jump_positions, line.statements[2], innermost_loop, false);

      // Add the ELSE clause.
      for (auto &loop_line : line.statements[1].lines) {
        AddLineToPCode(loop_line, innermost_loop);
      }
      // Now resolve the jumps, if any.
      for (int position : jump_positions) {
        pcodes->at(position).jump_count = pcodes->size() - position;
      }

    }
    break;
    case Line::FORNEXT: {
      // ASSIGNMENT var = Expression
      // FORLOOP (var, expr1, expr2)
      // statements
      // WAIT 0 (iff wait_on_next is set)
      // RELATIVE_JUMP (back to FORLOOP).
      PCode assign = Assignment(line.str1, line.variable_ptr,
                                line.assign_port, line.expr1);
      // Tells the FORLOOP to re-evaluate limit
      assign.state = PCode::ENTERING_FOR_LOOP;
      pcodes->push_back(assign);
      PCode forloop;
      forloop.type = PCode::FORLOOP;
      forloop.str1 = line.str1;  // Variable name.
      forloop.assign_port = line.assign_port;
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
      if (line.wait_on_next) {
        // Insert smallest possible WAIT.
        pcodes->push_back(PCode::Wait(expression_factory.Number(0.0f)));
      }
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
    break;
    case Line::WHILE: {
      // IFNOT
      // statements
      // WAIT 0
      // RELATIVE_JUMP (back to the IFNOT).
      PCode ifnot;
      ifnot.type = PCode::IFNOT;
      ifnot.expr1 = line.expr1;
      pcodes->push_back(ifnot);
      // Need to find this IFNOT PCode later, so I can fill in jump_count
      // after adding all of the statements.
      int ifnot_position = pcodes->size() - 1;
      // TODO: changes to support EXIT WHILE and CONTINUE WHILE.
      for (auto &loop_line : line.statements[0].lines) {
        AddLineToPCode(loop_line, innermost_loop);
      }

      // Insert smallest possible WAIT.
      pcodes->push_back(PCode::Wait(expression_factory.Number(0.0f)));

      PCode jump_back;
      jump_back.type = PCode::RELATIVE_JUMP;
      // jump_count must be negative to go backwards in program to FORLOOP.
      jump_back.jump_count = ifnot_position - pcodes->size();
      pcodes->push_back(jump_back);
      // Tell the IFNOT where to go when exiting loop.
      pcodes->at(ifnot_position).jump_count =
          pcodes->size() - ifnot_position;
    }
    break;
    case Line::PRINT: {
      PCode print;
      print.type = PCode::PRINT;
      print.assign_port = line.assign_port;  // Port we are sending text out of.
      print.expr_list = line.expr_list;
      pcodes->push_back(print);
    }
    break;
    case Line::SET_CHANNELS: {
      PCode set_channels;
      set_channels.type = PCode::SET_CHANNELS;
      set_channels.assign_port = line.assign_port;  // Port we are sending text out of.
      set_channels.expr1 = line.expr1;
      pcodes->push_back(set_channels);
    }
    break;
  }
}
