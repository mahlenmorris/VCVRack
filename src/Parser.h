#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <vector>

#include "Tokenizer.h"

/*
 * Break up the input program into runnable chunks of operations for the
 * Runner to execute.
 * One quirk is that the Runner will give up control whenever a
 * WAIT is hit; and also at the end of the program, before it restarts.
 */

/*
 * The parsed program is a series of Lines in a pseudocode the
 * Runner can execute. The Program Counter (PC) is an index into
 * the array of lines.
 */

enum class LINETYPE {
  WAIT,
  ASSIGNMENT
};

class Expression;
class Expression {
  // Only one of constant or operation should be filled.
  Token constant;
  Token operation;
  // [0] = lhs, [1] = rhs. Are only filled when operation is set.
  std::vector<Expression> lhs_rhs;
};

struct Line {
  LINETYPE line_type;
};

struct Wait : Line {
  Expression ticks_to_wait;
};

struct Assignment : Line {
  std::string lhs_name;
  Expression value_expression;
};

struct Program {
   std::vector<Line> lines;
   std::string error;  // If there's an error, this explains it.
};

struct Parser {
  Tokenizer token_source = Tokenizer("");

  Parser(std::string src) {
    token_source = Tokenizer(src);
  }

  Expression add_expression() {
    Expression lhs = multiply_expression();
    
    return lhs;
  }

  Program parse_program() {
    Program program;

    while (true) {
      Token token = token_source.getToken();
      if (token.type == TOKEN::NO_TOKEN_FOUND) {
        break;
      }
      if (token.type == TOKEN::KEYWORD && token.value == "wait") {
        // "wait {expression}"
        Wait line;
        line.line_type = LINETYPE::WAIT;
        line.ticks_to_wait = add_expression();
        program.lines.push_back(line);
      }
    }

    return program;
  }

};

#endif  // PARSER_H_
