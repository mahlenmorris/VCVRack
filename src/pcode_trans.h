#ifndef PCODE_TRANS_H
#define PCODE_TRANS_H

#include "code_block.h"
#include "parser/driver.hh"
#include "parser/tree.h"

// Class for turning nested vector of Line objects into a flat vector of
// PCode objects.
class PCodeTranslator {
 public:
  explicit PCodeTranslator(Driver* the_driver)
      : pcodes{nullptr}, driver{the_driver} {
    ExpressionFactory factory;
    zero = factory.Number(0.0f);
  }
  bool BlockToCodeBlock(CodeBlock* dest, const Block& source);
  void LinesToPCode(const std::vector<Line>& lines, std::vector<PCode>* pcodes);
  void AddElseifs(std::vector<int>* jump_positions, const Statements& elseifs,
                  bool last_falls_through);
  PCode Assignment(float* variable_ptr, const PortPointer& port,
                   const Expression& expr1);
  PCode StringAssignment(std::string* str_variable_ptr,
                         const Expression& expr1);

 private:
  void AddLineToPCode(const Line& line);
  // Just useful for making Number expressions.
  Expression zero;
  std::vector<PCode>* pcodes;
  std::vector<Loop> loops;
  std::vector<Exit> exits;
  Driver* driver;
};

#endif  // PCODE_TRANS_H
