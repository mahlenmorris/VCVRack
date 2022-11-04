#include "gtest/gtest.h"

#include "../src/parser/driver.hh"
#include "../src/parser/environment.h"
#include <stdexcept>

TEST(ParserTest, RunsAtAll)
{
    Driver drv;
    Environment env;
    EXPECT_EQ(0, drv.parse("f = 3 + 4*6"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("f", drv.lines[0].str1);
    EXPECT_EQ(27, drv.lines[0].expr1.Compute(&env));
}

TEST(ParserTest, SimpleTest)
{
    Driver drv;
    Environment env;

    EXPECT_EQ(0, drv.parse("out1 = (3 + 3) * 6"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(36, drv.lines[0].expr1.Compute(&env));

    // Space at end.
    EXPECT_EQ(0, drv.parse("out1 = (3 + 3) * 6  \n"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(36, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("out1 = 3 + in2\n"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    env.variables["in2"] = 0.338;
    EXPECT_FLOAT_EQ(3.338, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("out1 = 3 + in2\nwait 1000"));
    ASSERT_EQ(2, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_FLOAT_EQ(3.338, drv.lines[0].expr1.Compute(&env));
    EXPECT_FLOAT_EQ(1000, drv.lines[1].expr1.Compute(&env));

    // TODO: negative and float constants.
    EXPECT_EQ(0, drv.parse("out1 = -0.5 + in2"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    env.variables["in2"] = 1.0f;
    EXPECT_FLOAT_EQ(0.5, drv.lines[0].expr1.Compute(&env));
}

TEST(ParserTest, NegativeTest)
{
    Driver drv;
    Environment env;

    EXPECT_EQ(0, drv.parse("out1 = 5 -3"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(2, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("out1 = 5 - 3"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(2, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("out1 = 1 - -0.33"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    env.variables["in2"] = 1.0f;
    EXPECT_FLOAT_EQ(1.33, drv.lines[0].expr1.Compute(&env));
}

TEST(ParserTest, BooleanTest)
{
    Driver drv;
    Environment env;

    EXPECT_EQ(0, drv.parse("if 5 > 3 then wait 0 endif"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].bool1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 5 < 3 then wait 0 endif"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].bool1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 > 5 then wait 0 endif"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].bool1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 < 5 then wait 0 endif"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].bool1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    env.variables["in1"] = 5.0f;

    EXPECT_EQ(0, drv.parse("if in1 > 3 then wait 0 endif"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].bool1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if in1 < 3 then wait 0 endif"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].bool1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 > in1 then wait 0 endif"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].bool1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 < in1 then wait 0 endif"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].bool1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());


}

TEST(ParserTest, IfThenTest)
{
    Driver drv;
    Environment env;

    EXPECT_EQ(0, drv.parse("if 5 > 3 then out1 = 5 - 3 endif"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].bool1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());
}
