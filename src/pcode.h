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
    ASSIGNMENT,  // *variable_ptr = expr1
    WAIT,        // wait expr1
    IFNOT,       // ifnot expr1 jump jump_count
    RELATIVE_JUMP,  // add jump_count to line_number.
    FORLOOP      // On ENTERING_FOR_LOOP, compute limit (expr1) & step (expr2).
                 // if beyond limit, jump jump_count.
                 // On NONE, add step to str1 value.
                 // if beyond limit, jump jump_count.
  };
  Type type;
  std::string str1;
  // For assignments to variables/ports.
  float* variable_ptr;
  PortPointer assign_port;
  Expression expr1, expr2;
  int jump_count;
  float step, limit;
  bool stop_execution;
  enum State {
    NONE,
    // When a FORLOOP line receives ENTERING_FOR_LOOP, it must Compute() the
    // step and limit values.
    ENTERING_FOR_LOOP
  };
  State state;

  PCode() {
    jump_count = 0;
    state = NONE;
    stop_execution = false;
  };

  static PCode Wait(const Expression &expr1);
  std::string to_string();
};

// Helps resolve "continue" statements.
struct Loop {
  const std::string loop_type;  // E.g., "for".
  const int line_number;        // Position of the loop start (e.g., FORLOOP).
  Loop(const std::string type, int line) : loop_type{type}, line_number{line} {}
};

// Helps resolve "exit" statements.
struct Exit {
  const std::string exit_type;  // E.g., "for".
  // Position of the unfinished RELATIVE_JUMP for this "exit" statement.
  int exit_line_number;
  // Position of the loop start (e.g., FORLOOP) that this exit statement
  // is exiting.
  const int loop_start_Line_number;
  // When an Exit is created, we typically do not know the exit_line_number.
  Exit(const std::string type, int loop_pos) : exit_type{type},
      loop_start_Line_number{loop_pos} {}
};

// Class for turning nested vector of Line objects into a flat vector of
// PCode objects.
class PCodeTranslator {
public:
  PCodeTranslator() { }
  // TODO: This should return a vector of Error objects, so that we can
  // prevent running and report them.
  void LinesToPCode(const std::vector<Line> &lines, std::vector<PCode> *pcodes);
  PCode Assignment(const std::string str1, float* variable_ptr,
                   const PortPointer &port, const Expression &expr1);

private:
  void AddLineToPCode(const Line &line, const Exit &innermost_loop);
  // Just useful for making Number expressions.
  ExpressionFactory expression_factory;
  std::vector<PCode> *pcodes;
  std::vector<Loop> loops;
  std::vector<Exit> exits;
};

#endif // PCODE_H
