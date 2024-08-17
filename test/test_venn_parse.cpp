#include "gtest/gtest.h"

#include "../src/parser-venn/driver.h"
#include "../src/parser-venn/tree.h"
#include <stdexcept>

TEST(ParserTest, ParsesAtAll)
{
    Driver drv;

    EXPECT_EQ(0, drv.parse("[foo]\n x = 2\ny=-1 \n radius = 3.5"));
    EXPECT_EQ(0, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    Diagram* diagram = &(drv.diagram);
    ASSERT_EQ(1, diagram->circles.size());
    EXPECT_EQ(2.0, diagram->circles.at(0).x_center);
}

TEST(ParserTest, BadVar)
{
    Driver drv;

    EXPECT_EQ(1, drv.parse("[foo]\n x = 2\ny=-1 \n radius = 3.5\n bloont = 4.333"));
    EXPECT_EQ(1, drv.errors.size());
    if (drv.errors.size() > 0) {
      std::cout << "\n" << drv.errors.at(0).to_string() << "\n";
    }
    Diagram* diagram = &(drv.diagram);
    ASSERT_EQ(1, diagram->circles.size());
    EXPECT_EQ(2.0, diagram->circles.at(0).x_center);
}

TEST(ParserTest, ParsesMultipleCircles)
{
    Driver drv;
    const char * text =
      "[foo]\n"
      " x = 2\n"
      "y=-1 \n"
      "radius = 3.5\n"
      "\n"
      "[bar]\n"
      " radius = 2\n"
      "y=-1 \n"
      "x = 3.5\n";

    EXPECT_EQ(0, drv.parse(text));
    EXPECT_EQ(0, drv.errors.size());
    Diagram* diagram = &(drv.diagram);
    ASSERT_EQ(2, diagram->circles.size());
    EXPECT_EQ(2.0, diagram->circles.at(0).x_center);
}
