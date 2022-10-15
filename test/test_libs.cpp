#include "gtest/gtest.h"

#include "../src/Tokenizer.h"
#include <stdexcept>

TEST(TokenizerTest, RunsAtAll)
{
    auto tokenizer = Tokenizer("out1 = 3.3");
    EXPECT_EQ("IDENT: out1", tokenizer.getToken().toString());
    EXPECT_EQ("ASSIGN", tokenizer.getToken().toString());
    EXPECT_EQ("Number: 3.3", tokenizer.getToken().toString());
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
}
