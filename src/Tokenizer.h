#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "ctype.h"

enum class TOKEN
{
    KEYWORD,
    IDENT,
    ASSIGN,
    OPERATOR,
    STRING,
    NUMBER,
    ARRAY_OPEN,
    ARRAY_CLOSE,
    COMMA
};

std::string KEYWORDS[] = {"for", "next", "wait"};  // TODO: add the real ones.

struct Token
{
    std::string value;
    TOKEN type;
    std::string toString() {
        switch (type) {
        case TOKEN::KEYWORD:
        {
            return "KEYWORD: " + value;
        }
        case TOKEN::IDENT:
        {
            return "IDENT: " + value;
        }
        case TOKEN::ASSIGN:
        {
            return "ASSIGN";
        }
        case TOKEN::OPERATOR:
        {
            return "OPERATOR: " + value;
        }
        case TOKEN::NUMBER:
        {
            return "Number: " + value;
        }
        case TOKEN::STRING:
        {
            return "String: " + value;
        }
        case TOKEN::ARRAY_OPEN:
        {
            return "Array open";
        }
        case TOKEN::ARRAY_CLOSE:
        {
            return "Array close";
        }
        case TOKEN::COMMA:
        {
            return "Comma";
        }
        default:
          return "!No Token!";
      }
    }
};

class Tokenizer
{
    std::string file;
    size_t next_char;
    size_t prevPos;

public:
    Tokenizer(std::string source) {
        file = source;
        next_char = 0;
    }

    bool good() {
        return next_char < file.size();
    }

    char get() {
      char c = file.at(next_char); // check
      next_char++;
      return c;
    }

    auto getWithoutWhiteSpace()
    {
        char c = ' ';
        while ((c == ' ' || c == '\n')) {
            c = get();

            if ((c == ' ' || c == '\n') && !good()) {
                // std::cout << file.eof() << " " << file.fail() << std::endl;
                throw std::logic_error("Ran out of tokens");
            } else if (!good()) {
                return c;
            }
        }
        return c;
    }

    auto getToken()
    {
        char c;
        if (!good()) {
            std::cout << "Exhausted tokens" << std::endl;
            // throw std::exception("Exhausted tokens");
        }
        prevPos = next_char;
        c = getWithoutWhiteSpace();

        struct Token token;
        if (c == '"') {
            token.type = TOKEN::STRING;
            token.value = "";
            c = get();
            while (c != '"') {
                token.value += c;
                c = get();
            }
        } else if (c=='-' || (c >= '0' && c <= '9')) {
              //Check if string is numeric
              token.type = TOKEN::NUMBER;
              token.value = "";
              token.value += c;
              size_t prevCharPos = next_char;
              while ((c=='-')||(c >= '0' && c <= '9') || c == '.') {
                  prevCharPos = next_char;
                  c = get();

                  if ((c=='-')||(c >= '0' && c <= '9')||(c=='.')) {
                      token.value += c;
                  } else {
                      next_char = prevCharPos;
                      // std::cout << c << std::endl;
                  }
                  if (!good()) {  // No more characters to read.
                      break;
                  }
              }
        } else if (isalpha(c)) {
            // Could be KEYWORD or IDENT; decide when we have the whole thing.
            token.value = "";
            token.value += tolower(c);
            size_t prevCharPos;
            while (isalpha(c)) {
                prevCharPos = next_char;
                c = get();

                if (isalpha(c) || isdigit(c)) {
                    token.value += tolower(c);
                } else {
                    next_char = prevCharPos;
                }
                if (!good()) {  // No more characters to read.
                    break;
                }
            }
            std::string *found = std::find(std::begin(KEYWORDS),
                                           std::end(KEYWORDS), token.value);
            if (found == std::end(KEYWORDS)) {
              token.type = TOKEN::IDENT;
            } else {
              token.type = TOKEN::KEYWORD;
            }
        } else if ((c == '+') || (c == '-') || (c == '*') || (c == '/')) {
            token.type = TOKEN::OPERATOR;
            token.value = "";
            token.value += c;
        } else if (c == '[') {
            token.type = TOKEN::ARRAY_OPEN;
        } else if (c == ']') {
            token.type = TOKEN::ARRAY_CLOSE;
        } else if (c == '=') {  // TODO: differ from == test.
            token.type = TOKEN::ASSIGN;
        } else if (c == ',') {
            token.type = TOKEN::COMMA;
        }
        return token;
    }

    auto hasMoreTokens()
    {
        return !good();
    }

    void rollBackToken()
    {
        next_char = prevPos;
    }

    /*
        "": String begin?
        {}: Object
        []: List
        num: number

     */
};
