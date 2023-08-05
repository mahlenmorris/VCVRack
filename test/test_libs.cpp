#include "gtest/gtest.h"

#include "../src/code_block.h"
#include "../src/parser/driver.hh"
#include "../src/parser/environment.h"
#include "../src/pcode_trans.h"
#include <stdexcept>

struct TestEnvironment : Environment {
  std::unordered_map<int, bool> connected;
  std::unordered_map<int, float> outputs;
  std::unordered_map<int, float> inputs;
  bool clear_called = false;
  bool starting = false;
  // List of strings sent via Send().
  std::vector<std::string> text_sent;

  void SetIns(float a, float b, float c, float d) {
    inputs[0] = a;
    inputs[1] = b;
    inputs[2] = c;
    inputs[3] = d;
  }
  void setConnected(int index, bool is_connected) {
    connected[index] = is_connected;
  }
  float SampleRate() override {
    return 43210;  // An unusual sample rate!
  }
  float GetVoltage(const PortPointer &port) override {
    if (port.port_type == PortPointer::INPUT) {
      return inputs[port.index];
    } else {
      return outputs[port.index];
    }
  }
  void SetVoltage(const PortPointer &port, float value) override {
    if (port.port_type == PortPointer::INPUT) {
      inputs[port.index] = value;
    } else {
      outputs[port.index] = value;
    }
  }
  float Connected(const PortPointer &port) override {
    return (connected.find(port.index) != connected.end() &&
            connected.at(port.index)) ? 1.0f : 0.0f;
  }
  float Random(float min_value, float max_value) override {
    return (max_value + min_value) / 2.0;  // Not very random.
  }
  float Normal(float mean, float std_dev) override {
    return mean + std_dev;  // Also not random.
  }
  float Time(bool millis) override {
    return millis ? 1000.0 : 1.0;
  }
  void Clear() override {
    clear_called = true;
  }
  void Reset() override { }
  bool Start() override { return starting; }

  bool Trigger(const PortPointer &port) override {
    return (port.port_type == PortPointer::INPUT);
  }

  void Send(const PortPointer &port, const std::string &str) override {
    (void) port;  // To skip unused parameter warning during compilation.
    text_sent.push_back(str);
  }
};

TEST(ParserTest, ParsesAtAll)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("f = 3 + 4*6"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("f", lines->at(0).str1);
    EXPECT_EQ(27, lines->at(0).expr1.Compute());
}

TEST(ParserTest, Comments)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("' I'm a comment! Look at me \nf=3+4*6 ' Me Too!'"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("f", lines->at(0).str1);
    EXPECT_EQ(27, lines->at(0).expr1.Compute());
}

