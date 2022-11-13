#include "../src/extended_text.h"

#include "gtest/gtest.h"
#include <stdexcept>


TEST(TextFieldTest, ProcessTest)
{
  ExtendedText et;
  et.ProcessUpdatedText(std::string("one line"));
  ASSERT_EQ(1, et.line_map.size());
  TextLine a(0, 0, 8);
  EXPECT_TRUE(a == et.line_map[0]);

  et.ProcessUpdatedText(std::string("a line\nanother"));
  ASSERT_EQ(2, et.line_map.size());
  TextLine b(0, 0, 6), c(1, 7, 7);
  EXPECT_TRUE(b == et.line_map[0]);
  EXPECT_TRUE(c == et.line_map[1]);

  et.ProcessUpdatedText(std::string("a line\nanother\n"));
  ASSERT_EQ(3, et.line_map.size());
  TextLine d(2, 15, 0);
  EXPECT_TRUE(b == et.line_map[0]);
  EXPECT_TRUE(c == et.line_map[1]);
  EXPECT_TRUE(d == et.line_map[2]);
}

TEST(TextFieldTest, ColumnTest)
{
  ExtendedText et;
  et.ProcessUpdatedText(std::string("a line\nanother\n"));
  ASSERT_EQ(3, et.line_map.size());
  LineColumn a(0, 0);
  EXPECT_TRUE(a == et.GetCurrentLineColumn(0));
  a.column = 1;
  EXPECT_TRUE(a == et.GetCurrentLineColumn(1));
  a.line = 1;
  a.column = 0;
  EXPECT_TRUE(a == et.GetCurrentLineColumn(7));
  a.column = 7;
  EXPECT_TRUE(a == et.GetCurrentLineColumn(14));
  a.line = 2;
  a.column = 0;
  EXPECT_TRUE(a == et.GetCurrentLineColumn(15));

  EXPECT_EQ(0, et.GetCursorForLineColumn(0, 0));
  EXPECT_EQ(1, et.GetCursorForLineColumn(0, 1));
  EXPECT_EQ(6, et.GetCursorForLineColumn(0, 9));
  EXPECT_EQ(15, et.GetCursorForLineColumn(3, 0));
  EXPECT_EQ(15, et.GetCursorForLineColumn(2, 1));
}
