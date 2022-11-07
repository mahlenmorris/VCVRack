#ifndef PCODE_H
#define PCODE_H

/*
The structures that make up the pseudocode that gets run during a call
to process(). Needs to be
* interruptible and efficient, because process() isn't allowed to run for long
* always have a position (e.g., line number) that we can pick up execution at.
The Line's that the Parser created are turned into these instructions.
For some Line types, this means simply embedding them in the PCode (e.g.,
ASSIGN, WAIT).
For structural Line types (IFTHEN, FORTO), we need to decompose the Line
into jumps and conditional jumps, so that the Lines embedded within the
structure are properly flattened into the whole program.
*/

// For now, it's more efficient to have the code that wants the results
// do the work of figuring out what to do for each instruction type.
// So I'm just making this one big structure.

struct PCode {
  enum Type {
    ASSIGNMENT,  // str1 = expr1
    WAIT,        // wait expr1
    IFNOT,       // ifnot bool1 jump jump_count
    RELATIVE_JUMP,  // add jump_count to line_number.
    FORLOOP      // On ENTERING_FOR_LOOP, compute limit (expr1) & step (expr2).
                 // if beyond limit, jump jump_count.
                 // On NONE, add step to str1 value.
                 // if beyond limit, jump jump_count.
  };
  Type type;
  std::string str1;
  Expression expr1, expr2;
  BoolExpression bool1;
  int jump_count;
  float step, limit;
  enum State {
    NONE,
    ENTERING_FOR_LOOP
  };
  State state;

  PCode() {
    jump_count = 0;
    state = NONE;
  };

  static void LinesToPCode(const std::vector<Line> &lines,
                           std::vector<PCode> *pcodes);
  static void AddLineToPCode(const Line &line, std::vector<PCode> *pcodes);
  static PCode Assignment(const std::string str1, const Expression &expr1);
  static PCode Wait(const Expression &expr1);
  std::string to_string();
};

#endif // PCODE_H
