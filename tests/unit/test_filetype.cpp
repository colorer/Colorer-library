#include <colorer/FileType.h>
#include <catch2/catch.hpp>

TEST_CASE("Create FileType and set base properties")
{
  const UnicodeString name(u"TestType");
  const UnicodeString group(u"TestGroup");
  const UnicodeString description(u"TestDescription");
  FileType file_type(name, group, description);

  REQUIRE(name.compare(file_type.getName()) == 0);
  REQUIRE(group.compare(file_type.getGroup()) == 0);
  REQUIRE(description.compare(file_type.getDescription()) == 0);

  SECTION("change base properties")
  {
    const UnicodeString name1(u"TestType1");
    const UnicodeString group1(u"TestGroup1");
    const UnicodeString description1(u"TestDescription1");

    file_type.setName(name1);
    file_type.setGroup(group1);
    file_type.setDescription(description1);

    REQUIRE(name1.compare(file_type.getName()) == 0);
    REQUIRE(group1.compare(file_type.getGroup()) == 0);
    REQUIRE(description1.compare(file_type.getDescription()) == 0);
  }

  SECTION("change base properties to empty string")
  {
    const UnicodeString empty_string(u"");

    REQUIRE_THROWS_AS(file_type.setName(empty_string), FileTypeException);
    file_type.setGroup(empty_string);
    file_type.setDescription(empty_string);

    REQUIRE(empty_string.compare(file_type.getGroup()) == 0);
    REQUIRE(empty_string.compare(file_type.getDescription()) == 0);
  }
}

SCENARIO("Set param value for FileType")
{
  const UnicodeString param1(u"param1");
  const UnicodeString def_value(u"value1");
  const UnicodeString def_value2(u"value2");
  const UnicodeString user_value(u"user_value");

  const UnicodeString name(u"TestType");
  const UnicodeString group(u"TestGroup");
  const UnicodeString description(u"TestDescription");

  GIVEN("A simple FileType with one parameter")
  {
    FileType file_type(name, group, description);
    REQUIRE_THROWS_AS(file_type.addParam(param1, nullptr), FileTypeException);
  }
  GIVEN("A simple FileType with two parameter")
  {
    FileType file_type(name, group, description);
    file_type.addParam(param1, def_value2);

    REQUIRE(file_type.getParamValue(param1)->compare(def_value2) == 0);
    REQUIRE_THROWS_AS(file_type.addParam(param1, nullptr), FileTypeException);

    WHEN("set only default value for parameter")
    {
      file_type.setParamDefaultValue(param1, &def_value);

      THEN("return this value as default and as value")
      {
        REQUIRE(file_type.getParamDefaultValue(param1)->compare(def_value) == 0);
        REQUIRE(file_type.getParamValue(param1)->compare(def_value) == 0);
      }
      THEN("user value is empty")
      {
        REQUIRE(nullptr == file_type.getParamUserValue(param1));
      }
    }
    WHEN("set value for parameter")
    {
      file_type.setParamValue(param1, &def_value);

      THEN("return this value as value, but not default")
      {
        REQUIRE(file_type.getParamValue(param1)->compare(def_value) == 0);
        REQUIRE(file_type.getParamDefaultValue(param1)->compare(def_value2) == 0);
      }
    }
    WHEN("set nullptr as default value ")
    {
      file_type.setParamDefaultValue(param1, &def_value);

      THEN("exception to set null value")
      {
        REQUIRE_THROWS_AS(file_type.setParamDefaultValue(param1, nullptr), FileTypeException);
      }
    }
    WHEN("set nullptr as value ")
    {
      file_type.setParamValue(param1, user_value);
      file_type.setParamValue(param1, nullptr);

      THEN("return default and user value equal nullptr")
      {
        REQUIRE(file_type.getParamUserValue(param1) == nullptr);
        REQUIRE(file_type.getParamValue(param1)->compare(def_value2) == 0);
      }
    }
    WHEN("FileType don`t contains param2")
    {
      THEN("return it value throws exception")
      {
        UnicodeString param2("param2");
        REQUIRE_THROWS_AS(file_type.setParamValue(param2, def_value), FileTypeException);
        REQUIRE_THROWS_AS(file_type.setParamDefaultValue(param2, &def_value), FileTypeException);
        REQUIRE_THROWS_AS(file_type.setParamDescription(param2, &def_value), FileTypeException);
      }
    }
    WHEN("set param description")
    {
      REQUIRE_NOTHROW(file_type.setParamDescription(param1, &description));
      THEN("return it empty value")
      {
        REQUIRE(file_type.getParamDescription(param1)->compare(description) == 0);
      }
    }
    WHEN("reset param description")
    {
      REQUIRE_NOTHROW(file_type.setParamDescription(param1, nullptr));
      THEN("return it empty value")
      {
        REQUIRE(file_type.getParamDescription(param1) == nullptr);
      }
    }
  }
}

TEST_CASE("Work with integer type of param value")
{
  UnicodeString param1(u"param1");
  UnicodeString param2(u"param2");
  UnicodeString param3(u"param3");
  UnicodeString def_value1(u"0");
  UnicodeString value1(u"value1");
  UnicodeString value2(u"5");
  UnicodeString value3(u"5.5");

  UnicodeString name(u"TestType");
  UnicodeString group(u"TestGroup");
  UnicodeString description(u"TestDescription");

  FileType file_type(name, group, description);

  file_type.addParam(param1, def_value1);
  file_type.addParam(param2, def_value1);
  file_type.addParam(param3, def_value1);
  file_type.setParamValue(param1, &value1);
  file_type.setParamValue(param2, &value2);
  file_type.setParamValue(param3, &value3);

  CHECK(5 == file_type.getParamValueInt(param2));
  CHECK(5 == file_type.getParamValueInt(param3));
  int ret;
  REQUIRE_NOTHROW(ret = file_type.getParamValueInt(param1));
}
