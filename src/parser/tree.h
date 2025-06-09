#ifndef TREE_HH
#define TREE_HH

#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "environment.h"

typedef std::vector<float> FloatArray;
typedef std::vector<std::string> StringArray;

class Expression;
class Driver;
// Base class for computing expressions.
class Expression {
 public:
  enum Type {
    NUMBER,  // 3, 4.5, -283823
    STRING,  // "hello, world"
    BINOP,   // plus, times
    VARIABLE, // in1, out1, foo
    STRING_VARIABLE, // foo$
    ARRAY_VARIABLE, // array[subexpressions[0]]
    STRING_ARRAY_VARIABLE, // array$[subexpressions[0]]
    NOT,      // not bool
    ZEROARGFUNC, // operation
    ONEARGFUNC, // operation (subexpressions[0])
    ONEPORTFUNC, // operation (port)
    TWOARGFUNC, // func2(subexpressions[0], subexpressions[1])
    TERNARYFUNC, // subexpressions[0] ? subexpressions[1] : subexpressions[2]
    STRINGFUNC  // Depends on function.
  };
  Type type;
  // Which method/operation is this?
  enum Operation {
    PLUS,
    MINUS,
    TIMES,
    DIVIDE,
    EQUAL,
    NOT_EQUAL,
    GT,
    GTE,
    LT,
    LTE,
    AND,
    OR,
    ABS,
    CEILING,
    CHANNELS,
    CONNECTED,
    DEBUG,
    FLOOR,
    LOG2,
    LOGE,
    LOG10,
    NORMAL,
    RANDOM,
    SAMPLE_RATE,
    SIGN,
    SIN,
    START,
    TIME,
    TIME_MILLIS,
    TRIGGER,
    MOD,
    MAX,
    MIN,
    POW
  };
  Operation operation;
  float float_value;
  std::string string_value;
  // Some variables are just pointers to a float (e.g., i, foo, etc.).
  float* variable_ptr;
  // Some variables are just pointers to a string (e.g., i$, foo$, etc.).
  std::string* str_variable_ptr;
  // But other variables are Input or Output ports in the UI. We can avoid
  // always updating them by pointing to their location in the Environment.
  PortPointer port;
  // And ARRAY_VARIABLE has a pointer to the array it getting data from.
  FloatArray* array_ptr;
  StringArray* str_array_ptr;
  Environment* env = nullptr;

  std::string name;
  std::vector<Expression> subexpressions;

  static std::unordered_map<std::string, float> note_to_volt_octave_4;
  Expression() : variable_ptr{nullptr}, str_variable_ptr{nullptr}, array_ptr{nullptr},
                 str_array_ptr{nullptr} {}

  // Compute the float numeric result of this Expression.
  float Compute();

  // Compute the string version of this expression.
  std::string ComputeString();
  // Determine if this Expression can Compute() different results depending on
  // INn or any other volatile source (e.g., a random() function.)
  bool Volatile();

  static bool is_zero(float value);

  // Don't force users to understand that comparing floats is hard.
  static bool float_equal(float f1, float f2);

  // Bison seems to require this; I don't use it.
  friend std::ostream& operator<<(std::ostream& os, const Expression &ex);
  std::string to_string() const;
 private:
  float bool_to_float(bool value);
  float binop_compute();
  float zero_arg_compute();
  float one_arg_compute(float arg1);
  float two_arg_compute(float arg1, float arg2);
};