TEST(ParserTest, Arrays)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("a[4] = 6"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("a", lines->at(0).str1);
    EXPECT_EQ(4, lines->at(0).expr1.Compute());
    EXPECT_EQ(6, lines->at(0).expr2.Compute());

    EXPECT_EQ(0, drv.parse("b = a[4]"));
    ASSERT_EQ(1, lines->size());

    EXPECT_EQ(0, drv.parse("b = a[-3]"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("a[4] = {1, 0, 1, sin(in1)}"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("a", lines->at(0).str1);
    EXPECT_EQ(4, lines->at(0).expr1.Compute());
    EXPECT_EQ(4, lines->at(0).expr_list.size());
}

TEST(ParserTest, SimpleTest)
{
    Driver drv;
    drv.AddPortForName("in1", true, 0);
    drv.AddPortForName("in2", true, 1);
    drv.AddPortForName("in3", true, 2);
    drv.AddPortForName("in4", true, 3);
    TestEnvironment test_env;
    drv.SetEnvironment(&test_env);

    EXPECT_EQ(0, drv.parse("out1 = (3 + 3) * 6"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_EQ(36, lines->at(0).expr1.Compute());
    EXPECT_FALSE(lines->at(0).expr1.Volatile());

    // Space at end.
    EXPECT_EQ(0, drv.parse("out1 = (3 + 3) * 6  \n"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_EQ(36, lines->at(0).expr1.Compute());
    EXPECT_FALSE(lines->at(0).expr1.Volatile());

    test_env.SetIns(0.0, 0.338, 0, 0);

    EXPECT_EQ(0, drv.parse("out1 = 3 + in2\n"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_FLOAT_EQ(3.338, lines->at(0).expr1.Compute());
    EXPECT_TRUE(lines->at(0).expr1.Volatile());

    EXPECT_EQ(0, drv.parse("out1 = 3 + in2\nwait 1000"));
    ASSERT_EQ(2, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_FLOAT_EQ(3.338, lines->at(0).expr1.Compute());
    EXPECT_TRUE(lines->at(0).expr1.Volatile());
    EXPECT_FLOAT_EQ(1000, lines->at(1).expr1.Compute());
    EXPECT_FALSE(lines->at(1).expr1.Volatile());

    test_env.SetIns(0.0, 1.0f, 0, 0);
    // TODO: negative and float constants.
    EXPECT_EQ(0, drv.parse("out1 = -0.5 + in2 + pow(in2, in4)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_FLOAT_EQ(1.5, lines->at(0).expr1.Compute());
    EXPECT_TRUE(lines->at(0).expr1.Volatile());
}

TEST(ParserTest, FunctionTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("f = abs(2.3)"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(2.3f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = ABS(-1.0 * 2.3)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(2.3f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = ceiling(2.3)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(3.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = ceILIng(-2.3)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(-2.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = floor(2.3)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(2.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = Floor(-2.3)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(-3.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = sign(2.3)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(1.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = sign(-2.3)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(-1.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = SiGn(0)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0, lines->at(0).expr1.Compute());

    // sin of 30 degress = 0.5
    EXPECT_EQ(0, drv.parse("f = sin(30*3.14159265/180)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0.5f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = max(-2.3, 4)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(4.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = MIN(1.0, max(-0.5, 0.0))"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = mod(10, 4)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(2.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = pow(49, 0.5)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(7.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = log2(8)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(3.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = log2(0)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = LOG2(-3)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = loge(8)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(2.07944154f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = loge(0)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = loge(-3)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = log10(100)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(2.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = log10(0)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("f = log10(-3)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0.0f, lines->at(0).expr1.Compute());
}

TEST(ParserTest, NegativeTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("out1 = 5 -3"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_EQ(2, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = 5 - 3"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_EQ(2, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = 1 - -0.33"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    *(drv.GetVarFromName("in2")) = 1.0f;
    EXPECT_FLOAT_EQ(1.33, lines->at(0).expr1.Compute());
}

TEST(ParserTest, BooleanTest)
{
    Driver drv;
    drv.AddPortForName("in1", true, 0);
    drv.AddPortForName("in2", true, 1);
    drv.AddPortForName("in3", true, 2);
    drv.AddPortForName("in4", true, 3);
    TestEnvironment test_env;
    drv.SetEnvironment(&test_env);

    EXPECT_EQ(0, drv.parse("if 5 > 3 then wait 0 end if"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(true, lines->at(0).expr1.Compute());
    ASSERT_EQ(2, lines->at(0).statements.size());
    ASSERT_EQ(0, lines->at(0).statements[1].size());  // no elseifs

    EXPECT_EQ(0, drv.parse("if 5 < 3 then wait 0 end if"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(false, lines->at(0).expr1.Compute());
    ASSERT_EQ(2, lines->at(0).statements.size());
    ASSERT_EQ(0, lines->at(0).statements[1].size());  // no elseifs

    EXPECT_EQ(0, drv.parse(
      "if 3 > 5 then wait 0 elseif 6 == 9 then out1 = 9 end if"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(false, lines->at(0).expr1.Compute());
    ASSERT_EQ(2, lines->at(0).statements.size());
    ASSERT_EQ(1, lines->at(0).statements[1].size());  // one elseifs

    EXPECT_EQ(0, drv.parse(
      "if 3 < 5 then wait 0 "
      "elseif 2 == 0 then wait 1 "
      "elseif 2 == 1 then wait 2 out1 = 2"
      "elseif 2 == 2 then wait 3 out1 = 3 out2 = 5"
      "end if"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(true, lines->at(0).expr1.Compute());
    ASSERT_EQ(2, lines->at(0).statements.size());
    ASSERT_EQ(3, lines->at(0).statements[1].size());  // three elseifs

    test_env.SetIns(5.0, 0, 0, 0);

    EXPECT_EQ(0, drv.parse("if in1 > 3 then wait 0 end if"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(true, lines->at(0).expr1.Compute());
    ASSERT_EQ(2, lines->at(0).statements.size());

    EXPECT_EQ(0, drv.parse("if in1 < 3 then wait 0 end if"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(false, lines->at(0).expr1.Compute());
    ASSERT_EQ(2, lines->at(0).statements.size());

    EXPECT_EQ(0, drv.parse("if 3 > in1 then wait 0 end if"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(false, lines->at(0).expr1.Compute());
    ASSERT_EQ(2, lines->at(0).statements.size());

    EXPECT_EQ(0, drv.parse("if 3 < in1 then wait 0 end if"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(true, lines->at(0).expr1.Compute());
    ASSERT_EQ(2, lines->at(0).statements.size());

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 and 2"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(1, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 and 0.0"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 and not 0.0"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(1, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("ok = 3 < in1 or 0.0"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(1, lines->at(0).expr1.Compute());

    // "and" should bind more tightly than "or"
    EXPECT_EQ(0, drv.parse("ok = 1 == 3 and 3 > 2 or 2 == 2"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(1, lines->at(0).expr1.Compute());
    EXPECT_EQ(0, drv.parse("ok = 1 == 3 and (3 > 2 or 2 == 2)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0, lines->at(0).expr1.Compute());

    // "not" should bind to "3", not "3 and 3 > 2 or 2 == 2"
    EXPECT_EQ(0, drv.parse("ok = not 3 and 3 > 2 or 2 == 2"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(1, lines->at(0).expr1.Compute());
    EXPECT_EQ(0, drv.parse("ok = not (3 and 3 > 2 or 2 == 2)"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("ok = 1 == 1 and 2 <= 2 and 3 >= 3"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(1, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("ok = 1 != 1 or 2 < 2 or 3 > 3"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(0, lines->at(0).expr1.Compute());
}

TEST(ParserTest, IfThenTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("if 5 > 3 then out1 = 5 - 3 end if"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ(true, lines->at(0).expr1.Compute());
    ASSERT_EQ(2, lines->at(0).statements.size());
}

TEST(ParserTest, ForTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("for a = 1 to 10 out1 = a / 10 next"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    Line line = lines->at(0);
    EXPECT_EQ(Line::FORNEXT, line.type);
    EXPECT_EQ("a", line.str1);
    EXPECT_EQ(1, line.expr1.Compute());
    EXPECT_EQ(10, line.expr2.Compute());
    EXPECT_EQ(1, line.expr3.Compute());
    ASSERT_EQ(1, line.statements.size());
    ASSERT_EQ(1, line.statements[0].size());

    EXPECT_EQ(0, drv.parse("for a = 10 to 1 step -.4 out1 = a / 10 next"));
    ASSERT_EQ(1, lines->size());
    line = lines->at(0);
    EXPECT_EQ(Line::FORNEXT, line.type);
    EXPECT_EQ("a", line.str1);
    EXPECT_EQ(10, line.expr1.Compute());
    EXPECT_EQ(1, line.expr2.Compute());
    EXPECT_FLOAT_EQ(-0.4, line.expr3.Compute());
    ASSERT_EQ(1, line.statements.size());
    ASSERT_EQ(1, line.statements[0].size());

    EXPECT_EQ(0, drv.parse("continue for"));
    ASSERT_EQ(1, lines->size());
    line = lines->at(0);
    EXPECT_EQ(Line::CONTINUE, line.type);
    EXPECT_EQ("for", line.str1);

    EXPECT_EQ(0, drv.parse("exit for"));
    ASSERT_EQ(1, lines->size());
    line = lines->at(0);
    EXPECT_EQ(Line::EXIT, line.type);
    EXPECT_EQ("for", line.str1);
}

TEST(ParserTest, AllTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("continue all"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    Line line = lines->at(0);
    EXPECT_EQ(Line::CONTINUE, line.type);
    EXPECT_EQ("all", line.str1);

    EXPECT_EQ(0, drv.parse("exit all"));
    ASSERT_EQ(1, lines->size());
    line = lines->at(0);
    EXPECT_EQ(Line::EXIT, line.type);
    EXPECT_EQ("all", line.str1);
}

TEST(ParserTest, ErrorTest)
{
  Driver drv;

  EXPECT_EQ(0, drv.parse("out1 = 5 - 3"));
  std::vector<Line>* lines = &(drv.blocks[0].lines);
  ASSERT_EQ(1, lines->size());
  ASSERT_EQ(0, drv.errors.size());

  EXPECT_EQ(1, drv.parse("out1 = 5 - "));
  ASSERT_EQ(1, drv.errors.size());
  Error err = drv.errors[0];
  EXPECT_EQ(1, err.line);
  EXPECT_EQ(12, err.column);
  EXPECT_EQ("syntax error, unexpected end of file", err.message);

  // Effect of a comment.
  EXPECT_EQ(1, drv.parse("out1 = 5\n' comment\nidjfi"));
  ASSERT_EQ(1, drv.errors.size());
  err = drv.errors[0];
  EXPECT_EQ(3, err.line);
  EXPECT_EQ(6, err.column);
  EXPECT_EQ("syntax error, unexpected end of file, expecting = or [", err.message);

  EXPECT_EQ(1, drv.parse("out1 = 5 ' comment\nidjfi"));
  ASSERT_EQ(1, drv.errors.size());
  err = drv.errors[0];
  EXPECT_EQ(2, err.line);
  EXPECT_EQ(6, err.column);
  EXPECT_EQ("syntax error, unexpected end of file, expecting = or [", err.message);
}

TEST(ParserTest, NoteTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("out1 = c3"));
    std::vector<Line>* lines = &(drv.blocks[0].lines);
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_EQ(-1.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("OUT1 = c#0"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_FLOAT_EQ(-4.0f + 0.0833333, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("Out1 = c-1"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_FLOAT_EQ(-5.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = C#-1"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_FLOAT_EQ(-5.0f + 0.0833333, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = c10"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_FLOAT_EQ(6.0f, lines->at(0).expr1.Compute());

    EXPECT_EQ(0, drv.parse("out1 = Db10"));
    ASSERT_EQ(1, lines->size());
    EXPECT_EQ("out1", lines->at(0).str1);
    EXPECT_FLOAT_EQ(6.0f + 0.0833333, lines->at(0).expr1.Compute());
}

TEST(ParserTest, EnvironmentTest)
{
  Driver drv;
  drv.AddPortForName("in1", true, 0);
  drv.AddPortForName("in2", true, 1);
  drv.AddPortForName("in3", true, 2);
  drv.AddPortForName("in4", true, 3);
  TestEnvironment test_env;
  drv.SetEnvironment(&test_env);
  drv.AddPortForName("in1", true, 7);

  EXPECT_EQ(0, drv.parse("out1 = sample_rate()"));
  std::vector<Line>* lines = &(drv.blocks[0].lines);
  ASSERT_EQ(1, lines->size());
  EXPECT_EQ(43210, lines->at(0).expr1.Compute());

  EXPECT_EQ(0, drv.parse("out1 = normal(1, 0.2)"));
  ASSERT_EQ(1, lines->size());
  EXPECT_FLOAT_EQ(1.2, lines->at(0).expr1.Compute());

  EXPECT_EQ(0, drv.parse("out1 = random(0, 10)"));
  ASSERT_EQ(1, lines->size());
  EXPECT_EQ(5, lines->at(0).expr1.Compute());

  EXPECT_EQ(0, drv.parse("out1 = Time()"));
  ASSERT_EQ(1, lines->size());
  EXPECT_EQ(1, lines->at(0).expr1.Compute());

  EXPECT_EQ(0, drv.parse("out1 = time_millis()"));
  ASSERT_EQ(1, lines->size());
  EXPECT_EQ(1000, lines->at(0).expr1.Compute());

  EXPECT_EQ(0, drv.parse("out1 = 10 * CONNECTED(in1)"));
  ASSERT_EQ(1, lines->size());
  EXPECT_EQ(0, lines->at(0).expr1.Compute());
  test_env.setConnected(7, true);
  EXPECT_EQ(10, lines->at(0).expr1.Compute());

  EXPECT_EQ(0, drv.trigger_port_indexes.size());
  ASSERT_EQ(0, drv.parse("out1 = 10 * trigger(in1)"));
  ASSERT_EQ(1, lines->size());
  EXPECT_EQ(1, drv.trigger_port_indexes.size());
  EXPECT_EQ(10, lines->at(0).expr1.Compute());

  // Should not compile for an out port.
  ASSERT_EQ(1, drv.parse("out1 = 10 * trigger(out1)"));
}

TEST(ParserTest, BlocksTest)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("out1 = 2 also out2 = 4 end also"));
    ASSERT_EQ(2, drv.blocks.size());

    EXPECT_EQ(0, drv.parse("out1 = 2"));
    ASSERT_EQ(1, drv.blocks.size());

    // Two ALSO blocks is fine.
    EXPECT_EQ(0, drv.parse("also out1 = 2 end also  also out2 = 4 end also"));
    ASSERT_EQ(2, drv.blocks.size());  // two Main blocks are created.

    // main block must be first or not at all.
    EXPECT_EQ(1, drv.parse("also out1 = 2 end also  out2 = 4"));

    EXPECT_EQ(0, drv.parse("out1 = 2  when start() f = 0 end when"));
    ASSERT_EQ(2, drv.blocks.size());

    EXPECT_EQ(0, drv.parse("when foo == 1 foo = 0 end when"));
    ASSERT_EQ(1, drv.blocks.size());
}

TEST(ParserTest, ParseStringTest)
{
  Driver drv;

  EXPECT_EQ(0, drv.parse("print(out1, \"foo\")"));
  ASSERT_EQ(1, drv.blocks.size());

  EXPECT_EQ(0, drv.parse("print(out1, 7, sin(bar), \"foo\")"));
  ASSERT_EQ(1, drv.blocks.size());

  EXPECT_EQ(1, drv.parse("print(in1, \"foo\")"));
}

TEST(RunTest, RunsAtAll) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  CodeBlock block(&test_env);

  EXPECT_EQ(0, drv.parse("foo = 3.14159"));
  ASSERT_EQ(1, drv.blocks.size());
  ASSERT_TRUE(translator.BlockToCodeBlock(&block, drv.blocks[0]));
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  EXPECT_EQ(3.14159f, *(drv.GetVarFromName("foo")));
}

TEST(RunTest, RunsClear) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  drv.SetEnvironment(&test_env);
  CodeBlock block(&test_env);

  EXPECT_EQ(0, drv.parse("clear all"));
  ASSERT_EQ(1, drv.blocks.size());
  ASSERT_TRUE(translator.BlockToCodeBlock(&block, drv.blocks[0]));
  EXPECT_EQ(false, test_env.clear_called);
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  EXPECT_EQ(true, test_env.clear_called);
}

TEST(RunTest, RunsPrint) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  drv.SetEnvironment(&test_env);
  CodeBlock block(&test_env);

  EXPECT_EQ(0, drv.parse("print(out1, 7, \" \", sin(6), \" hello, world \")"));
  ASSERT_EQ(1, drv.blocks.size());
  ASSERT_TRUE(translator.BlockToCodeBlock(&block, drv.blocks[0]));
  ASSERT_EQ(0, test_env.text_sent.size());
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  ASSERT_EQ(1, test_env.text_sent.size());
  EXPECT_EQ("7 -0.279415 hello, world ", test_env.text_sent[0]);
}

TEST(RunTest, RunsPrintNewline) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  drv.SetEnvironment(&test_env);
  CodeBlock block(&test_env);

  EXPECT_EQ(0, drv.parse("print(out1, \"first line\nsecond line\")"));
  ASSERT_EQ(1, drv.blocks.size());
  ASSERT_TRUE(translator.BlockToCodeBlock(&block, drv.blocks[0]));
  ASSERT_EQ(0, test_env.text_sent.size());
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  ASSERT_EQ(1, test_env.text_sent.size());
  EXPECT_EQ("first line\nsecond line", test_env.text_sent[0]);
}

TEST(RunTest, RunsPrintCase) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  drv.SetEnvironment(&test_env);
  CodeBlock block(&test_env);

  EXPECT_EQ(0, drv.parse("print(out1, \"UPPER lower\")"));
  ASSERT_EQ(1, drv.blocks.size());
  ASSERT_TRUE(translator.BlockToCodeBlock(&block, drv.blocks[0]));
  ASSERT_EQ(0, test_env.text_sent.size());
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  ASSERT_EQ(1, test_env.text_sent.size());
  EXPECT_EQ("UPPER lower", test_env.text_sent[0]);
}

TEST(RunTest, RunsStringFunctions1) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  drv.SetEnvironment(&test_env);
  CodeBlock block(&test_env);

  EXPECT_EQ(0, drv.parse("foo = 3.7\n  print(out1, debug(foo))"));
  ASSERT_EQ(1, drv.blocks.size());
  ASSERT_TRUE(translator.BlockToCodeBlock(&block, drv.blocks[0]));
  ASSERT_EQ(0, test_env.text_sent.size());
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  ASSERT_EQ(1, test_env.text_sent.size());
  EXPECT_EQ("foo = 3.700000", test_env.text_sent[0]);
}

TEST(RunTest, RunsStringFunctions2) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  drv.SetEnvironment(&test_env);
  CodeBlock block(&test_env);

  EXPECT_EQ(0, drv.parse("a[0] = { 5, 4, 3, 2, 1}\n a[2] = 8.8\n print(out1, debug(a[], 0, 5))"));
  ASSERT_EQ(1, drv.blocks.size());
  ASSERT_TRUE(translator.BlockToCodeBlock(&block, drv.blocks[0]));
  ASSERT_EQ(0, test_env.text_sent.size());
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  ASSERT_EQ(1, test_env.text_sent.size());
  EXPECT_EQ("a[0] = {5, 4, 8.800000, 2, 1, 0}", test_env.text_sent[0]);
}

TEST(RunTest, RunsStringFunctions3) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  drv.SetEnvironment(&test_env);
  CodeBlock block(&test_env);

  // Make sure works even when debug() is first mention of the variable.
  EXPECT_EQ(0, drv.parse("print(out1, debug(a[], 0, 5))"));
  ASSERT_EQ(1, drv.blocks.size());
  ASSERT_TRUE(translator.BlockToCodeBlock(&block, drv.blocks[0]));
  ASSERT_EQ(0, test_env.text_sent.size());
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  ASSERT_EQ(1, test_env.text_sent.size());
  EXPECT_EQ("a[0] = {0, 0, 0, 0, 0, 0}", test_env.text_sent[0]);
}

TEST(RunTest, RunsStringFunctions4) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  drv.SetEnvironment(&test_env);
  CodeBlock block(&test_env);

  // Make sure works even when debug() is first mention of the variable.
  EXPECT_EQ(0, drv.parse("print(OUT3, debug(foo))\nfoo = 4\nprint(OUT3, debug(foo))"));
  ASSERT_EQ(1, drv.blocks.size());
  ASSERT_TRUE(translator.BlockToCodeBlock(&block, drv.blocks[0]));
  ASSERT_EQ(0, test_env.text_sent.size());
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  ASSERT_EQ(2, test_env.text_sent.size());
  EXPECT_EQ("foo = 0", test_env.text_sent[0]);
  EXPECT_EQ("foo = 4", test_env.text_sent[1]);
}

TEST(RunTest, StartTest) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  drv.SetEnvironment(&test_env);
  CodeBlock block(&test_env);

  ASSERT_EQ(0, drv.parse("foo = start()"));
  ASSERT_EQ(1, drv.blocks.size());
  std::vector<Line>* lines = &(drv.blocks[0].lines);
  ASSERT_EQ(1, lines->size());
  ASSERT_EQ(0, lines->at(0).expr1.Compute());
  test_env.starting = true;
  ASSERT_EQ(1, lines->at(0).expr1.Compute());
}

// TODO: Add tests for all the language constructs.

TEST(RunTest, RunsForLoop) {
  Driver drv;
  PCodeTranslator translator;
  TestEnvironment test_env;
  CodeBlock block(&test_env);

  EXPECT_EQ(0, drv.parse("for I = 0 To 1 STEP 0.1 nExT"));
  ASSERT_EQ(1, drv.blocks.size());
  ASSERT_TRUE(translator.BlockToCodeBlock(&block, drv.blocks[0]));
  ASSERT_EQ(4, block.pcodes.size());

  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  EXPECT_EQ(0.0f, *(drv.GetVarFromName("i")));
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  EXPECT_EQ(0.1f, *(drv.GetVarFromName("i")));
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  EXPECT_EQ(0.2f, *(drv.GetVarFromName("i")));
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  EXPECT_EQ(0.3f, *(drv.GetVarFromName("i")));
  EXPECT_EQ(CodeBlock::CONTINUES, block.Run(true));
  EXPECT_EQ(0.4f, *(drv.GetVarFromName("i")));
}
