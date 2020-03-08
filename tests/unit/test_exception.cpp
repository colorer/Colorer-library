#include <colorer/Exception.h>
#include <gtest/gtest.h>

TEST(Exception, ExceptionChar)
{
  Exception excp("error");
  std::string what = excp.what();
  EXPECT_EQ("error", what);
}

TEST(Exception, ExceptionUnicodeString)
{
  Exception excp(UnicodeString("error"));
  std::string what = excp.what();
  EXPECT_EQ("error", what);
}
