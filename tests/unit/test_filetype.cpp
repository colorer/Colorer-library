#include <colorer/FileType.h>
#include <spdlog/sinks/null_sink.h>
#include <catch2/catch.hpp>

TEST_CASE("Create FileType and set base properties")
{
  // disable logging
  spdlog::drop_all();
  auto log = spdlog::null_logger_mt("main");
  spdlog::set_default_logger(log);

  UnicodeString name("TestType");
  UnicodeString group("TestGroup");
  UnicodeString description("TestDescription");
  FileType file_type(name, group, description);
  file_type.setName(&name);
  file_type.setGroup(&group);
  file_type.setDescription(&description);

  CHECK(name.compare(file_type.getName()) == 0);
  CHECK(group.compare(file_type.getGroup()) == 0);
  CHECK(description.compare(file_type.getDescription()) == 0);
}

SCENARIO("Set param value for FileType")
{
  UnicodeString param1("param1");
  UnicodeString def_value("value1");
  UnicodeString def_value2("dvalue1");
  UnicodeString user_value("value2");

  UnicodeString name("TestType");
  UnicodeString group("TestGroup");
  UnicodeString description("TestDescription");

  GIVEN("A simple FileType with one parameter")
  {
    FileType file_type(name, group, description);
    REQUIRE_THROWS_AS(file_type.addParam(&param1, nullptr), FileTypeException);
  }
  GIVEN("A simple FileType with two parameter")
  {
    FileType file_type(name, group, description);
    file_type.addParam(&param1, &def_value2);

    WHEN("set only default value for parameter")
    {
      file_type.setParamDefaultValue(param1, &def_value);

      THEN("return this value as default and as value")
      {
        CHECK(file_type.getParamDefaultValue(param1)->compare(def_value) == 0);
        CHECK(file_type.getParamValue(param1)->compare(def_value) == 0);
      }
      THEN("user value is empty")
      {
        CHECK(nullptr == file_type.getParamUserValue(param1));
      }
    }
    WHEN("set value for parameter")
    {
      file_type.setParamValue(param1, &def_value);

      THEN("return this value as value, but not default")
      {
        CHECK(file_type.getParamValue(param1)->compare(def_value) == 0);
        CHECK(file_type.getParamDefaultValue(param1)->compare(def_value2) == 0);
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
      file_type.setParamValue(param1, &user_value);
      file_type.setParamValue(param1, nullptr);

      THEN("return default and user value equal nullptr")
      {
        CHECK(file_type.getParamUserValue(param1) == nullptr);
        CHECK(file_type.getParamValue(param1)->compare(def_value2) == 0);
      }
    }
    WHEN("FileType don`t contains param2")
    {
      THEN("return it value throws exception")
      {
        UnicodeString param2("param2");
        REQUIRE_THROWS_AS(file_type.setParamValue(param2, &def_value), FileTypeException);
        REQUIRE_THROWS_AS(file_type.setParamDefaultValue(param2, &def_value), FileTypeException);
        REQUIRE_THROWS_AS(file_type.setParamDescription(param2, &def_value), FileTypeException);
      }
    }
    WHEN("FileType exists")
    {
      THEN("return nullptr param value throws exception")
      {
        REQUIRE_THROWS_AS(file_type.setParamValue(UnicodeString(nullptr), &def_value),
                          FileTypeException);
        REQUIRE_THROWS_AS(file_type.setParamDefaultValue(UnicodeString(nullptr), &def_value),
                          FileTypeException);
        REQUIRE_THROWS_AS(file_type.setParamDescription(UnicodeString(nullptr), &def_value),
                          FileTypeException);
      }
    }
  }
}

TEST_CASE("Work with integer type of param value")
{
  // disable logging
  spdlog::drop_all();
  auto log = spdlog::null_logger_mt("main");
  spdlog::set_default_logger(log);

  UnicodeString param1("param1");
  UnicodeString param2("param2");
  UnicodeString param3("param3");
  UnicodeString def_value1("0");
  UnicodeString value1("value1");
  UnicodeString value2("5");
  UnicodeString value3("5.5");

  UnicodeString name("TestType");
  UnicodeString group("TestGroup");
  UnicodeString description("TestDescription");

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
  REQUIRE_NOTHROW(ret = file_type.getParamValueInt(UnicodeString(nullptr)));
  REQUIRE_NOTHROW(ret = file_type.getParamValueInt(param1));
}
