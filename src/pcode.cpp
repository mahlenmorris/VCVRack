/* Class for manipulating PCode structures.
*/

// First order of business is to turn a vector of Lines (which may have
// nested Lines) into a flat vector of PCodes.
#include <algorithm>
#include <vector>

#include "parser/tree.hh"
#include "pcode.h"

PCode PCode::Assignment(const std::string str1, const Expression &expr1) {
  PCode assign;
  assign.type = PCode::ASSIGNMENT;
  assign.str1 = str1;
  assign.expr1 = expr1;
  return assign;
}

PCode PCode::Wait(const Expression &expr1) {
  PCode new_pcode;
  new_pcode.type = PCode::WAIT;
  new_pcode.expr1 = expr1;
  return new_pcode;
}

// Note; caller should _probably_ reset the line position after this.
// And should definitely NOT access 'pcodes' while this is running.
void PCodeTranslator::LinesToPCode(const std::vector<Line> &lines,
                                   std::vector<PCode> *new_pcodes) {
  pcodes = new_pcodes;
  pcodes->clear();
  for (auto &line : lines) {
    AddLineToPCode(line);
  }
}

std::string PCode::to_string() {
  return "PCode(" + std::to_string(type) + ", " + std::to_string(jump_count) +
    ")";
}

void PCodeTranslator::AddLineToPCode(const Line &line) {
  switch (line.type) {
    case Line::ASSIGNMENT: {
      pcodes->push_back(PCode::Assignment(line.str1, line.expr1));
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
      new_pcode.bool1 = line.bool1;
      pcodes->push_back(new_pcode);
      // Need to find this IFNOT PCode later, so I can fill in jump_count
      // after adding all of the statements.
      int ifnot_position = pcodes->size() - 1;
      // Add all of the THEN-clause Lines. Note that some of these might
      // also be control-flow Lines of unknown PCode length.
      for (auto &loop_line : line.statements[0].lines) {
        AddLineToPCode(loop_line);
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
      ifnot.bool1 = line.bool1;
      pcodes->push_back(ifnot);
      // Need to find this IFNOT PCode later, so I can fill in jump_count
      // after adding all of the statements.
      int ifnot_position = pcodes->size() - 1;
      // Add all of the THEN-clause Lines. Note that some of these might
      // also be control-flow Lines of unknown PCode length.
      for (auto &loop_line : line.statements[0].lines) {
        AddLineToPCode(loop_line);
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
        AddLineToPCode(loop_line);
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
      PCode assign = PCode::Assignment(line.str1, line.expr1);
      // Tells the FORLOOP to re-evaluate limit
      assign.state = PCode::ENTERING_FOR_LOOP;
      pcodes->push_back(assign);
      PCode forloop;
      forloop.type = PCode::FORLOOP;
      forloop.str1 = line.str1;  // Variable name.
      forloop.expr1 = line.expr2;  // Limit.
      forloop.expr2 = line.expr3;  // Step.
      pcodes->push_back(forloop);
      // Need to find this FORLOOP PCode later, so I can fill in jump_count
      // after adding all of the statements.
      int forloop_position = pcodes->size() - 1;
      // Add to stack.
      loops.push_back(Loop("for", forloop_position));
      for (auto &loop_line : line.statements[0].lines) {
        AddLineToPCode(loop_line);
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
      pcodes->at(forloop_position).jump_count = pcodes->size() - forloop_position;
    }
  }
}
