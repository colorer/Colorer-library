#include <colorer/FileType.h>
#include <gtest/gtest.h>

TEST(FileType, FileTypeName)
{
  UnicodeString name("TestType");
  FileType file_type;
  file_type.setName(&name);

  EXPECT_TRUE(name.compare(*file_type.getName()) == 0);
}

TEST(FileType, FileTypeGroup)
{
  UnicodeString group("TestGroup");
  FileType file_type;
  file_type.setGroup(&group);

  EXPECT_TRUE(group.compare(*file_type.getGroup()) == 0);
}

TEST(FileType, FileTypeDescription)
{
  UnicodeString description("TestDescription");
  FileType file_type;
  file_type.setDescription(&description);

  EXPECT_TRUE(description.compare(*file_type.getDescription()) == 0);
}

TEST(FileType, FileTypeParamValue)
{
  UnicodeString param1("param1");
  UnicodeString def_value("value1");
  UnicodeString user_value("value2");

  FileType file_type;
  file_type.addParam(&param1);
  file_type.setParamDefaultValue(param1, &def_value);

  EXPECT_TRUE(file_type.getParamDefaultValue(param1)->compare(def_value) == 0);
  EXPECT_TRUE(file_type.getParamValue(param1)->compare(def_value) == 0);
  EXPECT_EQ(nullptr, file_type.getParamUserValue(param1));

  file_type.setParamValue(param1, &def_value);
  EXPECT_TRUE(file_type.getParamValue(param1)->compare(def_value) == 0);

  file_type.setParamDefaultValue(param1, nullptr);
  EXPECT_EQ(nullptr, file_type.getParamDefaultValue(param1));

  file_type.setParamValue(param1, &user_value);
  EXPECT_TRUE(file_type.getParamValue(param1)->compare(user_value) == 0);

  file_type.setParamValue(param1, nullptr);
  EXPECT_EQ(nullptr, file_type.getParamUserValue(param1));
  EXPECT_EQ(nullptr, file_type.getParamValue(param1));
}

TEST(FileType, FileTypeParam)
{
  UnicodeString param1("param1");
  UnicodeString param2("param2");
  UnicodeString def_value("value1");

  FileType file_type;
  file_type.addParam(&param1);

  EXPECT_THROW(file_type.setParamValue(UnicodeString(nullptr), &def_value), FileTypeException);
  EXPECT_THROW(file_type.setParamValue(param2, &def_value), FileTypeException);

  EXPECT_THROW(file_type.setParamDefaultValue(UnicodeString(nullptr), &def_value), FileTypeException);
  EXPECT_THROW(file_type.setParamDefaultValue(param2, &def_value), FileTypeException);

  EXPECT_THROW(file_type.setParamDescription(UnicodeString(nullptr), &def_value), FileTypeException);
  EXPECT_THROW(file_type.setParamDescription(param2, &def_value), FileTypeException);
}

TEST(FileType, FileTypeParamValueInt)
{
  UnicodeString param1("param1");
  UnicodeString param2("param2");
  UnicodeString param3("param3");
  UnicodeString value1("value1");
  UnicodeString value2("5");
  UnicodeString value3("5.5");

  FileType file_type;
  file_type.addParam(&param1);
  file_type.addParam(&param2);
  file_type.addParam(&param3);
  file_type.setParamValue(param1,&value1);
  file_type.setParamValue(param2,&value2);
  file_type.setParamValue(param3,&value3);

  EXPECT_EQ(5,file_type.getParamValueInt(param2));
  EXPECT_EQ(5,file_type.getParamValueInt(param3));
  EXPECT_NO_THROW(file_type.getParamValueInt(UnicodeString(nullptr)));
  EXPECT_NO_THROW(file_type.getParamValueInt(param1));
}