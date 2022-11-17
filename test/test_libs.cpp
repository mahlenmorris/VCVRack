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

TEST(ParserTest, Comments)
{
    Driver drv;
    Environment env;
    EXPECT_EQ(0, drv.parse("' I'm a comment! Look at me \nf=3+4*6 ' Me Too!'"));
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

TEST(ParserTest, FunctionTest)
{
    Driver drv;
    Environment env;
    EXPECT_EQ(0, drv.parse("f = abs(2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(2.3f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = abs(-1.0 * 2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(2.3f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = ceiling(2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(3.0f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = ceiling(-2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(-2.0f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = floor(2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(2.0f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = floor(-2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(-3.0f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = sign(2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1.0f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = sign(-2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(-1.0f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = sign(0)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0, drv.lines[0].expr1.Compute(&env));

    // sin of 30 degress = 0.5
    EXPECT_EQ(0, drv.parse("f = sin(30*3.14159265/180)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0.5f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = max(-2.3, 4)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(4.0f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = min(1.0, max(-0.5, 0.0))"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0.0f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = mod(10, 4)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(2.0f, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("f = pow(49, 0.5)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(7.0f, drv.lines[0].expr1.Compute(&env));
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

    EXPECT_EQ(0, drv.parse("if 5 > 3 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].expr1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 5 < 3 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].expr1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 > 5 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].expr1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 < 5 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].expr1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    env.variables["in1"] = 5.0f;

    EXPECT_EQ(0, drv.parse("if in1 > 3 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].expr1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if in1 < 3 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].expr1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 > in1 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].expr1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 < in1 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].expr1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 and 2"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 and 0.0"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 and not 0.0"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 or 0.0"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute(&env));

    // "and" should bind more tightly than "or"
    EXPECT_EQ(0, drv.parse("ok = 1 == 3 and 3 > 2 or 2 == 2"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute(&env));
    EXPECT_EQ(0, drv.parse("ok = 1 == 3 and (3 > 2 or 2 == 2)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0, drv.lines[0].expr1.Compute(&env));

    // "not" should bind to "3", not "3 and 3 > 2 or 2 == 2"
    EXPECT_EQ(0, drv.parse("ok = not 3 and 3 > 2 or 2 == 2"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute(&env));
    EXPECT_EQ(0, drv.parse("ok = not (3 and 3 > 2 or 2 == 2)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("ok = 1 == 1 and 2 <= 2 and 3 >= 3"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute(&env));

    EXPECT_EQ(0, drv.parse("ok = 1 != 1 or 2 < 2 or 3 > 3"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0, drv.lines[0].expr1.Compute(&env));
}

TEST(ParserTest, IfThenTest)
{
    Driver drv;
    Environment env;

    EXPECT_EQ(0, drv.parse("if 5 > 3 then out1 = 5 - 3 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].expr1.Compute(&env));
    ASSERT_EQ(1, drv.lines[0].statements.size());
}

TEST(ParserTest, ForTest)
{
    Driver drv;
    Environment env;

    EXPECT_EQ(0, drv.parse("for a = 1 to 10 out1 = a / 10 next"));
    ASSERT_EQ(1, drv.lines.size());
    Line line = drv.lines[0];
    EXPECT_EQ(Line::FORNEXT, line.type);
    EXPECT_EQ("a", line.str1);
    EXPECT_EQ(1, line.expr1.Compute(&env));
    EXPECT_EQ(10, line.expr2.Compute(&env));
    EXPECT_EQ(1, line.expr3.Compute(&env));
    ASSERT_EQ(1, line.statements.size());
    ASSERT_EQ(1, line.statements[0].size());

    EXPECT_EQ(0, drv.parse("for a = 10 to 1 step -.4 out1 = a / 10 next"));
    ASSERT_EQ(1, drv.lines.size());
    line = drv.lines[0];
    EXPECT_EQ(Line::FORNEXT, line.type);
    EXPECT_EQ("a", line.str1);
    EXPECT_EQ(10, line.expr1.Compute(&env));
    EXPECT_EQ(1, line.expr2.Compute(&env));
    EXPECT_FLOAT_EQ(-0.4, line.expr3.Compute(&env));
    ASSERT_EQ(1, line.statements.size());
    ASSERT_EQ(1, line.statements[0].size());

    EXPECT_EQ(0, drv.parse("continue for"));
    ASSERT_EQ(1, drv.lines.size());
    line = drv.lines[0];
    EXPECT_EQ(Line::CONTINUE, line.type);
    EXPECT_EQ("for", line.str1);

    EXPECT_EQ(0, drv.parse("exit for"));
    ASSERT_EQ(1, drv.lines.size());
    line = drv.lines[0];
    EXPECT_EQ(Line::EXIT, line.type);
    EXPECT_EQ("for", line.str1);
}

TEST(ParserTest, AllTest)
{
    Driver drv;
    Environment env;

    EXPECT_EQ(0, drv.parse("continue all"));
    ASSERT_EQ(1, drv.lines.size());
    Line line = drv.lines[0];
    EXPECT_EQ(Line::CONTINUE, line.type);
    EXPECT_EQ("all", line.str1);

    EXPECT_EQ(0, drv.parse("exit all"));
    ASSERT_EQ(1, drv.lines.size());
    line = drv.lines[0];
    EXPECT_EQ(Line::EXIT, line.type);
    EXPECT_EQ("all", line.str1);
}
