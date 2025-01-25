#include "gtest/gtest.h"

#include "../src/parser-venn/driver.h"
#include "../src/parser-venn/tree.h"
#include <stdexcept>

TEST(ParserTest, ParsesAtAll)
{
    std::shared_ptr<VennVariables> variables = std::make_shared<VennVariables>();
    VennDriver drv(variables);

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
    std::shared_ptr<VennVariables> variables = std::make_shared<VennVariables>();
    VennDriver drv(variables);

    EXPECT_EQ(1, drv.parse("[]\n x = 2\ny=-1 \n radius = 3.5\n bloont = 4.333"));
    EXPECT_EQ(1, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
}

TEST(ParserTest, BadVar2)
{
    std::shared_ptr<VennVariables> variables = std::make_shared<VennVariables>();
    VennDriver drv(variables);

    EXPECT_EQ(1, drv.parse("[]\n x center = 2\ny=-1 \n radius = 3.5\n"));
    EXPECT_EQ(1, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
}

TEST(ParserTest, TwoWordName)
{
    std::shared_ptr<VennVariables> variables = std::make_shared<VennVariables>();
    VennDriver drv(variables);

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
    std::shared_ptr<VennVariables> variables = std::make_shared<VennVariables>();
    VennDriver drv(variables);

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
    std::shared_ptr<VennVariables> variables = std::make_shared<VennVariables>();
    VennDriver drv(variables);
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
    std::shared_ptr<VennVariables> variables = std::make_shared<VennVariables>();
    VennDriver drv(variables);
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
    std::shared_ptr<VennVariables> variables = std::make_shared<VennVariables>();
    VennDriver drv(variables);
    const char * text = "3.14159";

    EXPECT_EQ(0, drv.parse(text));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    VennExpression* exp = &(drv.exp);
    ASSERT_FLOAT_EQ(3.14159, exp->Compute());
}

TEST(ParserTest, ManyExpressions)
{
    std::shared_ptr<VennVariables> variables = std::make_shared<VennVariables>();
    VennDriver drv(variables);
    
    EXPECT_EQ(0, drv.parse("30 * 3"));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    VennExpression* exp = &(drv.exp);
    ASSERT_FLOAT_EQ(90, exp->Compute());

    EXPECT_EQ(0, drv.parse("(30 * 3)"));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    exp = &(drv.exp);
    ASSERT_FLOAT_EQ(90, exp->Compute());

    EXPECT_EQ(0, drv.parse("1 + x"));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    exp = &(drv.exp);
    variables->var_x = 3.14;
    ASSERT_FLOAT_EQ(4.14, exp->Compute());

    EXPECT_EQ(0, drv.parse("x + 1"));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    exp = &(drv.exp);
    variables->var_x = 3.14;
    ASSERT_FLOAT_EQ(4.14, exp->Compute());

    EXPECT_EQ(0, drv.parse("sin(30*3.14159265/180) * MAx(2, 0.001)"));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    exp = &(drv.exp);
    ASSERT_FLOAT_EQ(1.0, exp->Compute());

    EXPECT_EQ(0, drv.parse("e4"));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    exp = &(drv.exp);
    EXPECT_NEAR(0.33333332, exp->Compute(), 0.00001);
}
