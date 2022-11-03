/* Class for manipulating PCode structures.
*/

// First order of business is to turn a vector of Lines (which may have
// nested Lines) into a flat vector of PCodes.

#include <vector>

#include "parser/tree.hh"
#include "pcode.h"

// Note; caller should _probably_ reset the line position after this.
// And should definitely NOT access pcodes while this is running.
void PCode::LinesToPCode(const std::vector<Line> &lines,
                         std::vector<PCode> *pcodes) {
  pcodes->clear();
  for (auto &line : lines) {
    AddLineToPCode(line, pcodes);
  }
}

void PCode::AddLineToPCode(const Line &line, std::vector<PCode> *pcodes) {
  switch (line.type) {
    case Line::ASSIGNMENT: {
      PCode new_pcode;  // TODO: Should pcodes be vector of Pcode*?
      new_pcode.type = PCode::ASSIGNMENT;
      new_pcode.str1 = line.str1;
      new_pcode.expr1 = line.expr1;
      pcodes->push_back(new_pcode);
    }
    break;
    case Line::WAIT: {
      PCode new_pcode;  // TODO: Should pcodes be vector of Pcode*?
      new_pcode.type = PCode::WAIT;
      new_pcode.expr1 = line.expr1;
      pcodes->push_back(new_pcode);
    }
    break;
    case Line::IFTHEN: {
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
        AddLineToPCode(loop_line, pcodes);
      }
      pcodes->at(ifnot_position).jump_count = pcodes->size() - ifnot_position;
    }
    break;
  }
}