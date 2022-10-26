#include "gtest/gtest.h"

#include "../src/parser/driver.hh"
#include "../src/parser/environment.h"
#include <stdexcept>

TEST(TokenizerTest, RunsAtAll)
{
    driver drv;
    Environment env;

    EXPECT_EQ(0, drv.parse("f := 3 + 4*6"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("f", drv.lines[0].str1);
    EXPECT_EQ(27, drv.lines[0].expr1->Compute(env));


/*

    EXPECT_EQ("IDENT: out1", tokenizer.getToken().toString());
    EXPECT_EQ("ASSIGN", tokenizer.getToken().toString());
    EXPECT_EQ("Number: 3.3", tokenizer.getToken().toString());
    auto token = tokenizer.getToken();
    EXPECT_EQ(TOKEN::NO_TOKEN_FOUND, token.type);

    // Space at end.
    tokenizer = Tokenizer("out1 = 3.3  \n");
    EXPECT_EQ("IDENT: out1", tokenizer.getToken().toString());
    EXPECT_EQ("ASSIGN", tokenizer.getToken().toString());
    EXPECT_EQ("Number: 3.3", tokenizer.getToken().toString());
    token = tokenizer.getToken();
    EXPECT_EQ(TOKEN::NO_TOKEN_FOUND, token.type);

    tokenizer = Tokenizer("out1 = 0.5 * IN2");
    EXPECT_EQ("IDENT: out1", tokenizer.getToken().toString());
    EXPECT_EQ("ASSIGN", tokenizer.getToken().toString());
    EXPECT_EQ("Number: 0.5", tokenizer.getToken().toString());
    EXPECT_EQ("OPERATOR: *", tokenizer.getToken().toString());
    EXPECT_EQ("IDENT: in2", tokenizer.getToken().toString());

    tokenizer = Tokenizer("out1 = -0.5 * IN2");
    EXPECT_EQ("IDENT: out1", tokenizer.getToken().toString());
    EXPECT_EQ("ASSIGN", tokenizer.getToken().toString());
    EXPECT_EQ("Number: -0.5", tokenizer.getToken().toString());
    EXPECT_EQ("OPERATOR: *", tokenizer.getToken().toString());
    EXPECT_EQ("IDENT: in2", tokenizer.getToken().toString());

    tokenizer = Tokenizer("WAIT 1000");
    EXPECT_EQ("KEYWORD: wait", tokenizer.getToken().toString());
    EXPECT_EQ("Number: 1000", tokenizer.getToken().toString());

    tokenizer = Tokenizer("out1 = IN2 - -0.33");
    EXPECT_EQ("IDENT: out1", tokenizer.getToken().toString());
    EXPECT_EQ("ASSIGN", tokenizer.getToken().toString());
    EXPECT_EQ("IDENT: in2", tokenizer.getToken().toString());
    EXPECT_EQ("OPERATOR: -", tokenizer.getToken().toString());
    EXPECT_EQ("Number: -0.33", tokenizer.getToken().toString());
    */
}

/*
TEST(TokenizerTest, PushBack)
{
    auto tokenizer = Tokenizer("out1 = 3.3");
    Token token = tokenizer.getToken();
    tokenizer.pushBack(token);
    EXPECT_EQ("IDENT: out1", tokenizer.getToken().toString());
    EXPECT_EQ("ASSIGN", tokenizer.getToken().toString());
    EXPECT_EQ("Number: 3.3", tokenizer.getToken().toString());
    token = tokenizer.getToken();
    EXPECT_EQ(TOKEN::NO_TOKEN_FOUND, token.type);
}

TEST(ParserTest, Basic) {
  Parser parser = Parser("WAIT 1000");
  Program program = parser.parse_program();
}
*/