class ExpressionFactory {
 public:
  Environment* env;  // Only public for my debugging.
  void SetEnvironment(Environment* the_env) { env = the_env; }
  Expression Not(const Expression &expr);
  Expression Note(const std::string &note_name);
  Expression Number(float the_value);
  Expression Quoted(const std::string &quoted);
  Expression ZeroArgFunc(const std::string &func_name);
  Expression OneArgFunc(const std::string &func_name,
                        const Expression &arg1);
  Expression OnePortFunc(const std::string &func_name, const std::string &port1,
                         Driver* driver);
  Expression TwoArgFunc(const std::string &func_name,
                        const Expression &arg1, const Expression &arg2);
  Expression TernaryFunc(const Expression &condition, const Expression &if_true,
                         const Expression &if_false);
  Expression CreateBinOp(const Expression &lhs,
                         const std::string &op_string,
                         const Expression &rhs);
  Expression ArrayVariable(const std::string &array_name,
                           const Expression &arg1,
                           Driver* driver);
  Expression StringArrayVariable(const std::string &array_name,
                                 const Expression &arg1,
                                 Driver* driver);
  Expression Variable(const char *var_name, Driver* driver);
  // The parser seems to need many variants of Variable.
  Expression Variable(const std::string &expr, Driver* driver);
  Expression Variable(char * var_name, Driver* driver);
  Expression StringVariable(const std::string& var_name, Driver* driver);
  Expression DebugId(const std::string &var_name, Driver* driver);
  Expression DebugId(const std::string &var_name, const Expression &start,
                     const Expression &end, Driver* driver);
  // For String variables.
  Expression DebugIdString(const std::string &var_name, Driver* driver);
  Expression DebugIdString(const std::string &var_name, const Expression &start,
                           const Expression &end, Driver* driver);
 private:
  static std::unordered_map<std::string, Expression::Operation> string_to_operation;
};

struct ExpressionList {
  std::vector<Expression> expressions;
  bool is_strings;

  ExpressionList() {
    is_strings = false;
  }

  static ExpressionList StringList() {
    ExpressionList list;
    list.is_strings = true;
    return list;
  }

  ExpressionList(Expression new_expr) {
    expressions.push_back(new_expr);
  }

  ExpressionList add(Expression new_expr) {
    expressions.push_back(new_expr);
    return *this;
  }
  int size() const {
    return expressions.size();
  }
  friend std::ostream& operator<<(std::ostream& os, ExpressionList exprs) {
    os << "ExpressionList(" << std::to_string(exprs.size()) << " Expressions )";
    return os;
  }
};

struct Statements;

struct Line {
  enum Type {
    ARRAY_ASSIGNMENT,  // array_ptr[expr1] = expr2
    STRING_ARRAY_ASSIGNMENT,  // str_array_ptr[expr1] = expr2
    ASSIGNMENT,        // str1 = expr1
    STRING_ASSIGNMENT, // str1$ = expr1
    CLEAR,             // Set variables to initial state (0.0f).
    CONTINUE,          // continue str1
    ELSEIF,            // elseif expr1 then state1
    EXIT,              // exit str1
    FORNEXT,           // for str1 = expr1 to expr2 state1 next
    IFTHEN,            // if expr1 then state1 [elseifs - state2] end if
    IFTHENELSE,        // if expr1 then state1 [elseifs - state3] else state2 end if
    PRINT,             // print(out1, ...)
    RESET,             // Start all blocks from the top, as if newly compiled.
    SET_CHANNELS,      // Set number of channels on an OUTn port.
    WAIT,              // wait expr1
    WHILE              // while ... end while
  };
  Type type;
  std::string str1;

  // When assigning to a variable/string variable/port/array.
  float* variable_ptr;
  std::string* str_variable_ptr;
  PortPointer assign_port;
  FloatArray* array_ptr;
  StringArray* str_array_ptr;

  Expression expr1, expr2, expr3;
  ExpressionList expr_list;
  std::vector<Statements> statements;
  bool wait_on_next;  // to distinguish NEXT from NEXTHIGHCPU.

  static Line ArrayAssignment(const std::string &variable_name,
                              const Expression &index,
                              const Expression &value, Driver* driver);

  static Line ArrayAssignment(const std::string &variable_name,
                              const Expression &index,
                              const ExpressionList &values, Driver* driver);

  static Line StringArrayAssignment(const std::string &variable_name,
                                    const Expression &index,
                                    const Expression &value, Driver* driver);

