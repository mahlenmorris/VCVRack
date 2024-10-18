#include "gtest/gtest.h"

#include "../src/parser-venn/driver.h"
#include "../src/parser-venn/tree.h"
#include <stdexcept>

TEST(ParserTest, ParsesAtAll)
{
    VennDriver drv;

    EXPECT_EQ(0, drv.parse("[]\n x = 2\ny=-1 \n radius = 3.5"));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    Diagram* diagram = &(drv.diagram);
    ASSERT_EQ(1, diagram->circles.size());
    EXPECT_EQ(2.0, diagram->circles.at(0).x_center);
}

TEST(ParserTest, BadVar1)
{
    VennDriver drv;

    EXPECT_EQ(1, drv.parse("[]\n x = 2\ny=-1 \n radius = 3.5\n bloont = 4.333"));
    EXPECT_EQ(1, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    Diagram* diagram = &(drv.diagram);
    ASSERT_EQ(1, diagram->circles.size());
    EXPECT_EQ(2.0, diagram->circles.at(0).x_center);
}

TEST(ParserTest, BadVar2)
{
    VennDriver drv;

    EXPECT_EQ(1, drv.parse("[]\n x center = 2\ny=-1 \n radius = 3.5\n"));
    EXPECT_EQ(1, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
}

TEST(ParserTest, TwoWordName)
{
    VennDriver drv;

    EXPECT_EQ(0, drv.parse("[]\n x = 2\ny=-1 \n radius = 3.5\n name = \"Bass Delay\""));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    Diagram* diagram = &(drv.diagram);
    EXPECT_EQ(1, diagram->circles.size());
    EXPECT_EQ("Bass Delay", diagram->circles.at(0).name);
}

TEST(ParserTest, ThreeLineName)
{
    VennDriver drv;

    EXPECT_EQ(0, drv.parse("[]\n x = 2\ny=-1 \n radius = 3.5\n name = \"Bass\nDelay\nMenace\""));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    Diagram* diagram = &(drv.diagram);
    EXPECT_EQ(1, diagram->circles.size());
    EXPECT_EQ("Bass\nDelay\nMenace", diagram->circles.at(0).name);
}

TEST(ParserTest, ParsesMultipleCircles)
{
    VennDriver drv;
    const char * text =
      "[]\n"
      " x = 2\n"
      "y=-1 \n"
      "radius = 3.5\n"
      "\n"
      "[]\n"
      " Radius = 2\n"
      "y=-1 \n"
      "X = 3.5\n"
      "name = \"\"\n";

    EXPECT_EQ(0, drv.parse(text));
    EXPECT_EQ(0, drv.errors.size());
    Diagram* diagram = &(drv.diagram);
    ASSERT_EQ(2, diagram->circles.size());
    EXPECT_EQ(2.0, diagram->circles.at(0).x_center);
}

TEST(ParserTest, NoName)
{
    VennDriver drv;
    const char * text =
      "[]\n"
      "x = 2\n"
      "y = -1\n"
      "radius = 3.5\n"
      "\n";

    EXPECT_EQ(0, drv.parse(text));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    Diagram* diagram = &(drv.diagram);
    ASSERT_EQ(1, diagram->circles.size());
    EXPECT_EQ("", diagram->circles.at(0).name);
}

TEST(ParserTest, SimpleExpression)
{
    VennDriver drv;
    const char * text = "3.14159";

    EXPECT_EQ(0, drv.parse(text));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    Expression* exp = &(drv.exp);
    ASSERT_FLOAT_EQ(3.14159, exp->Compute());
}
