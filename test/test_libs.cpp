#include "gtest/gtest.h"

#include "../src/parser/driver.hh"
#include "../src/parser/environment.h"
#include <stdexcept>

TEST(ParserTest, RunsAtAll)
{
  std::cout << "0.";
    driver drv;
    Environment env;
    EXPECT_EQ(0, drv.parse("f := 3 + 4*6"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("f", drv.lines[0].str1);
    EXPECT_EQ(27, drv.lines[0].expr1.Compute(env));
}

TEST(ParserTest, SimpleTest)
{
    driver drv;
    Environment env;

    EXPECT_EQ(0, drv.parse("out1 := (3 + 3) * 6"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(36, drv.lines[0].expr1.Compute(env));

    // Space at end.
    EXPECT_EQ(0, drv.parse("out1 := (3 + 3) * 6  \n"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(36, drv.lines[0].expr1.Compute(env));

    EXPECT_EQ(0, drv.parse("out1 := 3 + in2\n"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    env.variables["in2"] = 0.338;
    EXPECT_FLOAT_EQ(3.338, drv.lines[0].expr1.Compute(env));

    EXPECT_EQ(0, drv.parse("out1 := 3 + in2\nwait 1000"));
    ASSERT_EQ(2, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_FLOAT_EQ(3.338, drv.lines[0].expr1.Compute(env));
    EXPECT_FLOAT_EQ(1000, drv.lines[1].expr1.Compute(env));

}

/*
// TODO: negative and float constants.
    tokenizer = Tokenizer("out1 = -0.5 * IN2");
    EXPECT_EQ("IDENT: out1", tokenizer.getToken().toString());
    EXPECT_EQ("ASSIGN", tokenizer.getToken().toString());
    EXPECT_EQ("Number: -0.5", tokenizer.getToken().toString());
    EXPECT_EQ("OPERATOR: *", tokenizer.getToken().toString());
    EXPECT_EQ("IDENT: in2", tokenizer.getToken().toString());


    tokenizer = Tokenizer("out1 = IN2 - -0.33");
    EXPECT_EQ("IDENT: out1", tokenizer.getToken().toString());
    EXPECT_EQ("ASSIGN", tokenizer.getToken().toString());
    EXPECT_EQ("IDENT: in2", tokenizer.getToken().toString());
    EXPECT_EQ("OPERATOR: -", tokenizer.getToken().toString());
    EXPECT_EQ("Number: -0.33", tokenizer.getToken().toString());
    */