  static Line StringArrayAssignment(const std::string &variable_name,
                                    const Expression &index,
                                    const ExpressionList &values, Driver* driver);

  // identifiers on both right hand and left hand side of = look the same.
  // So the lhs will get turned into a VariableExpression. We need to pull
  // the name out of it.
  static Line Assignment(const std::string &variable_name,
                         const Expression &expr, Driver* driver);

  // str_variable_name does NOT contain the $. I'm going to leave the '$'s
  // out of the names; string and float variables live in completely separate namespaces,
  // which the parser has sufficient information to determine. I hope.
  static Line StringAssignment(const std::string &str_variable_name,
                               const Expression &expr, Driver* driver);

  static Line ClearAll();

  // loop_type is the string identifying the loop type; e.g., "for" or "all".
  static Line Continue(const std::string &loop_type);

  static Line ElseIf(const Expression &bool_expr,
                     const Statements &state1);

  // loop_type is the string identifying the loop type; e.g., "for" or "all".
  static Line Exit(const std::string &loop_type);

  static Line ForNext(const Line &assign, const Expression &limit,
                      const Expression &step, const Statements &state,
                      bool wait_on_next, Driver* driver);

  static Line IfThen(const Expression &bool_expr,
                     const Statements &then_state,
                     const Statements &elseifs);

  static Line IfThenElse(const Expression &bool_expr,
                         const Statements &then_state,
                         const Statements &else_state,
                         const Statements &elseifs);

  static Line Print(const std::string &port1, const ExpressionList &args,
                    Driver* driver);

  static Line SetChannels(const std::string &port1,
     const Expression &channels_expr, Driver* driver);

  static Line Reset();

  static Line Wait(const Expression &expr);

  static Line While(const Expression &condition, const Statements &state,
                    Driver* driver);
  
  friend std::ostream& operator<<(std::ostream& os, Line line);
};

struct Statements {

  static Statements FirstStatement(Line stat) {
    Statements stats;
    stats.lines.push_back(stat);
    return stats;
  }
  Statements add(Line new_line) {
    lines.push_back(new_line);
    return *this;
  }
  int size() const {
    return lines.size();
  }
  friend std::ostream& operator<<(std::ostream& os, Statements statements) {
    os << "Statements(" << std::to_string(statements.size()) << " statements )";
    return os;
  }

  std::vector<Line> lines;
};

// A Block is a logical grouping of statements that will be turned into
// a CodeBlock later.
struct Block {
  enum Type {
    MAIN,  // Block of code that runs every sample that the module is "running".
    WHEN   // Waits for a condition to become true, and then runs.
  };
  enum Condition {
    START,  // Run when the program is compiled and starts running.
    EXPRESSION
  };
  static Block MainBlock(Statements stat) {
    Block block;
    block.type = Block::MAIN;
    block.lines = stat.lines;
    return block;
  }
  static Block WhenExpBlock(Expression &condition, Statements stat) {
    Block block;
    block.type = Block::WHEN;
    block.condition = Block::EXPRESSION;
    block.lines = stat.lines;
    block.run_condition = condition;
    return block;
  }
  friend std::ostream& operator<<(std::ostream& os, Block block) {
    os << "Block(" << std::to_string(block.lines.size()) << " statements)";
    return os;
  }

  Type type;
  Condition condition;
  std::vector<Line> lines;  // Code.
  Expression run_condition;
};

struct Blocks {
  std::vector<Block> block_list;
  Blocks() { }
  explicit Blocks(Block main_block) {
    block_list.push_back(main_block);
  }
  Blocks Add(Block new_block) {
    block_list.push_back(new_block);
    return *this;
  }
  friend std::ostream& operator<<(std::ostream& os, Blocks blocks) {
    os << "Blocks(" << std::to_string(blocks.block_list.size()) << " blocks)";
    return os;
  }
};

#endif // TREE_HH
