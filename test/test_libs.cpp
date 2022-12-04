#include "gtest/gtest.h"

#include "../src/parser/driver.hh"
#include <stdexcept>

TEST(ParserTest, RunsAtAll)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("f = 3 + 4*6"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("f", drv.lines[0].str1);
    EXPECT_EQ(27, drv.lines[0].expr1.Compute());
}

TEST(ParserTest, Comments)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("' I'm a comment! Look at me \nf=3+4*6 ' Me Too!'"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("f", drv.lines[0].str1);
    EXPECT_EQ(27, drv.lines[0].expr1.Compute());
}

TEST(ParserTest, SimpleTest)
{
    Driver drv;

    std::unordered_set<std::string> volatile_deps;

    EXPECT_EQ(0, drv.parse("out1 = (3 + 3) * 6"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(36, drv.lines[0].expr1.Compute());
    EXPECT_FALSE(drv.lines[0].expr1.Volatile(&volatile_deps));
    EXPECT_TRUE(volatile_deps.empty());

    // Space at end.
    EXPECT_EQ(0, drv.parse("out1 = (3 + 3) * 6  \n"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(36, drv.lines[0].expr1.Compute());
    EXPECT_FALSE(drv.lines[0].expr1.Volatile(&volatile_deps));
    EXPECT_TRUE(volatile_deps.empty());

    EXPECT_EQ(0, drv.parse("out1 = 3 + in2\n"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    *(drv.GetVarFromName("in2")) = 0.338;
    EXPECT_FLOAT_EQ(3.338, drv.lines[0].expr1.Compute());
    EXPECT_TRUE(drv.lines[0].expr1.Volatile(&volatile_deps));
    EXPECT_EQ(1, volatile_deps.size());
    volatile_deps.clear();

    EXPECT_EQ(0, drv.parse("out1 = 3 + in2\nwait 1000"));
    ASSERT_EQ(2, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_FLOAT_EQ(3.338, drv.lines[0].expr1.Compute());
    EXPECT_TRUE(drv.lines[0].expr1.Volatile(&volatile_deps));
    EXPECT_EQ(1, volatile_deps.size());
    volatile_deps.clear();
    EXPECT_FLOAT_EQ(1000, drv.lines[1].expr1.Compute());
    EXPECT_FALSE(drv.lines[1].expr1.Volatile(&volatile_deps));
    EXPECT_TRUE(volatile_deps.empty());

    // TODO: negative and float constants.
    EXPECT_EQ(0, drv.parse("out1 = -0.5 + in2 + pow(in2, in4)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    *(drv.GetVarFromName("in2")) = 1.0f;
    EXPECT_FLOAT_EQ(1.5, drv.lines[0].expr1.Compute());
    EXPECT_TRUE(drv.lines[0].expr1.Volatile(&volatile_deps));
    EXPECT_EQ(2, volatile_deps.size());
    EXPECT_TRUE(volatile_deps.find("in2") != volatile_deps.end());
    EXPECT_TRUE(volatile_deps.find("in4") != volatile_deps.end());
}

TEST(ParserTest, FunctionTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("f = abs(2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(2.3f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = abs(-1.0 * 2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(2.3f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = ceiling(2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(3.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = ceiling(-2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(-2.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = floor(2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(2.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = floor(-2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(-3.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = sign(2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = sign(-2.3)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(-1.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = sign(0)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0, drv.lines[0].expr1.Compute());

    // sin of 30 degress = 0.5
    EXPECT_EQ(0, drv.parse("f = sin(30*3.14159265/180)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0.5f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = max(-2.3, 4)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(4.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = min(1.0, max(-0.5, 0.0))"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = mod(10, 4)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(2.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = pow(49, 0.5)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(7.0f, drv.lines[0].expr1.Compute());
}

TEST(ParserTest, NegativeTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("out1 = 5 -3"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(2, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = 5 - 3"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(2, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = 1 - -0.33"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    *(drv.GetVarFromName("in2")) = 1.0f;
    EXPECT_FLOAT_EQ(1.33, drv.lines[0].expr1.Compute());
}

TEST(ParserTest, BooleanTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("if 5 > 3 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].expr1.Compute());
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 5 < 3 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].expr1.Compute());
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 > 5 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].expr1.Compute());
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 < 5 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].expr1.Compute());
    ASSERT_EQ(1, drv.lines[0].statements.size());

    *(drv.GetVarFromName("in1")) = 5.0f;

    EXPECT_EQ(0, drv.parse("if in1 > 3 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].expr1.Compute());
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if in1 < 3 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].expr1.Compute());
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 > in1 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(false, drv.lines[0].expr1.Compute());
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("if 3 < in1 then wait 0 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].expr1.Compute());
    ASSERT_EQ(1, drv.lines[0].statements.size());

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 and 2"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 and 0.0"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 and not 0.0"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 or 0.0"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute());

    // "and" should bind more tightly than "or"
    EXPECT_EQ(0, drv.parse("ok = 1 == 3 and 3 > 2 or 2 == 2"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute());
    EXPECT_EQ(0, drv.parse("ok = 1 == 3 and (3 > 2 or 2 == 2)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0, drv.lines[0].expr1.Compute());

    // "not" should bind to "3", not "3 and 3 > 2 or 2 == 2"
    EXPECT_EQ(0, drv.parse("ok = not 3 and 3 > 2 or 2 == 2"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute());
    EXPECT_EQ(0, drv.parse("ok = not (3 and 3 > 2 or 2 == 2)"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("ok = 1 == 1 and 2 <= 2 and 3 >= 3"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(1, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("ok = 1 != 1 or 2 < 2 or 3 > 3"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(0, drv.lines[0].expr1.Compute());
}

TEST(ParserTest, IfThenTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("if 5 > 3 then out1 = 5 - 3 end if"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ(true, drv.lines[0].expr1.Compute());
    ASSERT_EQ(1, drv.lines[0].statements.size());
}

TEST(ParserTest, ForTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("for a = 1 to 10 out1 = a / 10 next"));
    ASSERT_EQ(1, drv.lines.size());
    Line line = drv.lines[0];
    EXPECT_EQ(Line::FORNEXT, line.type);
    EXPECT_EQ("a", line.str1);
    EXPECT_EQ(1, line.expr1.Compute());
    EXPECT_EQ(10, line.expr2.Compute());
    EXPECT_EQ(1, line.expr3.Compute());
    ASSERT_EQ(1, line.statements.size());
    ASSERT_EQ(1, line.statements[0].size());

    EXPECT_EQ(0, drv.parse("for a = 10 to 1 step -.4 out1 = a / 10 next"));
    ASSERT_EQ(1, drv.lines.size());
    line = drv.lines[0];
    EXPECT_EQ(Line::FORNEXT, line.type);
    EXPECT_EQ("a", line.str1);
    EXPECT_EQ(10, line.expr1.Compute());
    EXPECT_EQ(1, line.expr2.Compute());
    EXPECT_FLOAT_EQ(-0.4, line.expr3.Compute());
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

TEST(ParserTest, ErrorTest)
{
  Driver drv;

  EXPECT_EQ(0, drv.parse("out1 = 5 - 3"));
  ASSERT_EQ(1, drv.lines.size());
  ASSERT_EQ(0, drv.errors.size());

  EXPECT_EQ(1, drv.parse("out1 = 5 - "));
  ASSERT_EQ(0, drv.lines.size());
  ASSERT_EQ(1, drv.errors.size());
  Error err = drv.errors[0];
  EXPECT_EQ(1, err.line);
  EXPECT_EQ(12, err.column);
  EXPECT_EQ("syntax error, unexpected end of file", err.message);

  // Effect of a comment.
  EXPECT_EQ(1, drv.parse("out1 = 5\n' comment\nidjfi"));
  ASSERT_EQ(0, drv.lines.size());
  ASSERT_EQ(1, drv.errors.size());
  err = drv.errors[0];
  EXPECT_EQ(3, err.line);
  EXPECT_EQ(6, err.column);
  EXPECT_EQ("syntax error, unexpected end of file, expecting =", err.message);

  EXPECT_EQ(1, drv.parse("out1 = 5 ' comment\nidjfi"));
  ASSERT_EQ(0, drv.lines.size());
  ASSERT_EQ(1, drv.errors.size());
  err = drv.errors[0];
  EXPECT_EQ(2, err.line);
  EXPECT_EQ(6, err.column);
  EXPECT_EQ("syntax error, unexpected end of file, expecting =", err.message);
}

TEST(ParserTest, NoteTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("out1 = c3"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_EQ(-1.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = c#0"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_FLOAT_EQ(-4.0f + 0.0833333, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = c-1"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_FLOAT_EQ(-5.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = c#-1"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_FLOAT_EQ(-5.0f + 0.0833333, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = c10"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_FLOAT_EQ(6.0f, drv.lines[0].expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = db10"));
    ASSERT_EQ(1, drv.lines.size());
    EXPECT_EQ("out1", drv.lines[0].str1);
    EXPECT_FLOAT_EQ(6.0f + 0.0833333, drv.lines[0].expr1.Compute());
}
