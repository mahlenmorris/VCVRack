#ifndef PCODE_TRANS_H
#define PCODE_TRANS_H

#include "code_block.h"
#include "parser/tree.h"

// Class for turning nested vector of Line objects into a flat vector of
// PCode objects.
class PCodeTranslator {
public:
  PCodeTranslator() { }
  // TODO: This should return a vector of Error objects, so that we can
  // prevent running when there are errors and report them.
  bool BlockToCodeBlock(CodeBlock* dest, const Block &source);
  void LinesToPCode(const std::vector<Line> &lines, std::vector<PCode> *pcodes);
  void AddElseifs(std::vector<int>* jump_positions,
                  const Statements &elseifs, const Exit &innermost_loop,
                  bool last_falls_through);
  PCode Assignment(const std::string str1, float* variable_ptr,
                   const PortPointer &port, const Expression &expr1);
  PCode StringAssignment(const std::string str1,
                         std::string* str_variable_ptr, const Expression &expr1);

private:
  void AddLineToPCode(const Line &line, const Exit &innermost_loop);
  // Just useful for making Number expressions.
  ExpressionFactory expression_factory;
  std::vector<PCode> *pcodes;
  std::vector<Loop> loops;
  std::vector<Exit> exits;
};

#endif // PCODE_TRANS_H
